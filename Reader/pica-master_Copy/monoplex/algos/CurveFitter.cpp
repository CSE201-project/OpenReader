//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/30.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include <ceres/ceres.h>

#include "CurveFitter.h"

double CurveFitter::exponantialModel(double x, const std::vector<double>& beta)
{
    return beta[0] + beta[1] * std::exp(-beta[2] * x);
}

struct ExponantialResidual {
    ExponantialResidual(double x, double y)
            : x_(x), y_(y) {}

    template <typename T> bool operator()(const T* const b1,
                                          const T* const b2,
                                          const T* const b3,
                                          T* residual) const {
        residual[0] = T(y_) - (b1[0] + b2[0] * exp(-b3[0] * T(x_)));
        return true;
    }

private:
    double x_;
    double y_;
};

double CurveFitter::poly2Model(double x, const std::vector<double>& beta)
{
    return beta[0] * std::pow(x, 2) + beta[1] * x + beta[2];
}

struct Poly2Residual {
    Poly2Residual(double x, double y)
            : x_(x), y_(y) {}

    template <typename T> bool operator()(const T* const b1,
                                          const T* const b2,
                                          const T* const b3,
                                          T* residual) const {
        residual[0] = T(y_) - (b1[0] * pow(T(x_), 2) + b2[0] * T(x_) + b3[0]);
        return true;
    }

private:
    double x_;
    double y_;
};

struct LinearRegressionResidual {
    LinearRegressionResidual(double x, double y)
            : x_(x), y_(y) {}

    template <typename T> bool operator()(const T* const A,
                        const T* const B,
                        T* residual) const {
        residual[0] = T(y_) - ((A[0] * T(x_)) + B[0] );
        return true;
    }

private:
    double x_;
    double y_;
};

ceres::CostFunction* initCostFunction(ResidualType residualType, double i, double inputData, uint32_t numberOfInputData)
{
    ceres::CostFunction* cost_function;

    switch(residualType) {
        case ExpoFit:
            cost_function = new ceres::AutoDiffCostFunction<ExponantialResidual, 1, 1, 1, 1>(
                    new ExponantialResidual(i, inputData));
            break;
        case Poly2:
            cost_function = new ceres::AutoDiffCostFunction<Poly2Residual, 1, 1, 1, 1>(
                    new Poly2Residual(i, inputData));
            break;
        case LinearRegression:
            cost_function = new ceres::AutoDiffCostFunction<LinearRegressionResidual, 1, 1, 1>(
                    new LinearRegressionResidual(i, inputData));
            break;
        default:
            throw std::runtime_error("Unknown type of residual");
    }
    return cost_function;
}

double computeCoefficientOfDetermination(const std::vector<double>& residualsVector, const std::vector<double>& inputData)
{
    uint32_t numberOfInputNotNan = residualsVector.size();
    double residualsSumSquares = std::accumulate(residualsVector.begin(), residualsVector.end(), 0.0,[](double sum, double residual) {
        return sum + std::pow(residual, 2);
    });

    double accumulator = 0;
    for(auto & nb : inputData)
    {
        if(std::isnan(nb))
            continue;
        accumulator += nb;
    }
    double mean = accumulator / numberOfInputNotNan;

    double variance = 0;
    for(auto & nb : inputData)
    {
        if(std::isnan(nb))
            continue;
        variance += std::pow(nb - mean, 2);
    }

    double FVU = residualsSumSquares / variance;
    return 1 - FVU;
}

std::tuple<std::vector<double>, double> CurveFitter::solveModelCeres(const std::vector<double> & inputData, double cauchyLossParam, ResidualType residualType)
{
    const uint32_t kNumObservations = inputData.size();

    double b1 = InitialValues[0];
    double b2 = InitialValues[1];
    double b3 = InitialValues[2];

    ceres::Problem problem;
    for (int i = 0; i < kNumObservations; ++i) {
        if(std::isnan(inputData[i]))
            continue;

        ceres::CostFunction* cost_function = initCostFunction(residualType, i, inputData[i], inputData.size());
        problem.AddResidualBlock(cost_function, new ceres::CauchyLoss(cauchyLossParam), &b1, &b2, &b3);
    }
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = 400;
    options.logging_type = ceres::SILENT;

    ceres::Solver::Summary summary;
    Solve(options, &problem, &summary);

    std::vector<double> residualsVector;
    double cost = 0.;
    problem.Evaluate(ceres::Problem::EvaluateOptions(), &cost, &residualsVector, nullptr, nullptr);

    double coefficient = computeCoefficientOfDetermination(residualsVector, inputData);
    std::vector<double> beta;

    beta = { b1, b2, b3 };

    return {beta, coefficient};
}

std::tuple<LinearRegressionCoefficients, double> CurveFitter::computeLinearRegressionFit(const std::map<double, double>& dataWithConcentrations,
                                                                                         LinearRegressionCoefficients initialValues)
{
    std::vector<double> inputDataForCoefficientOfDetermination;

    ceres::Problem problem;
    for(auto& data : dataWithConcentrations)
    {
        double concentration = data.first;
        double value = data.second;

        if(std::isnan(value))
            continue;

        inputDataForCoefficientOfDetermination.push_back(value);

        ceres::CostFunction* cost_function = initCostFunction(LinearRegression, concentration, value, dataWithConcentrations.size());
        problem.AddResidualBlock(cost_function, nullptr, &initialValues.A, &initialValues.B);
    }
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = 400;
    options.logging_type = ceres::SILENT;

    ceres::Solver::Summary summary;
    Solve(options, &problem, &summary);

    std::vector<double> residualsVector;
    double cost = 0.;
    problem.Evaluate(ceres::Problem::EvaluateOptions(), &cost, &residualsVector, nullptr, nullptr);

    double coefficient = computeCoefficientOfDetermination(residualsVector, inputDataForCoefficientOfDetermination);

    return {initialValues, coefficient};
}

TImageProfile CurveFitter::removeFitFromProfile(const TImageProfile& profile, const TImageProfile& fit)
{
    TImageProfile profileFitted;
    int size = profile.size();
    for(uint32_t i = 0; i < size; i++)
        profileFitted.push_back(profile[i] - fit[i]);

    double min = *std::min_element(profileFitted.begin(), profileFitted.end());

    for(uint32_t i = 0; i < profileFitted.size(); i++)
        profileFitted[i] -= min;

    return profileFitted;
}

double CurveFitter::computeSigmoidePrediction(SigmoideCoefficients coefficients, double y)
{
    double bottom = coefficients.Bottom;
    double top = coefficients.Top;
    double hillSlope = coefficients.HillSlope;
    double ic50 = coefficients.IC50;

    double result = ic50 / std::exp(std::log(((top - bottom)/(y-bottom)) - 1) / hillSlope);

    return std::round(result * 100)/100.;
}

double CurveFitter::computeLinearRegressionPrediction(LinearRegressionCoefficients coefficients, double y)
{
    double a = coefficients.A;
    double b = coefficients.B;

    double result = (y - b) / a;
    if(result < 0)
        return 0;
    else
        return std::round(result * 100)/100.;
}