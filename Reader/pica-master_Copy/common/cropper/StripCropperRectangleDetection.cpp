//
// Created by KarimSadki on 17/12/2020.
//

#include <opencv2/opencv.hpp>
#include <numeric>
#include "StripCropperRectangleDetection.h"

uint8_t WHITE = 255;

StripCropperRectangleDetection::StripCropperRectangleDetection(CropParams params)
    : cropParams(params)
{
    ROIsToCrop currentROIsToCrop = _ROIsToCropMap[cropParams.deviceFocal];
    _ROIToThreshold = currentROIsToCrop.ROIToThreshold;
    _outputROI = currentROIsToCrop.outputROI;
}

cv::Mat StripCropperRectangleDetection::cropToThreshold(const cv::Mat& frame, const CropROIMm ROIToThreshold, std::vector<cv::Mat>* intermediateSteps) const
{
    uint32_t marginX = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIToThreshold.upLeftCornerMarginX);
    uint32_t marginY = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIToThreshold.upLeftCornerMarginY);
    uint32_t width = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIToThreshold.width);
    uint32_t heigth = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIToThreshold.height);

    cv::Rect roiToThresholdPx(marginX, marginY, width, heigth);
    cv::Mat frameCenter = frame(roiToThresholdPx);

    if (intermediateSteps != nullptr)
        intermediateSteps->push_back(frameCenter);

    return frameCenter;
}

cv::RotatedRect StripCropperRectangleDetection::detectRotatedRectangle(const cv::Mat& channelThreshold, std::vector<cv::Mat>* intermediateSteps) const
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(channelThreshold, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if(contours.size() == 0)
        return cv::RotatedRect();

    int biggestContourIdx = -1;
    float biggestContourArea = 0;
    cv::Mat drawing;

    if (intermediateSteps != nullptr)
        drawing = cv::Mat::zeros( channelThreshold.size(), CV_8UC3 );

    for( int i = 0; i < contours.size(); i++ )
    {
        if (intermediateSteps != nullptr)
            drawContours( drawing, contours, i, cv::Scalar(0, 255, 0), 1, 8, hierarchy, 0, cv::Point() );

        float ctArea= cv::contourArea(contours[i]);
        if(ctArea > biggestContourArea)
        {
            biggestContourArea = ctArea;
            biggestContourIdx = i;
        }
    }

    cv::RotatedRect rotatedRect = cv::minAreaRect(contours[biggestContourIdx]);

    if (intermediateSteps != nullptr)
    {
        cv::Point2f vertices[4];
        rotatedRect.points(vertices);
        for (int i = 0; i < 4; i++)
            line(drawing, vertices[i], vertices[(i+1)%4], cv::Scalar(255,255,255), 2);

        intermediateSteps->push_back(drawing);
    }

    return rotatedRect;
}

int determineBorderToCrop(std::vector<int> borderPoints)
{
    borderPoints.erase(std::remove(borderPoints.begin(), borderPoints.end(), -1), borderPoints.end());

    double mean = std::accumulate(borderPoints.begin(),borderPoints.end(), 0) / borderPoints.size();

    double accumulator = 0;
    for(auto& width : borderPoints)
        accumulator += std::pow(width - mean, 2);

    double standardDeviation = std::sqrt(accumulator / borderPoints.size());

    borderPoints.erase(
            std::remove_if(borderPoints.begin(), borderPoints.end(),
                           [mean, standardDeviation](int number)
                                    {return (number < (mean - standardDeviation)) || ((mean + standardDeviation) < number);}),
                                 borderPoints.end());

    return std::accumulate(borderPoints.begin(),borderPoints.end(), 0) / borderPoints.size();
}

