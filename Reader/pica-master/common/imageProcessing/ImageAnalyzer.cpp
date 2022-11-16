//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/30.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "ImageAnalyzer.h"
#include <numeric>
#include <filesystem>
#ifdef ENABLE_OPENCV_PLOT
    #include <opencv2/plot.hpp>
#endif

template<typename T>
double meanColorChannel(const cv::Mat & image, int currentRow, ColorChannel color, int threshold = 0)
{
    double total = 0, nbUsedPoints = 0;
    const T * imageRow = image.ptr<T>(currentRow);
    for (int i = 0; i < image.cols; i++)
    {
        auto colorValue = imageRow[i].val[color];
        if (colorValue >= threshold)
        {
            total += colorValue;
            nbUsedPoints++;
        }
    }
    return nbUsedPoints > 0 ? total / nbUsedPoints : 0;
}

std::vector<uint32_t> ImageAnalyzer::computeHistogram8Bits(const cv::Mat& grayImage)
{
    std::vector<uint32_t> histogram(256, 0);
    for (int i = 0; i < grayImage.rows; ++i)
    {
        for (int j = 0; j < grayImage.cols; ++j)
        {
            uint32_t grayValue = (uint32_t) grayImage.at<uint8_t>(i, j);
            histogram[grayValue]++;
        }
    }

    return histogram;
}

cv::Mat ImageAnalyzer::revertGrayScaleImage8Bits(const cv::Mat& grayImage)
{
    cv::Mat imageResult(grayImage.rows, grayImage.cols, grayImage.type());

    for (int i = 0; i < grayImage.rows; ++i)
    {
        for (int j = 0; j < grayImage.cols; ++j)
        {
            uint8_t grayValue = grayImage.at<uint8_t>(i, j);
            imageResult.at<uint8_t>(i, j) = 255 - grayValue;
        }
    }

    return imageResult;
}

std::vector<double> ImageAnalyzer::computeImageProfile(const cv::Mat &image, bool isVisible,ColorChannel color, uint8_t bitsPerChannel)
{
    std::vector<double> profileResult;
    uint32_t maxColor = pow(2,bitsPerChannel)-1;

    for (int i = 0; i < image.rows; i++)
    {
        double mean = 0;

        if (bitsPerChannel == 8)
            mean = meanColorChannel<cv::Vec3b>(image, i, color);
        else
            mean = meanColorChannel<cv::Vec3w>(image, i, color);

        if(isVisible)
            mean = maxColor - mean;
        profileResult.push_back(mean);
    }
    return profileResult;
}

TImageProfile ImageAnalyzer::computeProfileSum(const cv::Mat& image)
{
    TImageProfile profileLineConvol;

    for(uint32_t i = 0; i < image.rows; i++)
    {
        double sumLineConvol = 0;
        for(uint32_t j = 0; j < image.cols; j++)
            sumLineConvol += image.at<double>(i, j);

        profileLineConvol.push_back(sumLineConvol);
    }

    return profileLineConvol;
}

std::tuple<cv::Mat, cv::Mat> ImageAnalyzer::extractChannelImageWithPadding(const cv::Mat & image, ColorChannel channel, uint32_t padding)
{
    uint32_t paddingToUse = (padding / 2);
    cv::Rect roiPadding = cv::Rect(cv::Point(paddingToUse, 0),
                                   cv::Point(image.cols - paddingToUse, image.rows-1));
    cv::Mat stripCroppedPadding = image(roiPadding);

    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(stripCroppedPadding, frameBGRChannels);

    cv::Mat oneChannelImage = frameBGRChannels[channel];
    oneChannelImage.convertTo(oneChannelImage,CV_64F);

    return { oneChannelImage, stripCroppedPadding };
}

cv::Mat ImageAnalyzer::extractLineAreaImage(uint32_t linePositionNotCutted,
                                            const cv::Mat& oneChannelImage,
                                            uint32_t margin)
{
    int yStartArea = linePositionNotCutted - margin;
    int yEndArea = linePositionNotCutted + margin + 1;

    if(yStartArea < 0)
        yStartArea = 0;
    if(yEndArea >= oneChannelImage.rows)
        yEndArea = oneChannelImage.rows - 1;

    cv::Rect imageLineAreaROI = cv::Rect(cv::Point(0, yStartArea),
                                         cv::Point(oneChannelImage.cols - 1, yEndArea));

    return oneChannelImage(imageLineAreaROI);
}


