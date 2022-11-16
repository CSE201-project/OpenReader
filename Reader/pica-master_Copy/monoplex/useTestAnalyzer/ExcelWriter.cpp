//
// Created by KarimSadki on 08/04/2021.
//

#include <filesystem>
#include <cmath>

#include <OpenXLSX.hpp>
#include "ExcelWriter.h"

double searchConcentration(const std::string& name)
{
    int lastUnderscorePosition = name.find_last_of('_');
    std::string nameWithoutImageNumber(name.begin(), name.begin() + lastUnderscorePosition);
    int beforeLastUnderscorePosition = nameWithoutImageNumber.find_last_of('_');
    int sizeOfUnit = 4;

    std::string concentration(name.begin() + beforeLastUnderscorePosition + 1,
                              name.begin() + lastUnderscorePosition - sizeOfUnit);

    return std::stod(concentration);
}

std::map<double, std::vector<std::string>> initConcentrationList(std::vector<std::string> croppedImagesNames)
{
    std::map<double, std::vector<std::string>> sortedNamesMap;

    for(auto& name : croppedImagesNames)
        sortedNamesMap[searchConcentration(name)].push_back(name);

    return sortedNamesMap;
}

void writeAdjustedProfile(OpenXLSX::XLWorkbook& workbook,
                          std::vector<double> stripProfile,
                          uint32_t numImage,
                          const std::string& nameImage,
                          const std::vector<uint32_t>& allCLPos,
                          uint32_t controlLine)
{
    std::string sheetName = "ProfilsBrutsRecales";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    int lineNumber = 1 + numImage;
    std::string cellNameImage = "A"+ std::to_string(lineNumber);
    worksheet.cell(OpenXLSX::XLCellReference(cellNameImage)).value() = nameImage;

    uint32_t maxCL = *std::max_element(allCLPos.begin(), allCLPos.end());
    uint32_t shift = maxCL - controlLine;

    for(int i = 0; i < shift; i++)
        stripProfile.insert(stripProfile.begin(),std::numeric_limits<double>::quiet_NaN());

    for(uint32_t i = 0; i < stripProfile.size(); i++)
    {
        if(std::isnan(stripProfile[i]))
            worksheet.cell(OpenXLSX::XLCellReference(lineNumber, 2 + i)).value() = "";
        else
            worksheet.cell(OpenXLSX::XLCellReference(lineNumber, 2 + i)).value() = stripProfile[i];
    }
}

void writeConcentrationList(OpenXLSX::XLWorkbook& workbook, const std::map<double, std::vector<std::string>>& sortedNamesMap)
{
    std::string sheetName = "Calculs T1a";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    uint8_t maxNumberConcentrationInExcel = 10;
    uint8_t acc = 0;
    for(auto& element : sortedNamesMap)
    {
        if(acc + 1 > maxNumberConcentrationInExcel)
            break;
        int lineNumber = 6 + acc;
        std::string cellName = "S"+ std::to_string(lineNumber);

        worksheet.cell(OpenXLSX::XLCellReference(cellName)).value() = element.first;
        acc++;
    }
}


void writeSummaryHeader(OpenXLSX::XLWorkbook& workbook,
                        const std::filesystem::path& croppedFolderPath,
                        const std::string& appVersion,
                        const std::string& currentDate)
{
    std::string sheetName = "Summary";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    worksheet.cell(OpenXLSX::XLCellReference("C4")).value() = croppedFolderPath.string();
    worksheet.cell(OpenXLSX::XLCellReference("C3")).value() = currentDate;
    worksheet.cell(OpenXLSX::XLCellReference("G3")).value() = appVersion;
}

void writeCalibrationParams(OpenXLSX::XLWorkbook& workbook, ParamsToAnalyze params)
{
    std::string sheetName = "Params";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    worksheet.cell(OpenXLSX::XLCellReference("B3")).value() = 4;
    worksheet.cell(OpenXLSX::XLCellReference("B4")).value() = params.measure;
    worksheet.cell(OpenXLSX::XLCellReference("B5")).value() = std::filesystem::path(params.proto).string();
    worksheet.cell(OpenXLSX::XLCellReference("B6")).value() = params.cassette;

    if(params.isVisible)
        worksheet.cell(OpenXLSX::XLCellReference("B7")).value() = "Visible";
    else
        worksheet.cell(OpenXLSX::XLCellReference("B7")).value() = "UV";

    worksheet.cell(OpenXLSX::XLCellReference("B8")).value() = params.gamme;
    worksheet.cell(OpenXLSX::XLCellReference("B9")).value() = params.nanoParticules;
    worksheet.cell(OpenXLSX::XLCellReference("B10")).value() = params.channelStr;

    worksheet.cell(OpenXLSX::XLCellReference("B13")).value() = params.theoricDistanceCLtoTLMm;
    worksheet.cell(OpenXLSX::XLCellReference("B15")).value() = params.referenceNumberOfPixels;
    worksheet.cell(OpenXLSX::XLCellReference("B17")).value() = params.proxNoiseTLSize;
    worksheet.cell(OpenXLSX::XLCellReference("B18")).value() = params.proxNoiseNoiseSize;

    worksheet.cell(OpenXLSX::XLCellReference("B23")).value() = params.maskLengthMm;
    worksheet.cell(OpenXLSX::XLCellReference("B26")).value() = params.lengthROIMm;
    worksheet.cell(OpenXLSX::XLCellReference("B27")).value() = params.rectangleWidthMm;
}