cv::Rect StripCropperRectangleDetection::reduceROI(const cv::Mat &rotatedChannelThreshold, double outputHeight, int leftBoundary, int rightBoundary) const
{
    std::vector<int> leftBorders(16, -1) , rightBorders(16, -1);
    int middle = rotatedChannelThreshold.rows/2, shift = 30;

    std::vector<int> verificationHeight = {middle - (7 * shift),
                                           middle - (6 * shift),
                                           middle - (5 * shift),
                                           middle - (4 * shift),
                                           middle - (3 * shift),
                                           middle - (2 * shift),
                                           middle - shift,
                                           middle,
                                           middle + shift,
                                           middle + (2 * shift),
                                           middle + (3 * shift),
                                           middle + (4 * shift),
                                           middle + (5 * shift),
                                           middle + (6 * shift),
                                           middle + (7 * shift),
                                           middle + (8 * shift)};

    for(int i = leftBoundary; i < rightBoundary; i++)
    {
        for(int j = 0; j < verificationHeight.size(); j++)
        {
            if(leftBorders[j] == -1)
                if(rotatedChannelThreshold.at<uchar>(verificationHeight[j], i) == WHITE)
                    leftBorders[j] = i;
        }
        if(std::find(leftBorders.begin(), leftBorders.end(), -1) == leftBorders.end())
            break;
    }

    for(int i = rightBoundary; i > leftBoundary; i--)
    {
        for(int j = 0; j < verificationHeight.size(); j++)
        {
            if(rightBorders[j] == -1)
                if(rotatedChannelThreshold.at<uchar>(verificationHeight[j], i) == WHITE)
                    rightBorders[j] = i;
        }
        if(std::find(rightBorders.begin(), rightBorders.end(), -1) == rightBorders.end())
            break;
    }

    int leftValueToCrop = determineBorderToCrop(leftBorders);
    int rightValueToCrop = determineBorderToCrop(rightBorders);

    uint32_t height = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, outputHeight);

    return cv::Rect(cv::Point(leftValueToCrop, 0), cv::Point(rightValueToCrop,  height - 1));
}

std::tuple<cv::Point, cv::Point> StripCropperRectangleDetection::determineTopAndBottomLeft(const cv::RotatedRect& rotatedRect) const
{
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);

    if((-_maxPossibleTilt-90 < rotatedRect.angle) && (rotatedRect.angle < -_maxPossibleTilt))
    {
        return { vertices[2], vertices[1] };
    }
    else // if((-maxPossibleTilt < rotatedRect.angle) && (rotatedRect.angle < maxPossibleTilt))
    {
        return { vertices[1], vertices[0] };
    }
}

double determineAngle(const cv::Point & topLeft, const cv::Point & bottomLeft, const cv::RotatedRect& rotatedRect)
{
    double degree;

    if(topLeft.x > bottomLeft.x)
        degree = rotatedRect.angle + 90;

    else if(topLeft.x < bottomLeft.x)
        degree = rotatedRect.angle;

    else{
        if((-45 < rotatedRect.angle) && (rotatedRect.angle < 45))
            degree = rotatedRect.angle;
        else
            degree = rotatedRect.angle + 90;
    }
    return degree;
}

cv::Mat StripCropperRectangleDetection::rotateAndCropImage(const cv::Mat& frameCenter,
                                                           cv::RotatedRect& rotatedRect,
                                                           const cv::Mat& channelThreshold,
                                                           double outputHeight,
                                                           std::vector<cv::Mat>* intermediateSteps)  const
{
    cv::Mat rotated, rotatedThreshold;

    auto [topLeft, bottomLeft] = determineTopAndBottomLeft(rotatedRect);
    double degree = determineAngle(topLeft, bottomLeft, rotatedRect);

    int rectWidth = rotatedRect.size.width;
    if (rotatedRect.angle < -45)
        rectWidth = rotatedRect.size.height;

    uint32_t minimumAcceptableWidthPx = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, minimumAcceptableWidth);

    if(rectWidth < minimumAcceptableWidthPx)
        return frameCenter;

    cv::Mat rotationMatrix = cv::getRotationMatrix2D( topLeft, degree, 1);
    cv::warpAffine( frameCenter, rotated, rotationMatrix, frameCenter.size() );
    cv::warpAffine( channelThreshold, rotatedThreshold, rotationMatrix, channelThreshold.size() );

    cv::Rect roi;
    if((cropParams.cassetteModel != WithoutCassette) && (cropParams.lightningCondition == Visible))
    {
        uint32_t height = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, outputHeight);
        roi = cv::Rect(cv::Point(topLeft.x, 0), cv::Point(topLeft.x + rectWidth, height - 1));
    }
    else
        roi = reduceROI(rotatedThreshold, outputHeight, topLeft.x, topLeft.x + rectWidth);

    if (intermediateSteps != nullptr)
    {
        cv::Mat tmp = rotated.clone();
        cv::rectangle(tmp, roi, cv::Scalar(0, 255, 0));
        intermediateSteps->push_back(tmp);
    }

    uint32_t maximumAcceptableWidthPx = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, maximumAcceptableWidth);

    if((roi.width < minimumAcceptableWidthPx) || (maximumAcceptableWidthPx < roi.width) || (roi.x + roi.width > rotated.cols))
        return frameCenter;

    const CassetteType::CassetteDataToCrop cassetteData = CassetteType::CassetteDataToCropMap[cropParams.cassetteModel];
    double currentCroppedWidth = ImageAnalyzer::convertPixelsToMillimeters(cropParams.referenceNumberOfPixels, roi.width);
    if(currentCroppedWidth < cassetteData.widthWindowMm)
    {
        uint32_t marginToAdd = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels,
                                                                         cassetteData.widthWindowMm - currentCroppedWidth);
        roi.x -= marginToAdd / 2;
        roi.width += marginToAdd;
    }

    if(roi.x + roi.width > rotated.cols)
        roi.width = rotated.cols - roi.x - 1;
    if(roi.y + roi.height > rotated.rows)
        roi.height = rotated.rows - roi.y - 1;

    cv::Mat cropped = rotated(roi);

    return cropped;
}

