//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "AlgoProxNoise.h"
#include "CurveFitter.h"
#include <numeric>
#include <tuple>
#include <limits>

std::tuple<double, double, double> AlgoProxNoise::computeNoisesAndLine(const TImageProfile & profile, uint32_t lineIndex)
{
    uint32_t extra = 0;
    if(TestLineLength % 2 == 1)
        extra++;

    int leftLimitNoiseBefore = lineIndex - (TestLineLength / 2) - NoiseBeforeLength;
    if(leftLimitNoiseBefore < 0)
        leftLimitNoiseBefore = 0;

    int leftLimitTestLine = lineIndex - (TestLineLength / 2);
    if(leftLimitTestLine < 0)
        leftLimitTestLine = 0;

    int rightLimitTestLine = lineIndex + (TestLineLength / 2) + extra;
    if(rightLimitTestLine >= profile.size())
        rightLimitTestLine = profile.size() - 1;

    int rightLimitNoiseAfter = lineIndex + (TestLineLength / 2) + extra + NoiseAfterLength;
    if(rightLimitNoiseAfter >= profile.size())
        rightLimitNoiseAfter = profile.size() - 1;

    double noiseBeforeLine = std::accumulate(profile.begin() + leftLimitNoiseBefore,
                                             profile.begin() + leftLimitTestLine,
                                             0.);
    double line = std::accumulate(profile.begin() + leftLimitTestLine,
                                  profile.begin() + rightLimitTestLine,
                                  0.);
    double noiseAfterLine = std::accumulate(profile.begin() + rightLimitTestLine,
                                            profile.begin() + rightLimitNoiseAfter,
                                            0.);
    return {noiseBeforeLine / NoiseBeforeLength, line / TestLineLength, noiseAfterLine / NoiseAfterLength};
}

std::tuple<double, double> AlgoProxNoise::processImageProfile(const TImageProfile & profile,
                                                                      std::vector<uint32_t> testLines)
{
    auto [noiseBeforeControlLine, controlLine, noiseAfterControlLine] = computeNoisesAndLine(profile, testLines[0]);
    auto [noiseBeforeTestLine, testLine, noiseAfterTestLine] = computeNoisesAndLine(profile, testLines[1]);

    if(noiseBeforeControlLine == 0)
        noiseBeforeControlLine = noiseAfterControlLine;
    if(noiseAfterTestLine == 0)
        noiseAfterTestLine = noiseBeforeTestLine;

    double meanNoiseCL = (noiseBeforeControlLine + noiseAfterControlLine) / 2;
    double meanNoiseTL = (noiseBeforeTestLine + noiseAfterTestLine) / 2;

    double controlLineDifference = controlLine - meanNoiseCL;

    double epsilon = 0.000001;
    if((controlLineDifference < epsilon) && (controlLineDifference > -epsilon))
        controlLineDifference = 0.001;

    double result = (testLine - meanNoiseTL) / controlLineDifference;

    return {result, std::numeric_limits<double>::quiet_NaN()};
}

void AlgoProxNoise::setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength)
{
    TestLineLength = newTestLineLength;
    NoiseBeforeLength = newNoiseBeforeLength;
    NoiseAfterLength = newNoiseAfterLength;
}

double AlgoProxNoise::compute(ProcessorConfig config, const ProfileData& profileData, const ProxNoiseROISizeMm& proxNoiseROIMm)
{
    _config = config;

    uint32_t testLineLengthPx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, proxNoiseROIMm.TLLength);
    uint32_t noiseBeforePx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, proxNoiseROIMm.noiseBeforeTL);
    uint32_t noiseAfterPx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, proxNoiseROIMm.noiseAfterTL);

    setROILengths(testLineLengthPx, noiseBeforePx, noiseAfterPx);

    auto [result, fitCoefficient] = processImageProfile(profileData.profileToAnalyze,{ profileData.controlLine, profileData.testLine });
    return result;
}