void insertResult(OpenXLSX::XLWorksheet& worksheet, const std::string& cellName, double result)
{

    if(std::isnan(result) || std::isinf(result))
        worksheet.cell(OpenXLSX::XLCellReference(cellName)).value() = "";
    else
        worksheet.cell(OpenXLSX::XLCellReference(cellName)).value() = result;
}

void insertResult(OpenXLSX::XLWorksheet& worksheet, int numLine, int numCol, double result)
{

    if(std::isnan(result) || std::isinf(result))
        worksheet.cell(OpenXLSX::XLCellReference(numLine, numCol)).value() = "";
    else
        worksheet.cell(OpenXLSX::XLCellReference(numLine, numCol)).value() = result;
}

void writeResults(OpenXLSX::XLWorkbook& workbook,
                  Results results,
                  const std::string& nameImage,
                  uint32_t numImage,
                  uint32_t numTL,
                  double concentration)
{
    std::string firstSheetName = "Calculs T1a";
    std::string currentSheetNameA = "Calculs T" + std::to_string(numTL) + "a";
    std::string currentSheetNameB = "Calculs T" + std::to_string(numTL) + "b";

    OpenXLSX::XLWorksheet firstWorksheet = workbook.worksheet(firstSheetName);
    OpenXLSX::XLWorksheet currentWorksheetA = workbook.worksheet(currentSheetNameA);
    OpenXLSX::XLWorksheet currentWorksheetB = workbook.worksheet(currentSheetNameB);

    std::string lineNumber = std::to_string(5 + numImage);
    currentWorksheetA.cell(OpenXLSX::XLCellReference("B" + lineNumber)).value() = nameImage;
    currentWorksheetA.cell(OpenXLSX::XLCellReference("C" + lineNumber)).value() = concentration;
    currentWorksheetB.cell(OpenXLSX::XLCellReference("B" + lineNumber)).value() = nameImage;
    currentWorksheetB.cell(OpenXLSX::XLCellReference("C" + lineNumber)).value() = concentration;

    insertResult(currentWorksheetA, "D" + lineNumber, results.rawTest);
    insertResult(currentWorksheetA, "E" + lineNumber, results.rawControl);
    insertResult(currentWorksheetA, "F" + lineNumber, results.rawNoise);
    insertResult(currentWorksheetA, "G" + lineNumber, std::get<0>(results.proxNoise));
    insertResult(currentWorksheetA, "H" + lineNumber, std::get<0>(results.expoFitProxNoise));
    insertResult(currentWorksheetA, "I" + lineNumber, std::get<0>(results.poly2ProxNoise));
    insertResult(currentWorksheetA, "J" + lineNumber, std::get<0>(results.testMinusProxNoise));
    insertResult(currentWorksheetA, "K" + lineNumber, std::get<0>(results.convolProxNoise));
    insertResult(currentWorksheetA, "L" + lineNumber, std::get<0>(results.convolExpoFitProxNoise));
    insertResult(currentWorksheetA, "M" + lineNumber, std::get<0>(results.convolPoly2ProxNoise));
    insertResult(currentWorksheetA, "N" + lineNumber, std::get<0>(results.convolTestMinusProxNoise));

    insertResult(currentWorksheetB, "D" + lineNumber, results.rawTest);
    insertResult(currentWorksheetB, "E" + lineNumber, results.rawControl);
    insertResult(currentWorksheetB, "F" + lineNumber, results.rawNoise);
    insertResult(currentWorksheetB, "G" + lineNumber, std::get<0>(results.proxNoiseBestParams));
    insertResult(currentWorksheetB, "H" + lineNumber, std::get<0>(results.expoFitProxNoiseBestParams));
    insertResult(currentWorksheetB, "I" + lineNumber, std::get<0>(results.poly2ProxNoiseBestParams));
    insertResult(currentWorksheetB, "J" + lineNumber, std::get<0>(results.testMinusProxNoiseBestParams));
    insertResult(currentWorksheetB, "K" + lineNumber, std::get<0>(results.convolProxNoiseBestParams));
    insertResult(currentWorksheetB, "L" + lineNumber, std::get<0>(results.convolExpoFitProxNoiseBestParams));
    insertResult(currentWorksheetB, "M" + lineNumber, std::get<0>(results.convolPoly2ProxNoiseBestParams));
    insertResult(currentWorksheetB, "N" + lineNumber, std::get<0>(results.convolTestMinusProxNoiseBestParams));

}