std::vector<double> StripCropperRectangleDetection::determineThresholdValues(const std::vector<uint32_t>& histogram,
                                                                             const ColorChannelLAB& channel) const
{
    int numberOfPixelInHistogram, index;
    double divider;

    if(cropParams.lightningCondition == Visible)
        index = R_Visible;
    else if(channel == L)
        index = R;
    else
        index = BG;

    if((cropParams.deviceType == Proto_v2_PreCropped))
    {
        numberOfPixelInHistogram = NUMBER_OF_STRIP_PIXEL_IN_PROTOV2_PRECROPPED[index];
        divider = 4;
    }
    else
    {
        numberOfPixelInHistogram = NUMBER_OF_STRIP_PIXEL_IN_PROTOV1[index];
        divider = 3;
    }

    uint8_t theoricStripMaxValue;
    int marginDust;
    if(channel == L)
        marginDust = 40;
    else
        marginDust = 25;
    if(std::accumulate(histogram.end() - marginDust, histogram.end(), 0) > numberOfPixelInHistogram / divider)
        marginDust = 0;

    theoricStripMaxValue = WHITE - marginDust;

    std::vector<double> thresholdValues;
    auto maxValueIndex = histogram.begin() + theoricStripMaxValue + 1;

    int sum = 0;
    int scope = 0;

    while(sum < numberOfPixelInHistogram)
    {
        scope++;
        sum = std::accumulate(maxValueIndex - scope, maxValueIndex, 0);
    }

    thresholdValues.push_back(maxValueIndex - histogram.begin() - scope - 1);
    thresholdValues.push_back(maxValueIndex - histogram.begin() - 1);

    return thresholdValues;
}

std::vector<double> StripCropperRectangleDetection::determineThresholdValuesCassette(const std::vector<uint32_t>& histogram,
                                                                                     double heigthToDetermineNumberOfPixels,
                                                                                     const ColorChannelLAB& channel) const
{
    int numberOfPixelInHistogram;
    const CassetteType::CassetteDataToCrop cassetteData = CassetteType::CassetteDataToCropMap[cropParams.cassetteModel];

    uint32_t widthWindow = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, cassetteData.widthWindowMm);
    uint32_t height = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, heigthToDetermineNumberOfPixels);
    numberOfPixelInHistogram = widthWindow * height;

    bool needToRevertFromMatrix = CassetteType::MatrixDataToCropMap[cropParams.stripMatrix];

    if(!(cassetteData.revertImageToThreshold || needToRevertFromMatrix))
        numberOfPixelInHistogram *= 1.1;

    std::vector<double> thresholdValues;

    int sum = 0;
    int scope = 0;

    while(sum < numberOfPixelInHistogram)
    {
        scope++;
        sum = std::accumulate(histogram.begin(), histogram.begin() + scope, 0);
    }

    thresholdValues.push_back(0);
    thresholdValues.push_back(scope - 1);

    return thresholdValues;
}

