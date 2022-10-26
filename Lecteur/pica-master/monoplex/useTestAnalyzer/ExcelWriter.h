//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/22.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef PICA_EXCELWRITER_H
#define PICA_EXCELWRITER_H

#include <climits>
#include <OpenXLSX.hpp>

#include "AnalyzeUseTestStructs.h"
#include "ROCCurveManager.h"

double searchConcentration(const std::string& name);
std::map<double, std::vector<std::string>> initConcentrationList(std::vector<std::string> croppedImagesNames);
void writeAdjustedProfile(OpenXLSX::XLWorkbook& workbook,
                          std::vector<double> stripProfile,
                          uint32_t numImage,
                          const std::string& nameImage,
                          const std::vector<uint32_t>& allCLPos,
                          uint32_t controlLine);
void writeConcentrationList(OpenXLSX::XLWorkbook& workbook, const std::map<double, std::vector<std::string>>& sortedNamesMap);
void writeSummaryHeader(OpenXLSX::XLWorkbook& workbook,
                        const std::filesystem::path& croppedFolderPath,
                        const std::string& appVersion,
                        const std::string& currentDate);
void writeCalibrationParams(OpenXLSX::XLWorkbook& workbook, ParamsToAnalyze params);
void insertResult(OpenXLSX::XLWorksheet& worksheet, const std::string& cellName, double result);
void writeResults(OpenXLSX::XLWorkbook& workbook,
                  Results results,
                  const std::string& nameImage,
                  uint32_t numImage,
                  uint32_t numTL,
                  double concentration);
void writeFitCoefficients(OpenXLSX::XLWorkbook& workbook,
                          Results results,
                          const std::string& nameImage,
                          uint32_t numImage,
                          uint32_t numTL,
                          double concentration);
void writeProxNoiseGrids(std::vector<std::map<ProxNoiseParams, double>> allProxNoiseLOD,
                         OpenXLSX::XLWorkbook& workbook,
                         int biggestConcentration);
void writeAllBestParams(OpenXLSX::XLWorkbook& workbook,
                        AllBestParamsData& bestParamsData,
                        int biggestConcentration,
                        bool isLightVersion);
void writeROIFolderPath(OpenXLSX::XLWorkbook& workbook,
                        std::filesystem::path& pathROI);
CalibrationCurvesCoefficients readCoefficientsFromExcel(OpenXLSX::XLWorkbook& workbook);
void writeConcentrationPredictions(OpenXLSX::XLWorkbook& workbook,
                                   const std::string& imageName,
                                   Predictions currentPredictions,
                                   int numLineToWrite,
                                   int numFirstColPredictions);
void writeUsedPredictionCoeffs(OpenXLSX::XLWorkbook& workbook,
                               const CalibrationCurvesCoefficients coefficients);
void writeAllComputedCoefficients(OpenXLSX::XLWorkbook& workbook,
                                  const std::vector<CalibrationCurvesCoefficients>& allTLCoefficients);
void writeUsedCutoffsValues(OpenXLSX::XLWorkbook& workbook,
                            const CalibrationCutoffsValues cutoffsValues);
CalibrationCutoffsValues readCutoffsValuesFromExcel(OpenXLSX::XLWorkbook& workbook);
void writeFindTLInformations(OpenXLSX::XLWorkbook& workbook,
                             const std::string& name,
                             int numImage,
                             double differenceInMm,
                             int differenceInPx);
void writeROCCurveValues(OpenXLSX::XLWorkbook& workbook, std::vector<ROCCurveParams> params, int numFirstCol);
void writeOneAlgoRegulatoryData(OpenXLSX::XLWorkbook& workbook, RegulatoryData data, int numLine);
#endif //PICA_EXCELWRITER_H
