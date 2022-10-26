//
// Created by KarimSadki on 21/04/2021.
//

#include <limits>
#include <tuple>
#include "IImageProcessor.h"
#include "CurveFitter.h"
#include "AlgoProxNoise.h"
#include "common/fileProcessing/Parser.h"
#include "common/imageProcessing/ImagePreProcessing.h"

ProcessorConfig::ProcessorConfig(cv::Mat croppedImageArg, bool isVisibleArg, ColorChannel channelArg, double lengthROIMmArg, double theoricDistanceCLtoTLMmArg, double TLLengthMmArg, double referenceNumberOfPixelsArg, double rectangleWidthMmArg, double maskLengthMmArg)
: croppedImage(croppedImageArg.clone()), isVisible(isVisibleArg), channel(channelArg), lengthROIMm(lengthROIMmArg), theoricDistanceCLtoTLMm(theoricDistanceCLtoTLMmArg), TLLengthMm(TLLengthMmArg), ratioNumberPixelsMm(referenceNumberOfPixelsArg), rectangleWidthMm(rectangleWidthMmArg), maskLengthMm(maskLengthMmArg)
{
    uint32_t lengthCenterPx = ImageAnalyzer::convertMillimetersToPixels(ratioNumberPixelsMm, lengthROIMm);
    int paddingToApply = croppedImage.cols - lengthCenterPx;
    if(paddingToApply < 2)
        paddingToApply = 2;
    padding = paddingToApply;

    rectangleWidth = ImageAnalyzer::convertMillimetersToPixels(ratioNumberPixelsMm, rectangleWidthMm);
    maskLength = ImageAnalyzer::convertMillimetersToPixels(ratioNumberPixelsMm, maskLengthMm);

    minCLSearchArea = ImageAnalyzer::convertMillimetersToPixels(ratioNumberPixelsMm, 0.5);
    maxCLSearchArea = croppedImage.rows / 3;
}

ProcessorConfig::ProcessorConfig(){}

std::tuple<TImageProfile, double> IImageProcessor::transformProfileForProcessing(const TImageProfile& profile, std::vector<uint32_t> testAreas)
{
    return {profile, std::numeric_limits<double>::quiet_NaN()};
}

ProfileData IImageProcessor::computeProfileForAnalyze(ProcessorConfig config, bool useConvolPreProcessing, std::optional<uint32_t> controlLineParam, std::optional<uint32_t> testLineParam)
{
    _config = config;

    const auto [oneChannelImage, stripCroppedPadding] = ImageAnalyzer::extractChannelImageWithPadding(_config.croppedImage, _config.channel, _config.padding);

    std::vector<double> rawProfile = ImageAnalyzer::computeImageProfile(stripCroppedPadding, _config.isVisible, _config.channel);

    uint32_t controlLine, testLine;
    if (controlLineParam.has_value())
        controlLine = controlLineParam.value();
    else
    {
        uint32_t theoricTLLengthPx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm,
                                                                               _config.TLLengthMm);
        controlLine = ImageAnalyzer::findControlLine(rawProfile, theoricTLLengthPx, _config.minCLSearchArea,
                                                     _config.maxCLSearchArea);
    }

    if (testLineParam.has_value())
        testLine = testLineParam.value();
    else
    {
        uint32_t distanceCLtoTL = ImageAnalyzer::initDistanceCLtoTL(_config.theoricDistanceCLtoTLMm, _config.TLLengthMm,
                                                                    _config.ratioNumberPixelsMm, controlLine,
                                                                    stripCroppedPadding, _config.channel);
        testLine = controlLine + distanceCLtoTL;
    }

    std::vector<double> profileToAnalyze;
    if(useConvolPreProcessing)
    {
        uint32_t widthGaussConvol
                = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, ImagePreProcessing::DEFAULT_WIDTH_GAUSSIAN_CONVOL_TL_MM);

        TImageProfile convolProfile = ImagePreProcessing::convolImageAndComputeProfile(oneChannelImage, widthGaussConvol);
        profileToAnalyze = convolProfile;
    }
    else
    {
        profileToAnalyze = rawProfile;
    }

    ProfileData profileData = {
            .profileToAnalyze = profileToAnalyze,
            .controlLine = controlLine,
            .testLine = testLine
    };

    return profileData;
}

double IImageProcessor::compute(ProcessorConfig config,
                                const ProfileData &profileData,
                                const ProxNoiseROISizeMm &proxNoiseROIMm)
{
    _config = config;

    auto [startCutProfile, endCutProfile] = ImageAnalyzer::computeCutProfileBorders(profileData.profileToAnalyze,
                                                                                    _config.minCLSearchArea,
                                                                                    _config.maxCLSearchArea,
                                                                                    profileData.controlLine,
                                                                                    profileData.testLine,
                                                                                    _config.rectangleWidth,
                                                                                    _config.maskLength / 2);
    std::vector<double> cutProfile(profileData.profileToAnalyze.begin() + startCutProfile,
                                   profileData.profileToAnalyze.begin() + endCutProfile);

    uint32_t controlLine = profileData.controlLine - startCutProfile;
    uint32_t testLine = profileData.testLine - startCutProfile;

    auto [transformedProfile, fitCoefficient] = transformProfileForProcessing(cutProfile, {controlLine, testLine});

    uint32_t testLineLengthPx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, proxNoiseROIMm.TLLength);
    uint32_t noiseBeforePx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, proxNoiseROIMm.noiseBeforeTL);
    uint32_t noiseAfterPx = ImageAnalyzer::convertMillimetersToPixels(_config.ratioNumberPixelsMm, proxNoiseROIMm.noiseAfterTL);

    setROILengths(testLineLengthPx, noiseBeforePx, noiseAfterPx);

    auto [result, fitCoefficient2] = processImageProfile(transformedProfile,
                                                         { controlLine, testLine });
    return result;
}

void IImageProcessor::setConfig(ProcessorConfig config)
{
    _config = config;
}

std::string IImageProcessor::determinePredictionMessage(double result, const std::string& cutoffsValues)
{
    std::vector<double> values = Parser::parseStringAsDoubles(cutoffsValues, " ");

    if(result <= values[0])
        return "Negative";
    else
        return "Positive";
}

double IImageProcessor::computeFirstPrediction(double result, const std::string& coefficients)
{
    std::vector<double> values = Parser::parseStringAsDoubles(coefficients, " ");

    LinearRegressionCoefficients coeffs = {
            values[0],
            values[1]
    };

    return CurveFitter::computeLinearRegressionPrediction(coeffs, result);
}

double IImageProcessor::computeSecondPrediction(double result, const std::string& coefficients)
{
    std::vector<double> values = Parser::parseStringAsDoubles(coefficients, " ");

    SigmoideCoefficients coeffs = {
            values[0],
            values[1],
            values[2],
            values[3]
    };

    return CurveFitter::computeSigmoidePrediction(coeffs, result);
}