cv::Mat StripCropperRectangleDetection::thresholdOneChannel(const cv::Mat& frameCenter,
                                                            double heightROIThreshold,
                                                            ColorChannelLAB channel,
                                                            std::vector<cv::Mat>* intermediateSteps) const
{
    cv::Mat frameCenterTransformed;

    if((cropParams.cassetteModel == K701WT) && (cropParams.lightningCondition == UV))
        frameCenterTransformed = frameCenter.clone();
    else
        cv::cvtColor(frameCenter, frameCenterTransformed, cv::COLOR_BGR2Lab);

    std::vector<cv::Mat> frameLABChannels(3);
    cv::split(frameCenterTransformed, frameLABChannels);

    cv::Mat channelImage = frameLABChannels[channel];

    if(cropParams.cassetteModel != WithoutCassette)
    {
        const CassetteType::CassetteDataToCrop cassetteData = CassetteType::CassetteDataToCropMap[cropParams.cassetteModel];
        bool invertImageFromStripMatrix = CassetteType::MatrixDataToCropMap[cropParams.stripMatrix];

        if(cassetteData.revertImageToThreshold || invertImageFromStripMatrix)
            channelImage = ImageAnalyzer::revertGrayScaleImage8Bits(channelImage);
    }

    if (intermediateSteps != nullptr)
        intermediateSteps->push_back(channelImage);

    std::vector<uint32_t> histogram = ImageAnalyzer::computeHistogram8Bits(channelImage);
    std::vector<double> thresholdValues;

    if(cropParams.cassetteModel != WithoutCassette)
        thresholdValues = determineThresholdValuesCassette(histogram, heightROIThreshold, channel);
    else
        thresholdValues = determineThresholdValues(histogram, channel);

    cv::Mat thresholdImageMin, thresholdImageMax;

    if (thresholdValues[0] == 0)
        thresholdImageMin = cv::Mat(channelImage.rows, channelImage.cols, channelImage.type(), cv::Scalar(WHITE));
    else
        cv::threshold(channelImage, thresholdImageMin, thresholdValues[0], WHITE, cv::THRESH_BINARY );

    cv::threshold(channelImage, thresholdImageMax, thresholdValues[1], WHITE, cv::THRESH_BINARY );
    cv::dilate(thresholdImageMax,
               thresholdImageMax,
               cv::getStructuringElement(cv::MORPH_RECT,cv::Size(4, 4)));

    cv::Mat thresholdImage = thresholdImageMin - thresholdImageMax;

    if (intermediateSteps != nullptr)
        intermediateSteps->push_back(thresholdImage);

    return thresholdImage;
}

ColorChannelLAB chooseChannelToCrop(LightningCondition lightningCondition,
                                 CassetteModel cassetteModel)
{
    ColorChannelLAB channel;

    if(cassetteModel == WithoutCassette)
        channel = L;
    else
    {
        const CassetteType::CassetteDataToCrop cassetteData = CassetteType::CassetteDataToCropMap[cassetteModel];
        if(lightningCondition == UV)
            channel = cassetteData.channelToCropUVFromVisible;
        else
            channel = cassetteData.channelToCropVisible;
    }

    return channel;
}

ROIsToCrop StripCropperRectangleDetection::checkAndInitCropROIMm(const cv::Mat& frame) const
{
    ROIsToCrop ROIs = {
            .ROIToThreshold = _ROIToThreshold,
            .outputROI = _outputROI
    };

    if(ROIs.ROIToThreshold.width < 0)
        ROIs.ROIToThreshold.width = ImageAnalyzer::convertPixelsToMillimeters(cropParams.referenceNumberOfPixels, frame.cols);
    if(ROIs.ROIToThreshold.height < 0)
        ROIs.ROIToThreshold.height = ImageAnalyzer::convertPixelsToMillimeters(cropParams.referenceNumberOfPixels, frame.rows);
    if(ROIs.outputROI.width < 0)
        ROIs.outputROI.width = ImageAnalyzer::convertPixelsToMillimeters(cropParams.referenceNumberOfPixels, frame.cols);
    if(ROIs.outputROI.height < 0)
        ROIs.outputROI.height = ImageAnalyzer::convertPixelsToMillimeters(cropParams.referenceNumberOfPixels, frame.rows);

    return ROIs;
}

