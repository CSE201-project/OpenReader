//
// Created by KarimSadki on 04/01/2021.
//

#include "AlgoExponantialFit.h"
#include "CurveFitter.h"

std::tuple<TImageProfile, double> AlgoExponantialFit::transformProfileForProcessing(const TImageProfile& profile, std::vector<uint32_t> testAreas)
{
    TImageProfile profileWithoutLines = ImageAnalyzer::removeLinesOnProfile(profile, testAreas, _config.maskLength);

    CurveFitter curveFitter;
    curveFitter.InitialValues = { 20., 10., 0., 0., 0. };
    auto [beta, coefficient] = curveFitter.solveModelCeres(profileWithoutLines, 12.52, ExpoFit);

    TImageProfile exponantialFit;
    uint32_t size = profile.size();
    for(uint32_t i = 1; i <= size; i++)
        exponantialFit.push_back(CurveFitter::exponantialModel(i, beta));

    TImageProfile profileFitted = CurveFitter::removeFitFromProfile(profile, exponantialFit);

    return {profileFitted, coefficient};
}

std::tuple<double, double> AlgoExponantialFit::processImageProfile(const TImageProfile& profile,
                                                                   std::vector<uint32_t> testAreas)
{
    //testAreas must contain {controlLinePos, firstTestLine, optional other test lines} in this order
    if((testAreas.size() < 2) || profile.empty())
        return {0, 0};

    return _algoProxNoise.processImageProfile(profile, testAreas);
}

void AlgoExponantialFit::setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength)
{
    _algoProxNoise.setROILengths(newTestLineLength, newNoiseBeforeLength, newNoiseAfterLength);
}