void writeFitCoefficients(OpenXLSX::XLWorkbook& workbook,
                          Results results,
                          const std::string& nameImage,
                          uint32_t numImage,
                          uint32_t numTL,
                          double concentration)
{
    std::string currentSheetName = "ExtraData TL" + std::to_string(numTL);

    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    std::string lineNumber = std::to_string(2 + numImage);
    currentWorksheet.cell(OpenXLSX::XLCellReference("B" + lineNumber)).value() = nameImage;
    currentWorksheet.cell(OpenXLSX::XLCellReference("C" + lineNumber)).value() = concentration;
    insertResult(currentWorksheet, "E" + lineNumber, std::get<1>(results.expoFitProxNoise));
    insertResult(currentWorksheet, "F" + lineNumber, std::get<1>(results.poly2ProxNoise));
}

void writeProxNoiseGrids(std::vector<std::map<ProxNoiseParams, double>> allProxNoiseLOD,
                         OpenXLSX::XLWorkbook& workbook,
                         int biggestConcentration)
{
    std::string currentSheetName = "Summary";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    for(int i = 0; i < allProxNoiseLOD.size(); i++)
    {
        for(auto& element : allProxNoiseLOD[i])
        {
            std::tuple<double, double> localisation = element.first.localisationInExcel;
            int numColumn = std::get<0>(localisation);
            int numLine = std::get<1>(localisation);

            if(element.second < 0)
                currentWorksheet.cell(OpenXLSX::XLCellReference(numLine, numColumn)).value() = ">" + std::to_string(biggestConcentration);
            else
                currentWorksheet.cell(OpenXLSX::XLCellReference(numLine, numColumn)).value() = element.second;
        }
    }
}

std::string constructProxNoiseParamsString(ProxNoiseParams proxNoiseParams)
{
    std::string proxNoiseParamsString = std::to_string(std::get<0>(proxNoiseParams.proxNoiseLenghtsMm)).substr(0, 3)
                                        + " - "
                                        + std::to_string(std::get<1>(proxNoiseParams.proxNoiseLenghtsMm)).substr(0, 3)
                                        + " --- "
                                        + std::to_string(std::lround(proxNoiseParams.percentageLODToWrite)) + "%";
    return proxNoiseParamsString;
}

void writeAllBestParams(OpenXLSX::XLWorkbook& workbook,
                        AllBestParamsData& bestParamsData,
                        int biggestConcentration,
                        bool isLightVersion)
{
    std::string currentSheetName = "Summary";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    uint32_t numberOfTL = bestParamsData.allProxNoiseBestParams.size();
    for(int i = 0; i < numberOfTL; i++)
    {
        ProxNoiseParams proxNoiseBestParams = bestParamsData.allProxNoiseBestParams[i];
        ProxNoiseParams convolProxNoiseBestParams = bestParamsData.allConvolProxNoiseBestParams[i];

        std::string proxNoiseLenghts = constructProxNoiseParamsString(proxNoiseBestParams);
        std::string convolProxNoiseLenghts = constructProxNoiseParamsString(convolProxNoiseBestParams);

        int numLineFirstBestParams = 15 + (i * 20);
        currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams))).value() = proxNoiseLenghts;
        currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 4))).value() = convolProxNoiseLenghts;

        if(!isLightVersion)
        {
            ProxNoiseParams expoFitProxNoiseBestParams  = bestParamsData.allExpoFitBestParams[i];
            ProxNoiseParams poly2ProxNoiseBestParams  = bestParamsData.allPoly2BestParams[i];
            ProxNoiseParams testMinusProxNoiseBestParams  = bestParamsData.allTestMinusNoiseBestParams[i];

            ProxNoiseParams convolExpoFitProxNoiseBestParams  = bestParamsData.allConvolExpoFitBestParams[i];
            ProxNoiseParams convolPoly2ProxNoiseBestParams  = bestParamsData.allConvolPoly2BestParams[i];
            ProxNoiseParams convolTestMinusNoiseBestParams  = bestParamsData.allConvolTestMinusNoiseBestParams[i];

            std::string expoFitProxNoiseLenghts = constructProxNoiseParamsString(expoFitProxNoiseBestParams);
            std::string poly2ProxNoiseLenghts = constructProxNoiseParamsString(poly2ProxNoiseBestParams);
            std::string testMinusProxNoiseLenghts = constructProxNoiseParamsString(testMinusProxNoiseBestParams);

            std::string convolExpoFitProxNoiseLenghts = constructProxNoiseParamsString(convolExpoFitProxNoiseBestParams);
            std::string convolPoly2ProxNoiseLenghts = constructProxNoiseParamsString(convolPoly2ProxNoiseBestParams);
            std::string convolTestMinusNoiseLenghts = constructProxNoiseParamsString(convolTestMinusNoiseBestParams);

            currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 1))).value() = expoFitProxNoiseLenghts;
            currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 2))).value() = poly2ProxNoiseLenghts;
            currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 3))).value() = testMinusProxNoiseLenghts;
            currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 5))).value() = convolExpoFitProxNoiseLenghts;
            currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 6))).value() = convolPoly2ProxNoiseLenghts;
            currentWorksheet.cell(OpenXLSX::XLCellReference("F" + std::to_string(numLineFirstBestParams + 7))).value() = convolTestMinusNoiseLenghts;
        }
    }
}

