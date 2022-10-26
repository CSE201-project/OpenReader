//
// Created by KarimSadki on 04/01/2021.
//

#ifndef PICA_CONCENTRATIONPREDICTOR_H
#define PICA_CONCENTRATIONPREDICTOR_H

#include "AnalyzeUseTestStructs.h"
#include "ExcelWriter.h"

class ConcentrationPredictor {
public:
    static void processConcentrationPrediction(OpenXLSX::XLWorkbook& workbook,
                                               const std::map<double, std::vector<std::vector<Results>>>& allImagesResults,
                                               const std::filesystem::path& excelFileCalibrationCurves);

    static void generateLinearRegressionCoefficients(OpenXLSX::XLWorkbook& workbook,
                                                     const std::map<double, std::vector<std::vector<Results>>>& allImagesResults);
private:
    static Predictions computePredictions(CalibrationCurvesCoefficients& coefficients,
                                          const Results& results,
                                          double realConcentration);

    static std::vector<MeansForLinearRegression> computeAllMeansToFit(std::map<double, std::vector<std::vector<Results>>> allImagesResults,
                                                                      int numberOfTL = 3);
    static std::vector<CalibrationCurvesCoefficients> computeAllLinearRegresssionCoefficients(std::vector<MeansForLinearRegression> calibrationCurvesData);
    static Results computeMeansFromResults(std::vector<Results> allResults);
    static std::string constructSigmoideCoefficientsString(SigmoideCoefficients coefficients);
    static std::string constructLinearRegressionCoefficientsString(LinearRegressionCoefficients coefficients);
};

#endif //PICA_CONCENTRATIONPREDICTOR_H
