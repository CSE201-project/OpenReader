//
// Created by KarimSadki on 04/01/2021.
//

#ifndef PICA_MONOPLEXUSETESTANALYZER_H
#define PICA_MONOPLEXUSETESTANALYZER_H

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "ExcelWriter.h"
#include "monoplex/algos/AlgoExponantialFit.h"
#include "monoplex/algos/AlgoTestMinusProxNoise.h"
#include "monoplex/algos/AlgoPoly2.h"

struct Algorithms{
    AlgoExponantialFit algoExponantialFit;
    AlgoTestMinusProxNoise algoTestMinusProxNoise;
    AlgoPoly2 algoPoly2;
    AlgoProxNoise algoProxNoise;

    void setConfig(ProcessorConfig processorConfig)
    {
        algoExponantialFit.setConfig(processorConfig);
        algoPoly2.setConfig(processorConfig);
    }
};

class MonoplexUseTestAnalyzer {
public:
    MonoplexUseTestAnalyzer(const std::vector<std::string>& paramsWithoutImagePath,
                            const std::filesystem::path& croppedFolderPath,
                            const std::filesystem::path& templateExcelFilePath,
                            const std::filesystem::path& excelFilePath,
                            const std::filesystem::path& excelFileCalibrationCurves);
    void processUseTestAnalyze();
private:
    ParamsToAnalyze _params;

    AllProfiles computeAllProfiles(std::map<double, std::vector<std::string>>& sortedNamesMap);
    std::tuple<TImageProfile, TImageProfile, uint32_t, double, double> cutAndFitProfile(TImageProfile& profile,
                                                                                        uint32_t controlLine,
                                                                                        uint32_t testLine,
                                                                                        ParamsToAnalyze params,
                                                                                        Algorithms& algorithms);
    std::vector<std::string> searchAllCroppedImagesNames(std::filesystem::path croppedFolderPath);
    Results computeResults(ProcessorConfig processorConfig,
                           const AllBestParamsData& bestParamsData,
                           AllProfiles& allProfiles,
                           uint32_t numTL,
                           uint32_t indexOfProfile);

    void analyzeImage(const cv::Mat &cropped,
                      double concentration,
                      const std::string &nameImage,
                      uint32_t numImage,
                      uint32_t numTL,
                      OpenXLSX::XLWorkbook &workbook,
                      const AllBestParamsData &bestParamsData,
                      AllProfiles &allProfiles,
                      std::map<double, std::vector<std::vector<Results>>> &allImagesResults);

    void saveImageWithROI(OpenXLSX::XLWorkbook& workbook,
                          const cv::Mat cropped,
                          uint32_t numImage,
                          const std::string& nameOriginalImage,
                          uint32_t controlLine,
                          std::vector<uint32_t>& testLines,
                          ColorChannel channel);
    static ResultsByAlgo sortResultsByAlgo(std::map<double, std::vector<std::vector<Results>>> allImagesResults, int numTL);
};

#endif //PICA_MONOPLEXUSETESTANALYZER_H