TImageProfile ImageAnalyzer::normalizeProfile(const TImageProfile& imageProfile, uint32_t normalizationFactor)
{
    TImageProfile normalizedProfile;
    for(auto nb : imageProfile)
        normalizedProfile.push_back(nb / normalizationFactor);

    return normalizedProfile;
}

cv::Mat ImageAnalyzer::generatePlotImage(const std::vector<double> &imageProfile, uint8_t bitsPerChannel)
{
#ifdef ENABLE_OPENCV_PLOT
    uint32_t normalizationFactor = pow(2,bitsPerChannel)-1;
    const TImageProfile normalizedProfile = normalizeProfile(imageProfile, normalizationFactor);

    int size = normalizedProfile.size();
    cv::Mat data(1, size, CV_64F);
    for(int i = 0; i < size; i++)
    {
        // Plot bug workaround : we limit the number of digit
        double tmp = normalizedProfile.at(i);
        tmp = floor(tmp * 1000000) / 1000000;
        data.at<double>(0, i) = tmp;
    }
    cv::Mat plot_result;

    cv::Ptr<cv::plot::Plot2d> plot = cv::plot::Plot2d::create(data);
    plot->setInvertOrientation(true);
    plot->setMaxX((size));

    plot->setMaxY(1);
    plot->setShowText(true);
    plot->render(plot_result);
    return plot_result;
#else
    return cv::Mat();
#endif
}

uint32_t ImageAnalyzer::findControlLine(const std::vector<double> & profile, uint32_t theoricTLLengthPx, uint32_t minCLSearchArea, uint32_t maxCLSearchArea)
{
    if (!maxCLSearchArea)
        maxCLSearchArea = profile.size() - 1;

    const double bottomProfileValue = profile[maxCLSearchArea];
    const double maxControlLineValue = *std::max_element(profile.begin() + minCLSearchArea,
                                                         profile.begin() + maxCLSearchArea);
    if(bottomProfileValue == maxControlLineValue)
        return (minCLSearchArea + maxCLSearchArea) / 2;

    const double middleControlLineHeight = (bottomProfileValue + maxControlLineValue) / 2;

    uint32_t leftValueCLMiddle = minCLSearchArea;
    uint32_t rightValueCLMiddle = maxCLSearchArea;

    for(uint32_t i = minCLSearchArea; i < maxCLSearchArea; i++)
    {
        if((profile[i] <= middleControlLineHeight) && (middleControlLineHeight <= profile[i+1]))
        {
            if((profile[i]-middleControlLineHeight) < (middleControlLineHeight-profile[i+1]))
                leftValueCLMiddle = i;
            else
                leftValueCLMiddle = i+1;
            break;
        }
    }

    for(uint32_t i = maxCLSearchArea; i > minCLSearchArea; i--)
    {
        if((profile[i] <= middleControlLineHeight) && (middleControlLineHeight <= profile[i-1]))
        {
            if((profile[i-1]-middleControlLineHeight) < (middleControlLineHeight-profile[i]))
                rightValueCLMiddle = i;
            else
                rightValueCLMiddle = i-1;
            break;
        }
    }

    uint32_t minimumTLLength = theoricTLLengthPx / 4;
    uint32_t maximumTLLength = theoricTLLengthPx * 1.5;

    int findedTLLength =  rightValueCLMiddle - leftValueCLMiddle;
    if((findedTLLength < minimumTLLength) || (maximumTLLength < findedTLLength))
        return (minCLSearchArea + maxCLSearchArea) / 2;

    return (leftValueCLMiddle + rightValueCLMiddle) / 2;
}

