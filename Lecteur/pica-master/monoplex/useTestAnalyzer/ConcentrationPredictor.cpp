//
// Created by KarimSadki on 04/01/2021.
//

#include <numeric>

#include "ConcentrationPredictor.h"

#include "monoplex/algos/CurveFitter.h"
#include "monoplex/algos/AlgoProxNoise.h"
#include "monoplex/algos/AlgoExponantialFit.h"
#include "monoplex/algos/AlgoPoly2.h"
#include "monoplex/algos/AlgoTestMinusProxNoise.h"

void ConcentrationPredictor::generateLinearRegressionCoefficients(OpenXLSX::XLWorkbook& workbook,
                                                                  const std::map<double, std::vector<std::vector<Results>>>& allImagesResults)
{
    std::vector<MeansForLinearRegression> allMeansToFit = computeAllMeansToFit(allImagesResults);
    std::vector<CalibrationCurvesCoefficients> allTLCoefficients = computeAllLinearRegresssionCoefficients(allMeansToFit);
    writeAllComputedCoefficients(workbook, allTLCoefficients);
}

Predictions ConcentrationPredictor::computePredictions(CalibrationCurvesCoefficients& coefficients,
                                                       const Results& results,
                                                       double realConcentration)
{
    AlgoProxNoise algoProxNoise;
    AlgoExponantialFit algoExponantialFit;
    AlgoPoly2 algoPoly2;
    AlgoTestMinusProxNoise algoTestMinusProxNoise;

    double predictionSigmoideProxNoise = algoProxNoise.computeSecondPrediction(std::get<0>(results.proxNoise), constructSigmoideCoefficientsString(coefficients.proxNoiseBrutSigmoide));
    double predictionSigmoideExponantialFit = algoExponantialFit.computeSecondPrediction(std::get<0>(results.expoFitProxNoise), constructSigmoideCoefficientsString(coefficients.expoFitSigmoide));
    double predictionSigmoidePoly2 = algoPoly2.computeSecondPrediction(std::get<0>(results.poly2ProxNoise), constructSigmoideCoefficientsString(coefficients.poly2Sigmoide));
    double predictionSigmoideTestMinusProxNoise = algoTestMinusProxNoise.computeFirstPrediction(std::get<0>(results.testMinusProxNoise), constructSigmoideCoefficientsString(coefficients.testMinusProxNoiseSigmoide));

    double predictionConvolSigmoideTestMinusProxNoise = algoTestMinusProxNoise.computeFirstPrediction(std::get<0>(results.convolTestMinusProxNoise), constructSigmoideCoefficientsString(coefficients.convolTestMinusProxNoiseSigmoide));

    double predictionLinearBrut = algoProxNoise.computeFirstPrediction(std::get<0>(results.proxNoise), constructLinearRegressionCoefficientsString(coefficients.proxNoiseBrutLinear));
    double predictionLinearExponantialFit = algoExponantialFit.computeFirstPrediction(std::get<0>(results.expoFitProxNoise), constructLinearRegressionCoefficientsString(coefficients.expoFitLinear));
    double predictionLinearPoly2 = algoPoly2.computeFirstPrediction(std::get<0>(results.poly2ProxNoise), constructLinearRegressionCoefficientsString(coefficients.poly2Linear));

    double predictionLinearConvolBrut = algoProxNoise.computeFirstPrediction(std::get<0>(results.convolProxNoise), constructLinearRegressionCoefficientsString(coefficients.convolBrutLinear));
    double predictionLinearConvolExpoFit = algoExponantialFit.computeFirstPrediction(std::get<0>(results.convolExpoFitProxNoise), constructLinearRegressionCoefficientsString(coefficients.convolExpoFitLinear));
    double predictionLinearConvolPoly2 = algoPoly2.computeFirstPrediction(std::get<0>(results.convolPoly2ProxNoise), constructLinearRegressionCoefficientsString(coefficients.convolPoly2Linear));

    Predictions predictions = {
            .realConcentrations = realConcentration,
            .concentrationSigmoideBrut = predictionSigmoideProxNoise,
            .concentrationSigmoideExpoFit = predictionSigmoideExponantialFit,
            .concentrationSigmoidePoly2 = predictionSigmoidePoly2,
            .concentrationSigmoideTestMinusProxNoise = predictionSigmoideTestMinusProxNoise,
            .concentrationSigmoideConvolTestMinusProxNoise = predictionConvolSigmoideTestMinusProxNoise,
            .concentrationLinearBrut = predictionLinearBrut,
            .concentrationLinearExpoFit = predictionLinearExponantialFit,
            .concentrationLinearPoly2 = predictionLinearPoly2,
            .concentrationLinearConvolBrut = predictionLinearConvolBrut,
            .concentrationLinearConvolExpoFit = predictionLinearConvolExpoFit,
            .concentrationLinearConvolPoly2 = predictionLinearConvolPoly2
    };
    return predictions;
}