void writeROIFolderPath(OpenXLSX::XLWorkbook& workbook,
                        std::filesystem::path& pathROI)
{
    std::string currentSheetName = "Summary";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);
    currentWorksheet.cell(OpenXLSX::XLCellReference("B72")).value() = pathROI.string();
}

CalibrationCurvesCoefficients readCoefficientsFromExcel(OpenXLSX::XLWorkbook& workbook)
{
    std::string currentSheetName = "Coeffs Algos";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    SigmoideCoefficients coeffsSigmoideProxNoiseCalibrationCurves = {
            std::stod(currentWorksheet.cell("B2").value().asString()),
            std::stod(currentWorksheet.cell("B3").value().asString()),
            std::stod(currentWorksheet.cell("B4").value().asString()),
            std::stod(currentWorksheet.cell("B5").value().asString())
    };

    SigmoideCoefficients coeffsSigmoideExpoFitCalibrationCurves = {
            std::stod(currentWorksheet.cell("C2").value().asString()),
            std::stod(currentWorksheet.cell("C3").value().asString()),
            std::stod(currentWorksheet.cell("C4").value().asString()),
            std::stod(currentWorksheet.cell("C5").value().asString())
    };

    SigmoideCoefficients coeffsSigmoidePoly2CalibrationCurves = {
            std::stod(currentWorksheet.cell("D2").value().asString()),
            std::stod(currentWorksheet.cell("D3").value().asString()),
            std::stod(currentWorksheet.cell("D4").value().asString()),
            std::stod(currentWorksheet.cell("D5").value().asString())
    };

    SigmoideCoefficients coeffsSigmoideTestMinusCalibrationCurves = {
            std::stod(currentWorksheet.cell("E2").value().asString()),
            std::stod(currentWorksheet.cell("E3").value().asString()),
            std::stod(currentWorksheet.cell("E4").value().asString()),
            std::stod(currentWorksheet.cell("E5").value().asString())
    };

    SigmoideCoefficients coeffsConvolSigmoideTestMinusCalibrationCurves = {
            std::stod(currentWorksheet.cell("F2").value().asString()),
            std::stod(currentWorksheet.cell("F3").value().asString()),
            std::stod(currentWorksheet.cell("F4").value().asString()),
            std::stod(currentWorksheet.cell("F5").value().asString())
    };

    LinearRegressionCoefficients coeffsLinearProxNoise = {
            std::stod(currentWorksheet.cell("B9").value().asString()),
            std::stod(currentWorksheet.cell("B10").value().asString())
    };

    LinearRegressionCoefficients coeffsLinearExpoFit = {
            std::stod(currentWorksheet.cell("C9").value().asString()),
            std::stod(currentWorksheet.cell("C10").value().asString())
    };

    LinearRegressionCoefficients coeffsLinearPoly2 = {
            std::stod(currentWorksheet.cell("D9").value().asString()),
            std::stod(currentWorksheet.cell("D10").value().asString())
    };

    LinearRegressionCoefficients coeffsLinearConvolBrut = {
            std::stod(currentWorksheet.cell("E9").value().asString()),
            std::stod(currentWorksheet.cell("E10").value().asString())
    };

    LinearRegressionCoefficients coeffsLinearConvolExpoFit = {
            std::stod(currentWorksheet.cell("F9").value().asString()),
            std::stod(currentWorksheet.cell("F10").value().asString())
    };

    LinearRegressionCoefficients coeffsLinearConvolPoly2 = {
            std::stod(currentWorksheet.cell("G9").value().asString()),
            std::stod(currentWorksheet.cell("G10").value().asString())
    };

    CalibrationCurvesCoefficients allCoefficients = {
            .proxNoiseBrutSigmoide = coeffsSigmoideProxNoiseCalibrationCurves,
            .expoFitSigmoide = coeffsSigmoideExpoFitCalibrationCurves,
            .poly2Sigmoide = coeffsSigmoidePoly2CalibrationCurves,
            .testMinusProxNoiseSigmoide = coeffsSigmoideTestMinusCalibrationCurves,
            .convolTestMinusProxNoiseSigmoide = coeffsConvolSigmoideTestMinusCalibrationCurves,
            .proxNoiseBrutLinear = coeffsLinearProxNoise,
            .expoFitLinear = coeffsLinearExpoFit,
            .poly2Linear = coeffsLinearPoly2,
            .convolBrutLinear = coeffsLinearConvolBrut,
            .convolExpoFitLinear = coeffsLinearConvolExpoFit,
            .convolPoly2Linear = coeffsLinearConvolPoly2
    };

    return allCoefficients;
}