std::tuple<uint32_t, uint32_t> ImageAnalyzer::computeCutProfileBorders(const std::vector<double>& profile,
                                                                       uint32_t minCLSearchArea,
                                                                       uint32_t maxCLSearchArea,
                                                                       uint32_t controlLine,
                                                                       uint32_t testLine,
                                                                       uint32_t rectangleWidth,
                                                                       uint32_t margin)
{
    uint32_t startCutProfile, endCutProfile;

    std::vector<double> derivatedProfile = profile;
    std::adjacent_difference(derivatedProfile.begin(), derivatedProfile.end(), derivatedProfile.begin());

    int maxDerivatedIndex = std::max_element(derivatedProfile.begin() + minCLSearchArea,
                                             derivatedProfile.begin() + maxCLSearchArea) - derivatedProfile.begin();

    uint32_t testLineAndMargin = testLine + margin + 1;
    if(testLineAndMargin >= profile.size())
        testLineAndMargin = profile.size() - 1;

    endCutProfile = std::max_element(profile.begin() + testLineAndMargin, profile.end()) - profile.begin();

    std::vector<uint32_t> negativeDerivatedProfileIndices;
    for(uint32_t i = 0; i < derivatedProfile.size(); i++)
        if(derivatedProfile[i] <= 0.)
            negativeDerivatedProfileIndices.push_back(i);

    int accumulator = std::count_if(negativeDerivatedProfileIndices.begin(), negativeDerivatedProfileIndices.end(),
                                    [maxDerivatedIndex](int nb){return nb <= maxDerivatedIndex;}) - 1;

    if(accumulator == -1)
        startCutProfile = minCLSearchArea;
    else
        startCutProfile = negativeDerivatedProfileIndices[accumulator];

    if((controlLine - (rectangleWidth / 2)) < startCutProfile)
        startCutProfile = minCLSearchArea;

    return { startCutProfile, endCutProfile };
}

std::tuple<double, double, double, double> ImageAnalyzer::computeRawControlTestNoise(const TImageProfile& profile,
                                                                             const std::vector<uint32_t>& linesAreas,
                                                                             uint32_t rectangleWidth,
                                                                             uint32_t localisationNoiseCL,
                                                                             uint32_t localisationNoiseTL)
{
    //linesAreas must contain {controlLinePos, testLinePos} in this order
    if((linesAreas.size() < 2) || profile.empty())
        return {0, 0, 0, 0};

    const uint32_t halfRectangleWidth = rectangleWidth / 2;
    const uint32_t controlLine = linesAreas.at(0);
    const uint32_t testLine = linesAreas.at(1);

    int limitLeftControlLine = controlLine - halfRectangleWidth;
    if(limitLeftControlLine < 0)
        limitLeftControlLine = 0;

    int limitRightTestLine = testLine + halfRectangleWidth;
    if(limitRightTestLine >= profile.size())
        limitRightTestLine = profile.size() - 1;

    int limitLeftNoiseTL = localisationNoiseTL - halfRectangleWidth;
    if(limitLeftNoiseTL >= profile.size())
        limitLeftNoiseTL = profile.size() - 1;

    int limitRightNoiseTL = localisationNoiseTL + halfRectangleWidth;
    if(limitRightNoiseTL >= profile.size())
        limitRightNoiseTL = profile.size() - 1;

    double controlMean = (std::accumulate(profile.begin() + limitLeftControlLine,
                                         profile.begin() + controlLine + halfRectangleWidth,
                                         0.0, [](double sum, double nb) { return sum + nb; }) / rectangleWidth);

    double testMean = (std::accumulate(profile.begin() + testLine - halfRectangleWidth,
                                      profile.begin() + limitRightTestLine,
                                      0.0, [](double sum, double nb) { return sum + nb; }) / rectangleWidth);

    double noiseTLMean = (std::accumulate(profile.begin() + limitLeftNoiseTL,
                                                      profile.begin() + limitRightNoiseTL,
                                                      0.0, [](double sum, double nb) { return sum + nb; }) / rectangleWidth);

    double noiseCLMean = (std::accumulate(profile.begin() + localisationNoiseCL - halfRectangleWidth,
                                                      profile.begin() + localisationNoiseCL + halfRectangleWidth,
                                                      0.0, [](double sum, double nb) { return sum + nb; }) / rectangleWidth);


    return { controlMean, testMean, noiseCLMean, noiseTLMean};
}

