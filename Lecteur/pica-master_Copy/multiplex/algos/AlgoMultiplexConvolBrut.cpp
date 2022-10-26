//
// Created by KarimSadki on 06/08/2021.
//

#include <numeric>

#include "AlgoMultiplexConvolBrut.h"
#include "common/imageProcessing/ImageAnalyzer.h"

AlgoMultiplexConvolBrut::AlgoMultiplexConvolBrut(uint32_t heightROIAroundSpot, uint32_t widthROIAroundSpot, uint32_t radiusSpot)
    : _heightROIAroundSpot(heightROIAroundSpot), _widthROIAroundSpot(widthROIAroundSpot), _radiusSpot(radiusSpot) {}


double AlgoMultiplexConvolBrut::computeSpotBrutResult(const cv::Mat& image, cv::Point center)
{
    uint8_t white = 255;
    uint8_t black = 0;

    cv::Mat convol = ImagePreProcessing::convolImage(image, _widthGaussianToConvol);

    auto [totalSquareValue, spotValue, noiseValue]
        = ImageAnalyzer::computeSpotValues(convol, center, _radiusSpot, _widthROIAroundSpot, _widthROIAroundSpot);

    return spotValue - noiseValue;
}

AlgoMultiplexResult AlgoMultiplexConvolBrut::processMultiplexAnalyze(const cv::Mat& image,
                                                                     GridOfSpots gridOfSpots)
{
    AlgoMultiplexResult allResults;
    std::map<ColumnIndex, double> CASResults;

    for(auto spot : gridOfSpots.SpotsCASCoordinates)
    {
        ColumnIndex columnIndex = spot.first;
        cv::Point center = spot.second.point;

        double currentSpotResult = computeSpotBrutResult(image, center);
        CASResults[columnIndex] = currentSpotResult;
    }

    for(auto element : gridOfSpots.SpotsTestCoordinates)
    {
        ColumnIndex columnIndex = element.first;
        double currentColumnCASResult = CASResults[columnIndex];

        std::map<SpotType, std::vector<double>> currentColumnResults;

        for(auto [spotLocation, spotType] : element.second)
        {
            double currentSpotResult = computeSpotBrutResult(image, spotLocation.point);
             currentColumnResults[spotType].push_back(currentSpotResult / currentColumnCASResult);
        }
        for(auto columnResults : currentColumnResults)
        {
            SpotType spotType = columnResults.first;
            std::vector<double> results = columnResults.second;
            double mean = std::accumulate( results.begin(), results.end(), 0. )/ results.size();

            allResults.ColumnsResults[spotType][columnIndex] = mean;
        }
    }

    for(auto spotTypeResults : allResults.ColumnsResults)
    {
        SpotType spotType = spotTypeResults.first;
        double accumulator = 0;

        for(auto result : spotTypeResults.second)
            accumulator += result.second;

        allResults.GlobalResults[spotType] = accumulator / spotTypeResults.second.size();
    }

    return allResults;
}

void AlgoMultiplexConvolBrut::setWidthGaussianToConvol(uint32_t newWidthGaussian)
{
    _widthGaussianToConvol = newWidthGaussian;
}