void writeConcentrationPredictions(OpenXLSX::XLWorkbook& workbook,
                                   const std::string& imageName,
                                   Predictions currentPredictions,
                                   int numLineToWrite,
                                   int numFirstColPredictions)
{
    std::string predictionsSheetName = "Predictions concentrations";
    OpenXLSX::XLWorksheet predictionsWorksheet = workbook.worksheet(predictionsSheetName);

    predictionsWorksheet.cell(OpenXLSX::XLCellReference(numLineToWrite, numFirstColPredictions)).value() = imageName;
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 1, currentPredictions.realConcentrations);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 2, currentPredictions.concentrationSigmoideBrut);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 4, currentPredictions.concentrationSigmoideExpoFit);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 6, currentPredictions.concentrationSigmoidePoly2);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 8, currentPredictions.concentrationSigmoideTestMinusProxNoise);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 10, currentPredictions.concentrationSigmoideConvolTestMinusProxNoise);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 12, currentPredictions.concentrationLinearBrut);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 14, currentPredictions.concentrationLinearExpoFit);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 16, currentPredictions.concentrationLinearPoly2);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 18, currentPredictions.concentrationLinearConvolBrut);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 20, currentPredictions.concentrationLinearConvolExpoFit);
    insertResult(predictionsWorksheet, numLineToWrite, numFirstColPredictions + 22, currentPredictions.concentrationLinearConvolPoly2);
}

