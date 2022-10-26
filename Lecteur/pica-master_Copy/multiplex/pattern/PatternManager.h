//
// Created by KarimSadki on 04/01/2021.
//

#ifndef PICA_PATTERNMANAGER_H
#define PICA_PATTERNMANAGER_H

#include <map>
#include <set>
#include <opencv2/opencv.hpp>
#include <filesystem>

#include "MultiplexStructs.h"

class PatternManager{
public:
    Pattern pattern;

    PatternManager(std::filesystem::path patternFilePath);

    static std::vector<SpotLocation> initAllLandmarks(const std::filesystem::path& gridsLocationsFilePath, double referenceNumberOfPixels);
    GridOfSpots buildGridOfSpots(const cv::Size &size, const SpotLocation &landmark, double referenceNumberOfPixels);

    double getHorizontalPitch();
    double getVerticalPitch();

    double getDefaultHeightNoiseROI();
    double getDefaultWidthNoiseROI();

private:
    int _numberOfCols;
    double _horizontalPitch;
    double _verticalPitch;

    void initPattern(const std::filesystem::path& patternFilePath);
    void initField(const std::string& informationName, const std::string& numberStr);
};

#endif //PICA_PATTERNMANAGER_H
