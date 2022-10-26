//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef PICA_IIMAGEPROCESSOR_H
#define PICA_IIMAGEPROCESSOR_H

#include <vector>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include "common/imageProcessing/ImageAnalyzer.h"

struct ProcessorConfig{
    cv::Mat croppedImage;

    bool isVisible;
    ColorChannel channel;

    uint32_t padding;
    uint32_t rectangleWidth;
    uint32_t maskLength;

    uint32_t minCLSearchArea;
    uint32_t maxCLSearchArea;

    double lengthROIMm;
    double theoricDistanceCLtoTLMm;
    double TLLengthMm;

    double ratioNumberPixelsMm;
    double rectangleWidthMm;
    double maskLengthMm;

    ProcessorConfig(cv::Mat croppedImageArg,
                    bool isVisibleArg,
                    ColorChannel channelArg,
                    double lengthROIMmArg,
                    double theoricDistanceCLtoTLMmArg,
                    double TLLengthMmArg,
                    double referenceNumberOfPixelsArg,
                    double rectangleWidthMmArg,
                    double maskLengthMmArg);
    ProcessorConfig();
};

struct ProxNoiseROISizeMm{
    double TLLength;
    double noiseBeforeTL;
    double noiseAfterTL;
};

struct ProfileData{
    TImageProfile profileToAnalyze;
    uint32_t controlLine;
    uint32_t testLine;
};

class IImageProcessor {
public:
    //Functions for Solarium
    virtual ProfileData computeProfileForAnalyze(ProcessorConfig config, bool useConvolPreProcessing, std::optional<uint32_t> controlLineParam = {}, std::optional<uint32_t> testLineParam = {});
    virtual double compute(ProcessorConfig config, const ProfileData& profileData, const ProxNoiseROISizeMm& proxNoiseROIMm);
    //----------------------
    virtual std::tuple<double, double> processImageProfile(const TImageProfile & profile,
                                                           std::vector<uint32_t> testLines) = 0;
    virtual std::tuple<TImageProfile, double> transformProfileForProcessing(const TImageProfile& profile, std::vector<uint32_t> testAreas);
    virtual void setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength) = 0;
    void setConfig(ProcessorConfig config);

    virtual std::string determinePredictionMessage(double result, const std::string& cutoffsValues);
    virtual double computeFirstPrediction(double result, const std::string& coefficients);
    virtual double computeSecondPrediction(double result, const std::string& coefficients);

protected:
    ProcessorConfig _config;
};

#endif //PICA_IIMAGEPROCESSOR_H
