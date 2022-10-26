//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/22.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef PICA_EXCELWRITER_H
#define PICA_EXCELWRITER_H

#include <vector>
#include <filesystem>
#include <climits>
#include <OpenXLSX.hpp>

#include "common/imageProcessing/ImageAnalyzer.h"
#include "multiplex/algos/AlgoMultiplexConvolBrut.h"

struct ParamsMultiplex{
    bool isVisible;
    std::string channelStr;
    ColorChannel channel;
    double referenceNumberOfPixels;
    double spotDiameter;

    std::string appVersion;
    std::string currentDate;

    std::string measure;
    std::string proto;
    std::string cassette;
    std::string focal;
    std::string nanoParticules;

    std::filesystem::path croppedFolderPath;
    std::filesystem::path templateExcelFilePath;
    std::filesystem::path excelFilePath;
    std::filesystem::path patternFilePath;
    std::filesystem::path gridsLocationsFilePath;
};

struct ImageInformations{
    cv::Mat cropped;
    std::string name;
    double concentration;
    SpotLocation landmark;
    uint32_t numImage;
};

struct MultiplexResults{
    AlgoMultiplexResult convolBrutResult;
};

std::map<double, std::vector<std::string>> initConcentrationListMultiplex(std::vector<std::string> croppedImagesNames);
void writeSummaryHeaderMultiplex(OpenXLSX::XLWorkbook& workbook,
                        const std::filesystem::path& croppedFolderPath,
                        const std::string& appVersion,
                        const std::string& currentDate,
                        const std::set<std::string> spotsSpecies);
void writeCalibrationParamsMultiplex(OpenXLSX::XLWorkbook& workbook, ParamsMultiplex params);
void writePatternInformations(OpenXLSX::XLWorkbook& workbook,
                              double theoricSpotDiameter,
                              double verticalSpacing,
                              double horizontalSpacing,
                              double heightNoiseROI,
                              double widthNoiseROI);
void writeConcentrationListMultiplex(OpenXLSX::XLWorkbook& workbook, const std::map<double, std::vector<std::string>>& sortedNamesMap);
void writeResults(OpenXLSX::XLWorkbook& workbook, MultiplexResults results, ImageInformations imageInformations);
void writeImageBrutData(OpenXLSX::XLWorkbook& workbook,
                        const std::vector<SpotLocation>& spotsLocations,
                        const std::vector<uint32_t>& totalSquareIntensities,
                        const std::vector<uint32_t>& spotsIntensities,
                        const std::vector<uint32_t>& noiseIntensities,
                        const std::string& name,
                        int numImage);

#endif //PICA_EXCELWRITER_H