cv::Mat StripCropperRectangleDetection::crop(const cv::Mat& frameToDetectRectangle, const cv::Mat& frameToCrop, std::vector<cv::Mat>* intermediateSteps) const
{
    ROIsToCrop ROIsForCropping = checkAndInitCropROIMm(frameToDetectRectangle);
    cv::Mat frameCenter = cropToThreshold(frameToDetectRectangle, ROIsForCropping.ROIToThreshold, intermediateSteps);

    cv::Mat image12bits, cropped;
    if (cropParams.imageDepth == Bits12)
    {
        image12bits = frameCenter.clone();
        image12bits.convertTo(frameCenter, CV_8UC3, 0.0625);
    }

    ColorChannelLAB channel = chooseChannelToCrop(cropParams.lightningCondition, cropParams.cassetteModel);
    cv::Mat channelThreshold = thresholdOneChannel(frameCenter, ROIsForCropping.ROIToThreshold.height, channel, intermediateSteps);

    if((cv::mean( channelThreshold )[0] > WHITE - 5) )
    {
        if(channel == L)
            channel = A;
        else
            channel = L;
        channelThreshold = thresholdOneChannel(frameCenter, ROIsForCropping.ROIToThreshold.height, channel, intermediateSteps);
    }

    cv::RotatedRect roi = detectRotatedRectangle(channelThreshold, intermediateSteps);
    if(cropParams.deviceType != Proto_v1)
    {
        uint32_t marginX = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIsForCropping.outputROI.upLeftCornerMarginX);
        uint32_t marginY = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIsForCropping.outputROI.upLeftCornerMarginY);
        uint32_t width = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIsForCropping.outputROI.width);
        uint32_t heigth = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels, ROIsForCropping.outputROI.height);

        if(marginX + width > frameToCrop.cols)
            width = frameToCrop.cols - marginX - 1;
        if(marginY + heigth > frameToCrop.rows)
            heigth = frameToCrop.rows - marginY - 1;

        cv::Rect outputROIPreCropped(marginX, marginY, width, heigth);
        frameCenter = frameToCrop(outputROIPreCropped);
    }

    if(cropParams.imageDepth == Bits12)
        frameCenter = image12bits;

    if(roi.size.empty())
        return frameCenter;

    return rotateAndCropImage(frameCenter, roi, channelThreshold, ROIsForCropping.outputROI.height, intermediateSteps);
}

DeviceFocal StripCropperRectangleDetection::determineProtoV3Focal(const std::string& focal)
{
    if(focal == "F041")
        return ProtoV3_F041;
    else if(focal == "F060")
        return ProtoV3_F060;
    else if(focal == "F060_L")
        return ProtoV3_F060_L;
    else if(focal == "F080")
        return ProtoV3_F080;
    else
        return UnknownFocal;
}

DeviceFocal StripCropperRectangleDetection::determineProtoV2Focal(const std::string& focal)
{
    if(focal == "F060")
        return ProtoV2_F060;
    else if(focal == "F041")
        return ProtoV2_DefaultFocal;
    else
        return UnknownFocal;
}

std::tuple<bool, CroppingValidationMessage> StripCropperRectangleDetection::isValidCroppedImage(const cv::Mat& cropped,
                                                                                  const CropParams& cropParams)
{
    const CassetteType::CassetteDataToCrop cassetteData = CassetteType::CassetteDataToCropMap[cropParams.cassetteModel];
    const uint32_t theoricCroppedWidthPx = ImageAnalyzer::convertMillimetersToPixels(cropParams.referenceNumberOfPixels,
                                                                                     cassetteData.widthWindowMm);
    uint32_t marginMinimum = 0.19 * theoricCroppedWidthPx;
    uint32_t marginMaximum = 0.12 * theoricCroppedWidthPx;

    uint32_t minimumAcceptableWidth = theoricCroppedWidthPx - marginMinimum;
    uint32_t maximumAcceptableWidth = theoricCroppedWidthPx + marginMaximum;

    if(cropped.cols < minimumAcceptableWidth)
        return {false, TooThin};
    else if(cropped.cols > maximumAcceptableWidth)
        return {false, TooLarge};

    return {true, IsValid};
}