void writeUsedPredictionCoeffs(OpenXLSX::XLWorkbook& workbook,
                               const CalibrationCurvesCoefficients coefficients)
{
    std::string currentSheetName = "Coeffs calibration";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    currentWorksheet.cell(OpenXLSX::XLCellReference("B4")).value() = coefficients.proxNoiseBrutSigmoide.Bottom;
    currentWorksheet.cell(OpenXLSX::XLCellReference("B5")).value() = coefficients.proxNoiseBrutSigmoide.Top;
    currentWorksheet.cell(OpenXLSX::XLCellReference("B6")).value() = coefficients.proxNoiseBrutSigmoide.IC50;
    currentWorksheet.cell(OpenXLSX::XLCellReference("B7")).value() = coefficients.proxNoiseBrutSigmoide.HillSlope;

    currentWorksheet.cell(OpenXLSX::XLCellReference("C4")).value() = coefficients.expoFitSigmoide.Bottom;
    currentWorksheet.cell(OpenXLSX::XLCellReference("C5")).value() = coefficients.expoFitSigmoide.Top;
    currentWorksheet.cell(OpenXLSX::XLCellReference("C6")).value() = coefficients.expoFitSigmoide.IC50;
    currentWorksheet.cell(OpenXLSX::XLCellReference("C7")).value() = coefficients.expoFitSigmoide.HillSlope;

    currentWorksheet.cell(OpenXLSX::XLCellReference("D4")).value() = coefficients.poly2Sigmoide.Bottom;
    currentWorksheet.cell(OpenXLSX::XLCellReference("D5")).value() = coefficients.poly2Sigmoide.Top;
    currentWorksheet.cell(OpenXLSX::XLCellReference("D6")).value() = coefficients.poly2Sigmoide.IC50;
    currentWorksheet.cell(OpenXLSX::XLCellReference("D7")).value() = coefficients.poly2Sigmoide.HillSlope;

    currentWorksheet.cell(OpenXLSX::XLCellReference("E4")).value() = coefficients.testMinusProxNoiseSigmoide.Bottom;
    currentWorksheet.cell(OpenXLSX::XLCellReference("E5")).value() = coefficients.testMinusProxNoiseSigmoide.Top;
    currentWorksheet.cell(OpenXLSX::XLCellReference("E6")).value() = coefficients.testMinusProxNoiseSigmoide.IC50;
    currentWorksheet.cell(OpenXLSX::XLCellReference("E7")).value() = coefficients.testMinusProxNoiseSigmoide.HillSlope;

    currentWorksheet.cell(OpenXLSX::XLCellReference("F4")).value() = coefficients.convolTestMinusProxNoiseSigmoide.Bottom;
    currentWorksheet.cell(OpenXLSX::XLCellReference("F5")).value() = coefficients.convolTestMinusProxNoiseSigmoide.Top;
    currentWorksheet.cell(OpenXLSX::XLCellReference("F6")).value() = coefficients.convolTestMinusProxNoiseSigmoide.IC50;
    currentWorksheet.cell(OpenXLSX::XLCellReference("F7")).value() = coefficients.convolTestMinusProxNoiseSigmoide.HillSlope;


    currentWorksheet.cell(OpenXLSX::XLCellReference("B11")).value() = coefficients.proxNoiseBrutLinear.A;
    currentWorksheet.cell(OpenXLSX::XLCellReference("B12")).value() = coefficients.proxNoiseBrutLinear.B;

    currentWorksheet.cell(OpenXLSX::XLCellReference("C11")).value() = coefficients.expoFitLinear.A;
    currentWorksheet.cell(OpenXLSX::XLCellReference("C12")).value() = coefficients.expoFitLinear.B;

    currentWorksheet.cell(OpenXLSX::XLCellReference("D11")).value() = coefficients.poly2Linear.A;
    currentWorksheet.cell(OpenXLSX::XLCellReference("D12")).value() = coefficients.poly2Linear.B;

    currentWorksheet.cell(OpenXLSX::XLCellReference("E11")).value() = coefficients.convolBrutLinear.A;
    currentWorksheet.cell(OpenXLSX::XLCellReference("E12")).value() = coefficients.convolBrutLinear.B;

    currentWorksheet.cell(OpenXLSX::XLCellReference("F11")).value() = coefficients.convolExpoFitLinear.A;
    currentWorksheet.cell(OpenXLSX::XLCellReference("F12")).value() = coefficients.convolExpoFitLinear.B;

    currentWorksheet.cell(OpenXLSX::XLCellReference("G11")).value() = coefficients.convolPoly2Linear.A;
    currentWorksheet.cell(OpenXLSX::XLCellReference("G12")).value() = coefficients.convolPoly2Linear.B;
}

void writeAllComputedCoefficients(OpenXLSX::XLWorkbook& workbook,
                                  const std::vector<CalibrationCurvesCoefficients>& allTLCoefficients)
{
    std::string currentSheetName = "Coeffs calibration";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    int colToWrite = 2;

    for(int numTL = 0; numTL < allTLCoefficients.size(); numTL++)
    {
        CalibrationCurvesCoefficients currentTLCoeffs = allTLCoefficients[numTL];

        insertResult(currentWorksheet, 25, colToWrite, currentTLCoeffs.proxNoiseBrutLinear.A);
        insertResult(currentWorksheet, 26, colToWrite, currentTLCoeffs.proxNoiseBrutLinear.B);
        insertResult(currentWorksheet, 27, colToWrite, currentTLCoeffs.proxNoiseBrutLinearFitScore);

        insertResult(currentWorksheet, 25, colToWrite + 1, currentTLCoeffs.expoFitLinear.A);
        insertResult(currentWorksheet, 26, colToWrite + 1, currentTLCoeffs.expoFitLinear.B);
        insertResult(currentWorksheet, 27, colToWrite + 1, currentTLCoeffs.expoFitLinearFitScore);

        insertResult(currentWorksheet, 25, colToWrite + 2, currentTLCoeffs.poly2Linear.A);
        insertResult(currentWorksheet, 26, colToWrite + 2, currentTLCoeffs.poly2Linear.B);
        insertResult(currentWorksheet, 27, colToWrite + 2, currentTLCoeffs.poly2LinearFitScore);

        insertResult(currentWorksheet, 25, colToWrite + 3, currentTLCoeffs.convolBrutLinear.A);
        insertResult(currentWorksheet, 26, colToWrite + 3, currentTLCoeffs.convolBrutLinear.B);
        insertResult(currentWorksheet, 27, colToWrite + 3, currentTLCoeffs.convolBrutLinearFitScore);

        insertResult(currentWorksheet, 25, colToWrite + 4, currentTLCoeffs.convolExpoFitLinear.A);
        insertResult(currentWorksheet, 26, colToWrite + 4, currentTLCoeffs.convolExpoFitLinear.B);
        insertResult(currentWorksheet, 27, colToWrite + 4, currentTLCoeffs.convolExpoFitLinearFitScore);

        insertResult(currentWorksheet, 25, colToWrite + 5, currentTLCoeffs.convolPoly2Linear.A);
        insertResult(currentWorksheet, 26, colToWrite + 5, currentTLCoeffs.convolPoly2Linear.B);
        insertResult(currentWorksheet, 27, colToWrite + 5, currentTLCoeffs.convolPoly2LinearFitScore);

        colToWrite += 8;
    }
}

