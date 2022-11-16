//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/06/09.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_PARAMSFINDER_H
#define SOLARIUM_PARAMSFINDER_H

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <device/SolariumDevice.h>

class StripCropperRectangleDetection;

class ParamsFinder {
public:

    struct ROIPos {
        int32_t x;
        int32_t y;
    };

    struct ROI {
        ROIPos topLeft;
        ROIPos bottomRight;
    };

    ParamsFinder(const std::string & name, const std::string & cassetteName);

    virtual ~ParamsFinder();

    typedef std::function<void(const std::string &)> runningLogCb;
    typedef std::vector<int> TLoopValues;

    void run(const std::map<std::string, TLoopValues> & inputParams, runningLogCb cb);

private:
    std::string _name;
    std::string _cassetteName;
    cv::Mat _croppingImage;
    std::unique_ptr<StripCropperRectangleDetection> _cropper;
    std::optional<std::pair<uint32_t, uint32_t>> _linesPos;

    std::tuple<int32_t, int32_t, std::vector<double>> takeControlSnapshot(const Poco::Timestamp & currentTime);
    std::pair<cv::Mat,std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult>> takeVerticalSnapshot(std::map<std::string, std::string> cameraParams, LEDParams uvLeds, LEDParams visibleLeds);
    std::pair<cv::Mat,std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult>> captureWindowCassette(std::map<std::string, std::string> cameraParams, LEDParams uvLeds, LEDParams visibleLeds);
    void initCropper();
};

std::vector<ICamera::TCameraParams> computeAcquisitionParamsCombination(std::map<std::string, ParamsFinder::TLoopValues> inputParams);
cv::Mat drawCTAndTLROI(const cv::Mat & redChannelImage, double oneMmLengthInPixel, int controlLinePos, int testLinePos);

#endif //SOLARIUM_PARAMSFINDER_H
