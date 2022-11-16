//
// Created by KarimSadki on 03/02/2021.
//

#include "AlgoPoly2.h"
#include "CurveFitter.h"

std::tuple<TImageProfile, double> AlgoPoly2::transformProfileForProcessing(const TImageProfile& profile, std::vector<uint32_t> testAreas)
{
    TImageProfile profileWithoutLines = ImageAnalyzer::removeLinesOnProfile(profile, testAreas, _config.maskLength);

    CurveFitter curveFitter;
    curveFitter.InitialValues = { 0.001, 0.5, 0., 0., 0.};
    auto [beta, coefficient] = curveFitter.solveModelCeres(profileWithoutLines, 0.99, Poly2);

    TImageProfile poly2Fit;
    for(uint32_t i = 1; i <= profile.size(); i++)
        poly2Fit.push_back(CurveFitter::poly2Model(i, beta));

    TImageProfile profileFitted = CurveFitter::removeFitFromProfile(profile, poly2Fit);

    return {profileFitted, coefficient};
}

std::tuple<double, double> AlgoPoly2::processImageProfile(const TImageProfile& profile,
                                                          std::vector<uint32_t> testAreas)
{
    //testAreas must contain {controlLinePos, firstTestLine, optional other test lines} in this order
    if((testAreas.size() < 2) || profile.empty())
        return {0, 0};

    return _algoProxNoise.processImageProfile(profile, testAreas);
}

void AlgoPoly2::setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength)
{
    _algoProxNoise.setROILengths(newTestLineLength, newNoiseBeforeLength, newNoiseAfterLength);
}