void ConcentrationPredictor::processConcentrationPrediction(OpenXLSX::XLWorkbook& workbook,
                                                            const std::map<double, std::vector<std::vector<Results>>>& allImagesResults,
                                                            const std::filesystem::path& excelFileCalibrationCurves)
{
    if(!std::filesystem::exists(excelFileCalibrationCurves))
        return;

    OpenXLSX::XLDocument file;
    file.open(excelFileCalibrationCurves.string());

    OpenXLSX::XLWorkbook workbookCoefficients = file.workbook();
    workbookCoefficients.setFullCalculationOnLoad();

    CalibrationCurvesCoefficients coefficients = readCoefficientsFromExcel(workbookCoefficients);
    CalibrationCutoffsValues cutoffsValues = readCutoffsValuesFromExcel(workbookCoefficients);

    file.close();

    int numFirstCol = 1;
    int numberOfColsBetweenGrids = 25;
    int numLineToWrite = 3;

    for(auto& element : allImagesResults)
    {
        double concentration = element.first;

        int numLineToWriteCurrentConcentration;

        for(int numTL = 0; numTL < element.second.size(); numTL++)
        {
            numLineToWriteCurrentConcentration = numLineToWrite;
            std::vector<Results> currentTLResults = element.second[numTL];

            for(auto& currentResult : currentTLResults)
            {
                int numFirstColPredictions = numFirstCol + (numTL * numberOfColsBetweenGrids);

                Predictions currentPredictions = computePredictions(coefficients, currentResult, concentration);
                writeConcentrationPredictions(workbook, currentResult.nameImage, currentPredictions, numLineToWriteCurrentConcentration, numFirstColPredictions);

                numLineToWriteCurrentConcentration++;
            }
        }
        numLineToWrite = numLineToWriteCurrentConcentration;
    }

    writeUsedPredictionCoeffs(workbook, coefficients);
    writeUsedCutoffsValues(workbook, cutoffsValues);
}

Results ConcentrationPredictor::computeMeansFromResults(std::vector<Results> allResults)
{
    Results means;

    means.proxNoise = { std::accumulate(allResults.begin(), allResults.end(), 0.0, [](double sum, Results& currentResult) {
        return sum + std::get<0>(currentResult.proxNoise);
    }) / allResults.size(), 0 };

    means.expoFitProxNoise = { std::accumulate(allResults.begin(), allResults.end(), 0.0, [](double sum, Results& currentResult) {
        return sum + std::get<0>(currentResult.expoFitProxNoise);
    }) / allResults.size(), 0 };

    means.poly2ProxNoise = { std::accumulate(allResults.begin(), allResults.end(), 0.0, [](double sum, Results& currentResult) {
        return sum + std::get<0>(currentResult.poly2ProxNoise);
    }) / allResults.size(), 0 };

    means.convolProxNoise = { std::accumulate(allResults.begin(), allResults.end(), 0.0, [](double sum, Results& currentResult) {
        return sum + std::get<0>(currentResult.convolProxNoise);
    }) / allResults.size(), 0 };

    means.convolExpoFitProxNoise = { std::accumulate(allResults.begin(), allResults.end(), 0.0, [](double sum, Results& currentResult) {
        return sum + std::get<0>(currentResult.convolExpoFitProxNoise);
    }) / allResults.size(), 0 };

    means.convolPoly2ProxNoise = { std::accumulate(allResults.begin(), allResults.end(), 0.0, [](double sum, Results& currentResult) {
        return sum + std::get<0>(currentResult.convolPoly2ProxNoise);
    }) / allResults.size(), 0 };

    return means;
}

