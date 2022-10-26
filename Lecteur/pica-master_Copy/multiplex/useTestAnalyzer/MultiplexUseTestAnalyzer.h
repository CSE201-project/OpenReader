//
// Created by KarimSadki on 04/01/2021.
//

#ifndef PICA_MULTIPLEXUSETESTANALYZER_H
#define PICA_MULTIPLEXUSETESTANALYZER_H

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "ExcelWriter.h"

class MultiplexUseTestAnalyzer {
public:
    MultiplexUseTestAnalyzer(const std::vector<std::string>& paramsWithoutImagePath,
                             const std::filesystem::path& croppedFolderPath,
                             const std::filesystem::path& templateExcelFilePath,
                             const std::filesystem::path& excelFilePath,
                             const std::filesystem::path& patternFilePath,
                             const std::filesystem::path& gridsLocationsFilePath);

    void processUseTestAnalyze();
private:
    ParamsMultiplex _params;
    PatternManager _patternManager;
    std::vector<SpotLocation> _landmarks;

    std::vector<std::string> searchAllCroppedImagesNames(std::filesystem::path croppedFolderPath);
    void saveImage(std::filesystem::path folder, const std::string& nameImage, const cv::Mat& image);
    std::map<SpotType, cv::Scalar> generateColorMap(std::set<SpotType> spotsSpecies);
    void saveImageWithROI(std::set<SpotType> spotsSpecies,
                          double widthNoiseROI,
                          double widthTotalSquare,
                          ImageInformations imageInformations,
                          ColorChannel channel,
                          GridOfSpots gridOfSpots);
    MultiplexResults computeResults(GridOfSpots gridOfSpots,
                                    const cv::Mat& cropped);
    void analyzeImage(const ImageInformations& imageInformations,
                      OpenXLSX::XLWorkbook& workbook);
    void saveImageBrutData(OpenXLSX::XLWorkbook& workbook,
                           const GridOfSpots& gridOfSpots,
                           double widthNoiseROI,
                           double widthTotalSquare,
                           const ImageInformations& imageInformations);
};

#endif //PICA_MULTIPLEXUSETESTANALYZER_H