void writeUsedCutoffsValues(OpenXLSX::XLWorkbook& workbook,
                            const CalibrationCutoffsValues cutoffsValues)
{
    std::string currentSheetName = "Coeffs calibration";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    int numFirstLineToWrite = 15;

    for(int i = 0; i < cutoffsValues.proxNoiseCutoffs.size(); i++)
    {
        insertResult(currentWorksheet, numFirstLineToWrite + i, 2, cutoffsValues.proxNoiseCutoffs[i]);
        insertResult(currentWorksheet, numFirstLineToWrite + i, 7, cutoffsValues.proxNoiseCutoffs[i]);
    }


    for(int i = 0; i < cutoffsValues.expoFitCutoffs.size(); i++)
    {
        insertResult(currentWorksheet, numFirstLineToWrite + i, 3, cutoffsValues.expoFitCutoffs[i]);
        insertResult(currentWorksheet, numFirstLineToWrite + i, 8, cutoffsValues.expoFitCutoffs[i]);
    }


    for(int i = 0; i < cutoffsValues.poly2Cutoffs.size(); i++)
    {
        insertResult(currentWorksheet, numFirstLineToWrite + i, 4, cutoffsValues.poly2Cutoffs[i]);
        insertResult(currentWorksheet, numFirstLineToWrite + i, 9, cutoffsValues.poly2Cutoffs[i]);
    }


    for(int i = 0; i < cutoffsValues.testMinusProxNoiseCutoffs.size(); i++)
    {
        insertResult(currentWorksheet, numFirstLineToWrite + i, 5, cutoffsValues.testMinusProxNoiseCutoffs[i]);
        insertResult(currentWorksheet, numFirstLineToWrite + i, 10, cutoffsValues.testMinusProxNoiseCutoffs[i]);
    }

    for(int i = 0; i < cutoffsValues.convolBrutCutoffs.size(); i++)
        insertResult(currentWorksheet, numFirstLineToWrite + i, 11, cutoffsValues.convolBrutCutoffs[i]);

    for(int i = 0; i < cutoffsValues.convolExpoFitCutoffs.size(); i++)
        insertResult(currentWorksheet, numFirstLineToWrite + i, 12, cutoffsValues.convolExpoFitCutoffs[i]);

    for(int i = 0; i < cutoffsValues.convolPoly2Cutoffs.size(); i++)
        insertResult(currentWorksheet, numFirstLineToWrite + i, 13, cutoffsValues.convolPoly2Cutoffs[i]);

    for(int i = 0; i < cutoffsValues.convolTestMinusProxNoiseCutoffs.size(); i++)
        insertResult(currentWorksheet, numFirstLineToWrite + i, 14, cutoffsValues.convolTestMinusProxNoiseCutoffs[i]);

}