TImageProfile ImageAnalyzer::removeLinesOnProfile(const TImageProfile& profile, const std::vector<uint32_t>& linesAreas, uint32_t maskLength)
{
    uint32_t controlLine = linesAreas[0];
    uint32_t testLine = linesAreas[1];
    uint32_t halfMaskLength = maskLength / 2;

    int32_t numberOfNanAroundCL = maskLength;
    int32_t leftBorderToCut = controlLine - halfMaskLength;
    if(leftBorderToCut < 0)
    {
        numberOfNanAroundCL += leftBorderToCut;
        leftBorderToCut = 0;
    }


    TImageProfile profileWithoutLines(profile.begin(), profile.begin() + leftBorderToCut);

    profileWithoutLines.insert(profileWithoutLines.end(),
                               numberOfNanAroundCL,
                               std::numeric_limits<double>::quiet_NaN());

    profileWithoutLines.insert(profileWithoutLines.end(),
                               profile.begin() + controlLine + halfMaskLength,
                               profile.begin() + testLine - halfMaskLength);

    profileWithoutLines.insert(profileWithoutLines.end(),
                               maskLength,
                               std::numeric_limits<double>::quiet_NaN());

    profileWithoutLines.insert(profileWithoutLines.end(),
                               profile.begin() + testLine + halfMaskLength,
                               profile.end());
    return profileWithoutLines;
}

double ImageAnalyzer::computeLOD(std::map<double, std::vector<double>> data)
{
    std::map<double, double> allMean;
    double lowestConcentrationMean3sigma = -1;

    for(auto& element : data)
    {
        std::vector<double> currentConcentrationData = element.second;

        double mean = std::accumulate(currentConcentrationData.begin(), currentConcentrationData.end(), 0.0, [](double sum, double currentResult) {
            return sum + currentResult;
        }) / currentConcentrationData.size();

        allMean[element.first] = mean;

        if(lowestConcentrationMean3sigma < 0)
        {
            double accumulator = 0;
            for(auto & nb : currentConcentrationData)
                accumulator += std::pow(nb - mean, 2);

            lowestConcentrationMean3sigma = mean + std::sqrt(accumulator / currentConcentrationData.size()) * 3;
        }
    }

    double result = -1;
    for(auto it = allMean.rbegin(); it != allMean.rend(); ++it)
    {
        if((it->second <= lowestConcentrationMean3sigma) || (std::isnan(it->second)))
            break;
        result = it->first;
    }

    return result;
}

