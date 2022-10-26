//
// Created by KarimSadki on 06/08/2021.
//

#ifndef PICA_ALGOMULTIPLEXCONVOLBRUT_H
#define PICA_ALGOMULTIPLEXCONVOLBRUT_H

#include "multiplex/pattern/PatternManager.h"
#include "common/imageProcessing/ImagePreProcessing.h"

struct AlgoMultiplexResult{
    std::map<SpotType, std::map<ColumnIndex, double>> ColumnsResults;
    std::map<SpotType, double> GlobalResults;
};

class AlgoMultiplexConvolBrut {
public:
    AlgoMultiplexConvolBrut(uint32_t heightROIAroundSpot, uint32_t widthROIAroundSpot, uint32_t radiusSpot);

    AlgoMultiplexResult processMultiplexAnalyze(const cv::Mat& image, GridOfSpots gridOfSpots);
    void setWidthGaussianToConvol(uint32_t newWidthGaussian);

private:
    uint32_t _heightROIAroundSpot;
    uint32_t _widthROIAroundSpot;
    uint32_t _radiusSpot;
    uint32_t _widthGaussianToConvol;

    double computeSpotBrutResult(const cv::Mat& image, cv::Point center);
};

#endif //PICA_ALGOMULTIPLEXCONVOLBRUT_H