std::vector<MeansForLinearRegression> ConcentrationPredictor::computeAllMeansToFit(std::map<double, std::vector<std::vector<Results>>> allImagesResults,
                                                                                   int numberOfTL)
{
    std::vector<MeansForLinearRegression> calibrationCurvesData(numberOfTL);

    for(auto& element : allImagesResults)
    {
        double concentration = element.first;
        if(concentration >= 1000)
        continue;

        std::vector<Results> currentConcentrationMeans;

        for(int numTL = 0; numTL < element.second.size(); numTL++)
        {
            Results currentTLMeans = computeMeansFromResults(element.second[numTL]);

            calibrationCurvesData[numTL].proxNoiseMeansToFit[concentration] = std::get<0>(currentTLMeans.proxNoise);
            calibrationCurvesData[numTL].expoFitMeansToFit[concentration] = std::get<0>(currentTLMeans.expoFitProxNoise);
            calibrationCurvesData[numTL].poly2MeansToFit[concentration] = std::get<0>(currentTLMeans.poly2ProxNoise);
            calibrationCurvesData[numTL].convolBrutMeansToFit[concentration] = std::get<0>(currentTLMeans.convolProxNoise);
            calibrationCurvesData[numTL].convolExpoFitMeansToFit[concentration] = std::get<0>(currentTLMeans.convolExpoFitProxNoise);
            calibrationCurvesData[numTL].convolPoly2MeansToFit[concentration] = std::get<0>(currentTLMeans.convolPoly2ProxNoise);
        }
    }

    return calibrationCurvesData;
}

std::vector<CalibrationCurvesCoefficients>
ConcentrationPredictor::computeAllLinearRegresssionCoefficients(std::vector<MeansForLinearRegression> calibrationCurvesData)
{
    std::vector<CalibrationCurvesCoefficients> allTLCoefficients;
    for(auto& data : calibrationCurvesData)
    {
        LinearRegressionCoefficients initialeValues = {1., 0.};

        auto [proxNoiseBrutLinear, proxNoiseBrutLinearFitScore] = CurveFitter::computeLinearRegressionFit(data.proxNoiseMeansToFit, initialeValues);
        auto [expoFitLinear, expoFitLinearFitScore] = CurveFitter::computeLinearRegressionFit(data.expoFitMeansToFit, initialeValues);
        auto [poly2Linear, poly2LinearFitScore] = CurveFitter::computeLinearRegressionFit(data.poly2MeansToFit, initialeValues);
        auto [convolBrutLinear, convolBrutLinearFitScore] = CurveFitter::computeLinearRegressionFit(data.convolBrutMeansToFit, initialeValues);
        auto [convolExpoFitLinear, convolExpoFitLinearFitScore] = CurveFitter::computeLinearRegressionFit(data.convolExpoFitMeansToFit, initialeValues);
        auto [convolPoly2Linear, convolPoly2LinearFitScore] = CurveFitter::computeLinearRegressionFit(data.convolPoly2MeansToFit, initialeValues);

        CalibrationCurvesCoefficients currentTLCoefficients = {
                .proxNoiseBrutLinear = proxNoiseBrutLinear,
                .expoFitLinear = expoFitLinear,
                .poly2Linear = poly2Linear,

                .convolBrutLinear = convolBrutLinear,
                .convolExpoFitLinear = convolExpoFitLinear,
                .convolPoly2Linear = convolPoly2Linear,

                .proxNoiseBrutLinearFitScore = proxNoiseBrutLinearFitScore,
                .expoFitLinearFitScore = expoFitLinearFitScore,
                .poly2LinearFitScore = poly2LinearFitScore,

                .convolBrutLinearFitScore = convolBrutLinearFitScore,
                .convolExpoFitLinearFitScore = convolExpoFitLinearFitScore,
                .convolPoly2LinearFitScore = convolPoly2LinearFitScore
        };

        allTLCoefficients.push_back(currentTLCoefficients);
    }

    return  allTLCoefficients;
}

std::string ConcentrationPredictor::constructSigmoideCoefficientsString(SigmoideCoefficients coefficients)
{
    return std::to_string(coefficients.Bottom) + " " + std::to_string(coefficients.Top) + " " + std::to_string(coefficients.IC50) + " " + std::to_string(coefficients.HillSlope);
}

std::string ConcentrationPredictor::constructLinearRegressionCoefficientsString(LinearRegressionCoefficients coefficients)
{
    return std::to_string(coefficients.A) + " " + std::to_string(coefficients.B);
}