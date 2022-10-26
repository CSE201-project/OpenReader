//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/30.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef GAUSSIANFIT_CURVEFITTER_H
#define GAUSSIANFIT_CURVEFITTER_H

#include <functional>
#include <vector>
#include "IImageProcessor.h"

enum ResidualType {ExpoFit, Poly2, LinearRegression};

class CurveFitter {
public:
    std::vector<double> InitialValues;

    static double exponantialModel(double x, const std::vector<double>& beta);
    static double poly2Model(double x, const std::vector<double>& beta);

    std::tuple<std::vector<double>, double> solveModelCeres(const std::vector<double> & inputData, double cauchyLossParam, ResidualType residualType);
    static std::tuple<LinearRegressionCoefficients, double> computeLinearRegressionFit(const std::map<double, double>& dataWithConcentrations,
                                                                                       LinearRegressionCoefficients initialValues);
    static TImageProfile removeFitFromProfile(const TImageProfile& profile, const TImageProfile& fit);

    static double computeSigmoidePrediction(SigmoideCoefficients coefficients, double y);
    static double computeLinearRegressionPrediction(LinearRegressionCoefficients coefficients, double y);
};



#endif //GAUSSIANFIT_CURVEFITTER_H
