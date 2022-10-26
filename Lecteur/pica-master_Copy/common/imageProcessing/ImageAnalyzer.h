//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/30.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef GAUSSIANFIT_IMAGEANALYZER_H
#define GAUSSIANFIT_IMAGEANALYZER_H

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

enum ColorChannel {
    Blue = 0, Green = 1, Red = 2
};

enum ColorChannelLAB {
    L = 0, A = 1, B = 2
};

typedef std::vector<double> TImageProfile;

struct SigmoideCoefficients {
    double Bottom;
    double Top;
    double IC50;
    double HillSlope;
};

struct LinearRegressionCoefficients {
    double A;
    double B;
};

struct MeansForLinearRegression {
    std::map<double, double> proxNoiseMeansToFit;
    std::map<double, double> expoFitMeansToFit;
    std::map<double, double> poly2MeansToFit;
    std::map<double, double> convolBrutMeansToFit;
    std::map<double, double> convolExpoFitMeansToFit;
    std::map<double, double> convolPoly2MeansToFit;
};

class ImageAnalyzer {
public:
    constexpr static double DEFAULT_STEP_MM = 0.1;

    static TImageProfile computeProfileSum(const cv::Mat& image);
    static std::vector<double> computeImageProfile(const cv::Mat & image, bool isVisible = false, ColorChannel color = ColorChannel::Red, uint8_t bitsPerChannel = 8);

    static std::tuple<cv::Mat, cv::Mat> extractChannelImageWithPadding(const cv::Mat & image, ColorChannel channel, uint32_t padding);
    static cv::Mat extractLineAreaImage(uint32_t linePositionNotCutted,
                                        const cv::Mat& oneChannelImage,
                                        uint32_t margin);
    static uint32_t findControlLine(const std::vector<double> & profile, uint32_t theoricTLLengthPx, uint32_t minCLSearchArea = 0, uint32_t maxCLSearchArea = 0);
    static std::tuple<uint32_t, uint32_t> computeCutProfileBorders(const std::vector<double>& profile,
                                                                   uint32_t minCLSearchArea,
                                                                   uint32_t maxCLSearchArea,
                                                                   uint32_t controlLine,
                                                                   uint32_t testLine,
                                                                   uint32_t rectangleWidth,
                                                                   uint32_t margin);

    static TImageProfile normalizeProfile(const TImageProfile& imageProfile, uint32_t normalizationFactor);
    static cv::Mat generatePlotImage(const std::vector<double> & imageProfile, uint8_t bitsPerChannel = 8);

    static std::vector<uint32_t> computeHistogram8Bits(const cv::Mat& grayImage);
    static cv::Mat revertGrayScaleImage8Bits(const cv::Mat& channelImage);

    static TImageProfile removeLinesOnProfile(const TImageProfile& profile, const std::vector<uint32_t>& linesAreas, uint32_t maskLength);
    static std::tuple<double, double, double, double> computeRawControlTestNoise(const TImageProfile& profile,
                                                                                 const std::vector<uint32_t>& linesAreas,
                                                                                 uint32_t rectangleWidth,
                                                                                 uint32_t localisationNoiseCL,
                                                                                 uint32_t localisationNoiseTL);
    static double computeLOD(std::map<double, std::vector<double>> data);
    static cv::Mat readImage(std::filesystem::path imagePath);
    static uint32_t convertMillimetersToPixels(double referenceNumberOfPixels, double lengthToConvert, double referenceLength = 1.);
    static double convertPixelsToMillimeters(double referenceNumberOfPixels, double numberOfPixelsToConvert, double referenceLength = 1.);
    static std::vector<double> computeCumulateHistogram8Bits(const cv::Mat& image);
    static uint32_t initDistanceCLtoTL(double theoricDistanceCLtoTLMm,
                                       double TLLengthMm,
                                       double referenceNumberOfPixels,
                                       uint32_t controlLine,
                                       const cv::Mat& stripCroppedPadding,
                                       ColorChannel channel);
    /*Compute three useful values around a spot in the image:
     * Average intensity define by a circle
     * Average intensity define by a noise square (a square without the circle)
     * Average intensity define by a square*/
    static std::tuple<double, double, double> computeSpotValues(const cv::Mat& image,
                                                                const cv::Point& point,
                                                                uint32_t circleRadius,
                                                                uint32_t noiseAreaWidth,
                                                                uint32_t totalAreaWidth);
};

#endif //GAUSSIANFIT_IMAGEANALYZER_H