CalibrationCutoffsValues readCutoffsValuesFromExcel(OpenXLSX::XLWorkbook& workbook)
{
    std::string currentSheetName = "Coeffs Algos";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    std::vector<double> cutoffsProxNoiseBrut = {
            std::stod(currentWorksheet.cell("B13").value().asString()),
            std::stod(currentWorksheet.cell("B14").value().asString()),
            std::stod(currentWorksheet.cell("B15").value().asString())
    };

    std::vector<double> cutoffsExpoFit = {
            std::stod(currentWorksheet.cell("C13").value().asString()),
            std::stod(currentWorksheet.cell("C14").value().asString()),
            std::stod(currentWorksheet.cell("C15").value().asString())
    };

    std::vector<double> cutoffsPoly2 = {
            std::stod(currentWorksheet.cell("D13").value().asString()),
            std::stod(currentWorksheet.cell("D14").value().asString()),
            std::stod(currentWorksheet.cell("D15").value().asString())
    };

    std::vector<double> cutoffsTestMinusProxNoise = {
            std::stod(currentWorksheet.cell("E13").value().asString()),
            std::stod(currentWorksheet.cell("E14").value().asString()),
            std::stod(currentWorksheet.cell("E15").value().asString())
    };

    std::vector<double> cutoffsConvolBrut = {
            std::stod(currentWorksheet.cell("F13").value().asString()),
            std::stod(currentWorksheet.cell("F14").value().asString()),
            std::stod(currentWorksheet.cell("F15").value().asString())
    };

    std::vector<double> cutoffsConvolExpoFit = {
            std::stod(currentWorksheet.cell("G13").value().asString()),
            std::stod(currentWorksheet.cell("G14").value().asString()),
            std::stod(currentWorksheet.cell("G15").value().asString())
    };

    std::vector<double> cutoffsConvolPoly2 = {
            std::stod(currentWorksheet.cell("H13").value().asString()),
            std::stod(currentWorksheet.cell("H14").value().asString()),
            std::stod(currentWorksheet.cell("H15").value().asString())
    };

    std::vector<double> cutoffsConvolTestMinusProxNoise = {
            std::stod(currentWorksheet.cell("I13").value().asString()),
            std::stod(currentWorksheet.cell("I14").value().asString()),
            std::stod(currentWorksheet.cell("I15").value().asString())
    };

    return {cutoffsProxNoiseBrut, cutoffsExpoFit, cutoffsPoly2, cutoffsTestMinusProxNoise, cutoffsConvolBrut, cutoffsConvolExpoFit, cutoffsConvolPoly2, cutoffsConvolTestMinusProxNoise};
}

void writeFindTLInformations(OpenXLSX::XLWorkbook& workbook,
                             const std::string& name,
                             int numImage,
                             double differenceInMm,
                             int differenceInPx)
{
    std::string currentSheetName = "FindTL informations";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(currentSheetName);

    currentWorksheet.cell(OpenXLSX::XLCellReference(numImage + 1, 1)).value() = name;
    insertResult(currentWorksheet, numImage + 1, 2, differenceInMm);
    insertResult(currentWorksheet, numImage + 1, 3, differenceInPx);
}

void writeROCCurveValues(OpenXLSX::XLWorkbook& workbook, std::vector<ROCCurveParams> params, int numFirstCol)
{
    std::string sheetName = "ROC curves T2";
    OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(sheetName);

    for(int i = 0; i < params.size(); i++)
    {
        ROCCurveParams currentParams = params[i];

        currentWorksheet.cell(OpenXLSX::XLCellReference(i + 2, numFirstCol)).value() = currentParams.cutoff;
        currentWorksheet.cell(OpenXLSX::XLCellReference(i + 2, numFirstCol + 4)).value() = currentParams.TruePositive;
        currentWorksheet.cell(OpenXLSX::XLCellReference(i + 2, numFirstCol + 5)).value() = currentParams.TrueNegative;
        currentWorksheet.cell(OpenXLSX::XLCellReference(i + 2, numFirstCol + 6)).value() = currentParams.FalsePositive;
        currentWorksheet.cell(OpenXLSX::XLCellReference(i + 2, numFirstCol + 7)).value() = currentParams.FalseNegative;
    }
}

void writeOneAlgoRegulatoryData(OpenXLSX::XLWorkbook& workbook, RegulatoryData data, int numLine)
{
    std::string sheetName = "Summary";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    auto [smallestLowLevel, highestLowLevel] = data.lowLevelsConcentrationsBoundaries;

    worksheet.cell(OpenXLSX::XLCellReference(numLine, 19)).value() = std::to_string((int) smallestLowLevel) + " - " + std::to_string((int) highestLowLevel);
    if(data.RegulatoryLODConcentration < 0)
    {
        worksheet.cell(OpenXLSX::XLCellReference(numLine, 22)).value() = "No LOD among low level";
        worksheet.cell(OpenXLSX::XLCellReference(numLine, 23)).value() = "No LOD among low level";
    }
    else
    {
        worksheet.cell(OpenXLSX::XLCellReference(numLine, 22)).value() = data.RegulatoryLODConcentration;
        worksheet.cell(OpenXLSX::XLCellReference(numLine, 23)).value() = data.RegulatoryLOD;
    }

    worksheet.cell(OpenXLSX::XLCellReference(numLine, 24)).value() = data.RegulatoryLOB;
    worksheet.cell(OpenXLSX::XLCellReference(numLine, 25)).value() = data.RegulatoryLOB * 5;
}