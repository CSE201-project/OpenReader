//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_ICAMERA_H
#define SOLARIUM_ICAMERA_H

#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <opencv2/core/mat.hpp>

#include "configure.h"

#define CAM_PARAM_EXPOSURE          "exposure"
#define CAM_PARAM_GAIN_MASTER       "gain_master"
#define CAM_PARAM_GAIN_RED          "gain_red"
#define CAM_PARAM_GAIN_GREEN        "gain_green"
#define CAM_PARAM_GAIN_BLUE         "gain_blue"
#define CAM_PARAM_AWB               "awb"
#define CAM_PARAM_COLOR_DEPTH       "color_depth"
#define CAM_PARAM_SNAPSHOT_DELAY    "snapshot_delay"
#define CAM_PARAM_LED_DURATION      "led_duration"
#define CAM_PARAM_LED_DELAY         "led_delay"
#define CAM_PARAM_LED_PEAKTIME      "led_peaktime"
#define CAM_PARAM_ROI_OFFSET        "roi_offset"
#define CAM_PARAM_ROI_X             "roi_x"
#define CAM_PARAM_ROI_Y             "roi_y"
#define CAM_PARAM_ROI_WIDTH         "roi_width"
#define CAM_PARAM_ROI_HEIGHT        "roi_height"
#define CAM_PARAM_ROI_SIZE          "roi_size"
#define CAM_PARAM_BLACKLEVEL_MODE   "blacklevel_mode"
#define CAM_PARAM_BLACKLEVEL_OFFSET "blacklevel_offset"

// DEFAULT_LED_WARMUP_DURATION 200ms
#define DEFAULT_LED_WARMUP_DURATION 200 * 1000

class ICamera {
public:
    typedef std::map<std::string, std::string> TCameraParams;

    struct TRectangle {
        int32_t x = 8;
        int32_t y = 8;
        int32_t width = 1920;
        int32_t height = 1080;
    };

#ifdef USE_UEYE_CAMERA
    constexpr static const TRectangle DefaultROI = {
        .x = 384,
        .y = 310,
        .width = 1056,
        .height = 504
    };

    constexpr static const TRectangle FullROI = {
            .x = 8,
            .y = 8,
            .width = 1920,
            .height = 1080
    };
#elif defined(USE_V4L2_CAMERA)
    constexpr static const TRectangle DefaultROI = {
            .x = 336,
            .y = 432,
            .width = 1920,
            .height = 1080
    };

    constexpr static const TRectangle FullROI = {
            .x = 0,
            .y = 0,
            .width = 2592,
            .height = 1944
    };
#else
    constexpr static const TRectangle DefaultROI = {
        .x = 384,
        .y = 310,
        .width = 1056,
        .height = 504
    };

    constexpr static const TRectangle FullROI = {
            .x = 8,
            .y = 8,
            .width = 1920,
            .height = 1080
    };
#endif
    virtual void initialize() = 0;
    virtual void uninitialize() {}

    void setupCapture(const TCameraParams & params) {
        setupCapture(params, TRectangle());
    }
    virtual void setupCapture(const TCameraParams & params, const TRectangle &roi) {};

    typedef std::function<void(bool enable)> FLedLightning;
    virtual cv::Mat takePictureAsMat(FLedLightning ledLightning = nullptr) = 0;

    virtual std::vector<uint8_t> takePicture(const TCameraParams & params, const TRectangle &roi = FullROI) = 0;

    virtual TCameraParams getCurrentParams() const { return TCameraParams(); }

    typedef std::unique_ptr<ICamera> UPtr;
};

#endif //SOLARIUM_ICAMERA_H
