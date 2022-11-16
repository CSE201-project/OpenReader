//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/08/26.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "UEyeCameraParams.h"
#include <ueye.h>
#include <iostream>
#include <opencv2/opencv.hpp>


#define PRINT(msg) std::cout << msg << std::endl
#define LOG_INFO(msg) std::cout << "[INFO] [UEyeCameraParams::" << __FUNCTION__ <<" l." << __LINE__ << "] : " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] [UEyeCameraParams::" << __FUNCTION__ <<" l." << __LINE__ << "] : " << msg << std::endl

static std::map<std::string, int> ColorModeParams = {
        {"8bits", IS_CM_BGR8_PACKED},
        {"10bits", IS_CM_BGR10_PACKED},
        {"12bits", IS_CM_BGR12_UNPACKED}
};

int Param2ColorMode(const std::string & param)
{
    if (auto pos = ColorModeParams.find(param); pos != ColorModeParams.end())
    {
        return pos->second;
    }
    else
    {
        return ColorModeParams.find("8bits")->second;
    }
}

static std::map<std::string, int> ColorConvertParams = {
        {"BGR2RGB", cv::COLOR_BGR2RGB},
        {"BayerBG2RGB", cv::COLOR_BayerBG2RGB},
        {"BayerGB2RGB", cv::COLOR_BayerGB2RGB},
        {"BayerGR2RGB", cv::COLOR_BayerGR2RGB}
};

static std::map<std::string, uint32_t> AWBParams = {
        {"off", IS_AUTOPARAMETER_DISABLE},
        {"on", IS_AUTOPARAMETER_ENABLE},
        {"once", IS_AUTOPARAMETER_ENABLE_RUNONCE}
};

UEyeCameraParams::UEyeCameraParams(const ICamera::TCameraParams &params)
        : _params(params)
{

}

void UEyeCameraParams::displayCameraParams() const
{
    PRINT("Camera Params, size: " << _params.size());
    for (const auto &[key, val] : _params)
    {
        PRINT(" [ " << key << " : " << val << " ]");
    }
}

int UEyeCameraParams::GetColorDepth() const
{
    auto pos = _params.find(CAM_PARAM_COLOR_DEPTH);
    return Param2ColorMode(pos != _params.end() ? pos->second : "-1");
}

int UEyeCameraParams::GetColorConvert() const
{
    auto pos = _params.find("color_convert");
    if (pos != _params.end())
    {
        auto paramValue = pos->second;
        auto colorConvertPos = ColorConvertParams.find(paramValue);
        if (colorConvertPos != ColorConvertParams.end())
        {
            return colorConvertPos->second;
        }
    }
    return 0;
}

int UEyeCameraParams::GetBitsPerPixel() const
{
    int result = 0;
    switch (GetColorDepth())
    {
        case IS_CM_RGBA12_UNPACKED:
        case IS_CM_BGRA12_UNPACKED:
            result = 64;
            break;

        case IS_CM_RGB12_UNPACKED:
        case IS_CM_BGR12_UNPACKED:
        case IS_CM_RGB10_UNPACKED:
        case IS_CM_BGR10_UNPACKED:
            result = 48;
            break;

        case IS_CM_RGB10_PACKED:
        case IS_CM_BGR10_PACKED:
        case IS_CM_RGBA8_PACKED:
        case IS_CM_BGRA8_PACKED:
        case IS_CM_RGBY8_PACKED:
        case IS_CM_BGRY8_PACKED:
            result = 32;
            break;

        case IS_CM_RGB8_PACKED:
        case IS_CM_BGR8_PACKED:
        case IS_CM_RGB8_PLANAR:
            result = 24;
            break;

        case IS_CM_BGR565_PACKED:
        case IS_CM_UYVY_PACKED:
        case IS_CM_CBYCRY_PACKED:
        case IS_CM_SENSOR_RAW16:
        case IS_CM_SENSOR_RAW12:
        case IS_CM_SENSOR_RAW10:
        case IS_CM_MONO16:
        case IS_CM_MONO12:
        case IS_CM_MONO10:
            result = 16;
            break;

        case IS_CM_BGR5_PACKED:
            result = 15;
            break;

        case IS_CM_MONO8:
        case IS_CM_SENSOR_RAW8:
        default:
            result = 8;
            break;
    }
    return result;
}

uint32_t UEyeCameraParams::GetAutoWhiteBalance() const
{
    auto pos = _params.find(CAM_PARAM_AWB);
    if (pos != _params.end())
    {
        auto param = pos->second;
        auto paramValue = AWBParams.find(param);
        if (paramValue != AWBParams.end())
        {
            return paramValue->second;
        }
    }
    return IS_AUTOPARAMETER_DISABLE;
}

std::optional<double> UEyeCameraParams::GetExposure() const
{
    auto pos = _params.find(CAM_PARAM_EXPOSURE);
    if (pos != _params.end())
    {
        try
        {
            auto exposureString = pos->second;
            auto result = std::stod(exposureString);
            return result;
        }
        catch (const std::exception & ex)
        {
            LOG_INFO("Cannot read exposure params : " << ex.what());
        }
    }
    return {};
}

std::optional<UEyeCameraParams::TGainParam> UEyeCameraParams::GetGain() const
{
    auto masterGain = GetOptionalParam<int>(CAM_PARAM_GAIN_MASTER);
    auto redGain = GetOptionalParam<int>(CAM_PARAM_GAIN_RED);
    auto greenGain = GetOptionalParam<int>(CAM_PARAM_GAIN_GREEN);
    auto blueGain = GetOptionalParam<int>(CAM_PARAM_GAIN_BLUE);

    if (!masterGain.has_value()
    && !redGain.has_value()
    && !greenGain.has_value()
    && !blueGain.has_value())
        return {};

    return std::optional<TGainParam>({
        .master = masterGain.value_or(IS_IGNORE_PARAMETER),
        .red = redGain.value_or(IS_IGNORE_PARAMETER),
        .green = greenGain.value_or(IS_IGNORE_PARAMETER),
        .blue = blueGain.value_or(IS_IGNORE_PARAMETER)
    });
}

template<class T>
[[maybe_unused]] std::optional<T> UEyeCameraParams::GetOptionalParam(const std::string &paramName) const
{
    auto pos = _params.find(paramName);
    if (pos != _params.end())
    {
        try
        {
            auto paramString = pos->second;
            std::istringstream iss(paramString);
            T result;
            iss >> result;
            return result;
        }
        catch (const std::exception & ex)
        {
            LOG_INFO("Cannot read " << paramName << " params : " << ex.what());
        }
    }
    return {};
}

std::optional<int> UEyeCameraParams::GetBlackLevelMode() const
{
    return GetOptionalParam<int>(CAM_PARAM_BLACKLEVEL_MODE);
}

std::optional<int> UEyeCameraParams::GetBlackLevelOffset() const
{
    return GetOptionalParam<int>(CAM_PARAM_BLACKLEVEL_OFFSET);
}

