//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/08/26.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "ImgTools.h"
#include <opencv2/opencv.hpp>
#include "Converter.h"
#include <common/imageProcessing/ImageAnalyzer.h>

void drawLetter(cv::Mat stripReducedROI, const std::string & text, uint32_t x, uint32_t y)
{
    cv::Scalar textColor(237, 158, 0);

    cv::putText(stripReducedROI, text,
                cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX, 2.0,
                cv::Scalar(0,0,0), 8, cv::LINE_AA
                );

    cv::putText(stripReducedROI, text,
                cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX, 2.0,
                textColor, 4, cv::LINE_AA
                );
}

std::vector<uint8_t> ImgTools::extractROIForReport(std::vector<uint8_t> image, uint32_t controlLinePos, uint32_t testLinePos, double pixelsToMm, double lengthROIMm)
{
    auto croppedImage = cv::imdecode(image, cv::IMREAD_ANYCOLOR);

    uint32_t lengthCenterPx = ImageAnalyzer::convertMillimetersToPixels(pixelsToMm, lengthROIMm);
    int paddingToApply = croppedImage.cols - lengthCenterPx;
    auto [oneChannelImage, stripCroppedPadding] = ImageAnalyzer::extractChannelImageWithPadding(croppedImage, Red, paddingToApply);

    //cv::Vec3d whitePoint(255, 255, 255);
    auto whitePoint = 255.0;
    for (int i = 0; i < oneChannelImage.cols; i++)
    {
        oneChannelImage.at<double>(controlLinePos, i) = whitePoint;
        oneChannelImage.at<double>(testLinePos, i) = whitePoint;
    }

    int maxLines = testLinePos + std::ceil(pixelsToMm * 3);
    cv::Rect reducedROI = cv::Rect(cv::Point(0, 0),
                                   cv::Point(oneChannelImage.cols, std::min(maxLines, oneChannelImage.rows-1)));
    cv::Mat stripReducedROI = oneChannelImage(reducedROI);

    cv::rotate(stripReducedROI, stripReducedROI, cv::ROTATE_90_COUNTERCLOCKWISE);

    stripReducedROI.convertTo(stripReducedROI, CV_8U);
    cv::cvtColor(stripReducedROI, stripReducedROI, cv::COLOR_GRAY2BGR);

    //Mat(int rows, int cols, int type, const Scalar& s);
    cv::Mat blank(stripReducedROI.rows * 1.3, stripReducedROI.cols, CV_8UC3, cv::Scalar(255, 255, 255));
    stripReducedROI.copyTo(blank(cv::Rect(0, 0, stripReducedROI.cols, stripReducedROI.rows)));

    auto marginH = (pixelsToMm/8);
    auto marginV = (pixelsToMm/2);
    drawLetter(blank, "C", controlLinePos - marginH, stripReducedROI.rows + marginV );
    drawLetter(blank, "T", testLinePos - marginH, stripReducedROI.rows + marginV );

    //drawLetter(stripReducedROI, "C", controlLinePos + (pixelsToMm/2), stripReducedROI.rows - (pixelsToMm/4));
    //drawLetter(stripReducedROI, "T", testLinePos + (pixelsToMm/2), stripReducedROI.rows - (pixelsToMm/4));

    return Converter::cvMatToPng(blank );
}

bool ImgTools::isCapture8BitsOK(const cv::Mat &capture)
{
    if (capture.empty() || capture.rows == 0) return false;

    const int RedChannel = 2;
    auto channels = capture.channels();

    auto middleLine = capture.rows / 2;
    auto middleLinePos = capture.cols * middleLine * channels;
    for (int col = 0; col < capture.cols; col++)
    {
        auto pixelRedValue = capture.data[middleLinePos + col*channels + RedChannel];
        if (pixelRedValue < 230) return true;
    }

    return false;
}