cv::Mat ImageAnalyzer::readImage(std::filesystem::path imagePath)
{
    cv::Mat image;
#ifdef WIN32
    std::ifstream sourceFile(imagePath.wstring().c_str(), std::ios::in | std::ios::binary);
    if(sourceFile)
    {
        sourceFile.seekg (0, sourceFile.end);
        int fileSize = sourceFile.tellg();
        sourceFile.seekg (0, sourceFile.beg);

        std::vector<char> buffer(fileSize);
        sourceFile.read(buffer.data(), fileSize);

        image = cv::imdecode(cv::Mat(buffer), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        sourceFile.close();
    }
    else{
        std::cerr << "Error during source image reading" << std::endl;
        exit(1);
    }
#else
    image = cv::imread(imagePath);
#endif
    return image;
}

uint32_t ImageAnalyzer::convertMillimetersToPixels(double referenceNumberOfPixels, double lengthToConvert, double referenceLength)
{
    return std::round((referenceNumberOfPixels * lengthToConvert) / referenceLength);
}

double ImageAnalyzer::convertPixelsToMillimeters(double referenceNumberOfPixels, double numberOfPixelsToConvert, double referenceLength)
{
    return (referenceLength * numberOfPixelsToConvert) / referenceNumberOfPixels;
}

std::vector<double> ImageAnalyzer::computeCumulateHistogram8Bits(const cv::Mat& image)
{
    const std::vector<uint32_t> histogram = ImageAnalyzer::computeHistogram8Bits(image);
    const uint32_t numberOfPixels = image.rows * image.cols;

    double accumulator = 0;
    std::vector<double> cumulateHistogram;

    for(int i = 0; i < histogram.size(); i++)
    {
        accumulator += histogram[i];
        cumulateHistogram.push_back(accumulator / numberOfPixels);
    }

    return cumulateHistogram;
}

uint32_t searchPercentageInCumulateHistogram(const std::vector<double>& cumulateHistogram, double searchedPercentage)
{
    uint32_t result = cumulateHistogram.size() - 1;

    for(uint32_t i = 0; i < cumulateHistogram.size(); i++)
    {
        if(cumulateHistogram[i] >= searchedPercentage)
        {
            result = i;
            break;
        }
    }

    return result;
}

uint32_t ImageAnalyzer::initDistanceCLtoTL(double theoricDistanceCLtoTLMm,
                                           double TLLengthMm,
                                           double referenceNumberOfPixels,
                                           uint32_t controlLine,
                                           const cv::Mat& stripCroppedPadding,
                                           ColorChannel channel)
{
    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(stripCroppedPadding, frameBGRChannels);
    cv::Mat oneChannelImage = frameBGRChannels[channel];

    uint32_t theoricDistanceCLtoTLPx = convertMillimetersToPixels(referenceNumberOfPixels, theoricDistanceCLtoTLMm);
    uint32_t TLLengthPx = convertMillimetersToPixels(referenceNumberOfPixels, TLLengthMm);

    std::vector<double> profilFromCumulateHistogram;

    //Height of extract ROI is marginToROI * 2 + 1
    uint32_t marginToROI = TLLengthPx / 4;

    for(uint32_t currentPosition = marginToROI; currentPosition < oneChannelImage.rows - 1 - marginToROI; currentPosition++)
    {
        const cv::Mat currentArea = ImageAnalyzer::extractLineAreaImage(currentPosition, oneChannelImage, marginToROI);
        const std::vector<double> cumulateHistogramCurrentArea = computeCumulateHistogram8Bits(currentArea);
        const uint32_t currentAreaValue = searchPercentageInCumulateHistogram(cumulateHistogramCurrentArea, 0.4);

        profilFromCumulateHistogram.push_back(currentAreaValue);
    }

    const uint32_t minTLSearchArea = theoricDistanceCLtoTLPx + controlLine - marginToROI - TLLengthPx;
    const uint32_t maxTLSearchArea = theoricDistanceCLtoTLPx + controlLine - marginToROI + TLLengthPx;

    const uint32_t testLine = ImageAnalyzer::findControlLine(profilFromCumulateHistogram, TLLengthPx, minTLSearchArea, maxTLSearchArea);

    return testLine + marginToROI - controlLine;
}

std::tuple<double, double, double> ImageAnalyzer::computeSpotValues(const cv::Mat& image,
                                                                    const cv::Point& point,
                                                                    uint32_t circleRadius,
                                                                    uint32_t noiseAreaWidth,
                                                                    uint32_t totalAreaWidth)
{
    uint8_t white = 255;
    uint8_t black = 0;

    cv::Rect noiseAreaROI(point.x - (noiseAreaWidth / 2),
                          point.y - (noiseAreaWidth / 2),
                          noiseAreaWidth,
                          noiseAreaWidth);

    cv::Mat noiseAreaImage = image(noiseAreaROI);

    cv::Mat mask(noiseAreaImage.size(), CV_8U, cv::Scalar::all(black));
    cv::circle(mask,cv::Point(mask.rows / 2,mask.cols / 2), circleRadius, cv::Scalar(white), -1);

    uint32_t nbPixelsSpot = 0;
    double sumSpot = 0, sumNoise = 0;

    for(uint32_t x = 0; x < noiseAreaImage.cols; x++)
    {
        for(uint32_t y = 0; y < noiseAreaImage.rows; y++)
        {
            if(mask.at<uint8_t>(y, x) == white)
            {
                if(image.type() == CV_64F)
                    sumSpot += noiseAreaImage.at<double>(y, x);
                else
                    sumSpot += noiseAreaImage.at<uint8_t>(y, x);
                nbPixelsSpot++;
            }
            else
            {
                if(image.type() == CV_64F)
                    sumNoise += noiseAreaImage.at<double>(y, x);
                else
                    sumNoise += noiseAreaImage.at<uint8_t>(y, x);
            }
        }
    }
    double meanSpot = sumSpot / nbPixelsSpot;
    double meanNoise = sumNoise / ((noiseAreaImage.rows * noiseAreaImage.cols) - nbPixelsSpot);

    cv::Rect totalAreaROI(point.x - (totalAreaWidth / 2),
                          point.y - (totalAreaWidth / 2),
                          totalAreaWidth,
                          totalAreaWidth);
    cv::Mat totalAreaImage = image(totalAreaROI);

    double meanTotalArea = cv::mean(totalAreaImage)[0];

    return {meanTotalArea, meanSpot, meanNoise};
}
