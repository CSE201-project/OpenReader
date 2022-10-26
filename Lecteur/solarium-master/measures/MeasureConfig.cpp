//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/12.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "MeasureConfig.h"
#include <regex>
#include <string>

#include "../tools/DebugInfo.h"
#include "../tools/Converter.h"
#include "SolariumApp.h"

#define DEFAULT_ROI_SIZES { 1.0, 1.3, 1.0 }

uint8_t getDefaultUVIntensity()
{
    return SolariumApp::instance().config().getUInt("camera.uv_leds_intensity", 255);
}

MeasureConfig::MeasureConfig(const std::string &measureType)
    : _stripType(measureType)
{
    _cameraParams = getCameraParamsForType(measureType);

    if (auto it = _cameraParams.find("LEDs_params"); it != _cameraParams.end())
    {
        auto ledsParams = Converter::jsonParams2StringsMap(it->second);
        _ledsParams[LEDParams::UV] = LEDParams::parse(ledsParams["uv"], LEDParams::UV);
        _ledsParams[LEDParams::Visible] = LEDParams::parse(ledsParams["visible"], LEDParams::Visible);
    }
    else
    {
        auto & ledsUv = _ledsParams[LEDParams::UV];
        ledsUv.ledType = LEDParams::UV;
        ledsUv.intensity = getDefaultUVIntensity();
        ledsUv.enabled = true;

        auto & ledsVisible = _ledsParams[LEDParams::Visible];
        ledsVisible.ledType = LEDParams::Visible;
        ledsVisible.intensity = 25;
        ledsVisible.enabled = false;
    }
}

MeasureConfig::MeasureConfig(const std::map<std::string, std::string> &params)
        : _stripType("calibration")
{
    _cameraParams = params;

    auto & ledsUv = _ledsParams[LEDParams::UV];
    ledsUv.ledType = LEDParams::UV;
    ledsUv.intensity = getDefaultUVIntensity();
    ledsUv.enabled = false;

    auto & ledsVisible = _ledsParams[LEDParams::Visible];
    ledsVisible.ledType = LEDParams::Visible;
    ledsVisible.intensity = 25;
    ledsVisible.enabled = false;

    if (const auto & ledsParamsIter = params.find("LEDs_params"); ledsParamsIter != params.end())
    {
        auto ledsParams = Converter::jsonParams2StringsMap(ledsParamsIter->second);
        if (auto visibleIter = ledsParams.find("visible"); visibleIter != ledsParams.end())
        {
            _ledsParams[LEDParams::Visible] = LEDParams::parse(visibleIter->second, LEDParams::Visible);
        }
        if (auto uvIter = ledsParams.find("uv"); uvIter != ledsParams.end())
        {
            _ledsParams[LEDParams::UV] = LEDParams::parse(uvIter->second, LEDParams::UV);
        }
    }
    LOGGER.information("Current LEDs uv (%s) visible (%s)",
                       std::string(_ledsParams[LEDParams::Visible].enabled && _ledsParams[LEDParams::Visible].enabled.value() ? "enabled" : "disabled"),
                       std::string(_ledsParams[LEDParams::UV].enabled && _ledsParams[LEDParams::UV].enabled.value() ? "enabled" : "disabled"));
}

const ICamera::TCameraParams & MeasureConfig::getCameraParams() const
{
    return _cameraParams;
}

const std::string &MeasureConfig::getStripType() const
{
    return _stripType;
}

const LEDParams & MeasureConfig::getLEDParams(LEDParams::LEDType type) const
{
    return _ledsParams[type];
}

ColorChannel MeasureConfig::getColorChannel() const
{
    if (auto res = _cameraParams.find("color_channel"); res != _cameraParams.end())
    {
        if (res->second == "red")
            return Red;
        else if (res->second == "blue")
            return Blue;
        else if (res->second == "green")
            return Green;
    }

    return Red;
}

uint8_t MeasureConfig::getColorDepth() const
{
    auto res = _cameraParams.find("color_depth");
    return (res != _cameraParams.end() && res->second == "12bits") ? 12 : 8;
}

std::string MeasureConfig::ToJSON() const
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"strip_type\":\"calibration\"";
    for (const auto & param : _cameraParams)
    {
        if (param.first != std::string("LEDs_params"))
            oss << ",\"" << param.first << "\":\"" << param.second << "\"";
    }
    oss << ",\"LEDs_params\":{";

    const auto & ledVisible = _ledsParams[LEDParams::Visible];
    oss << "\"visible\":{\"status\":\"" << ((ledVisible.enabled.has_value() && ledVisible.enabled.value()) ? "enabled" : "disabled") << "\",\"intensity\":" << std::to_string(ledVisible.intensity)  << "}";

    const auto & ledUV = _ledsParams[LEDParams::UV];
    oss << ",\"uv\":{\"status\":\"" << ((ledUV.enabled.has_value() && ledUV.enabled.value()) ? "enabled" : "disabled") << "\",\"intensity\":" << std::to_string(ledUV.intensity)  << "}";

    oss << "}}";

    return oss.str();
}

bool MeasureConfig::getUsingCassette() const
{
    auto res = _cameraParams.find("strip_packaging");
    return (res != _cameraParams.end() && res->second == "cassette");
}

double MeasureConfig::getPadding() const
{
    return SolariumApp::instance().config().getDouble("calib.covert.length_roi", 2);
}

std::string MeasureConfig::getCassetteName() const
{
    return "K701BT_SE";
}

double MeasureConfig::getDistanceCL2TL() const
{
    return SolariumApp::instance().config().getDouble("calib.covert.distance_cl_tl", 6);
}

double MeasureConfig::getPixelsToMm() const
{
    return SolariumApp::instance().config().getDouble("camera.pixels_to_mm", 45);
}

double MeasureConfig::getTLLength() const
{
    return SolariumApp::instance().config().getDouble("calib.covert.length_tl", 1);
}

double MeasureConfig::getMaskLength() const
{
    return 2.3;
}

std::string MeasureConfig::getAlgoInfos(const std::string & path) const
{
    std::ostringstream oss;
    oss << "calib." << _stripType << "." << path;
    auto fullname = oss.str();
    return SolariumApp::instance().config().getString(fullname, "");
}

ICamera::TCameraParams MeasureConfig::getCameraParamsForType(const std::string & type) const
{
    auto cameraParams = SolariumApp::instance().config().getString(std::string("camera.") + type, "");
    if (cameraParams.empty())
        cameraParams = SolariumApp::instance().config().getString(std::string("camera.default_params"), "");

    return Converter::jsonParams2StringsMap(cameraParams);
}

bool MeasureConfig::isCroppingParam() const
{
    if (auto res = _cameraParams.find("cropping_param");
        res != _cameraParams.end() && res->second == "true")
    {
        return true;
    }
    return false;
}

bool MeasureConfig::isImageInAmbientLight() const
{
    return _ledsParams[LEDParams::Visible].enabled.value_or(false);
}

MeasureConfig MeasureConfig::LoadDefaults(const std::string &type)
{
    auto savedConfig = SolariumApp::instance().config().getString(std::string("camera.") + type, "");
    if (savedConfig.empty())
        return MeasureConfig("default");
    else
        return MeasureConfig(Converter::jsonParams2StringsMap(savedConfig));
}

LensFocal MeasureConfig::getLensFocal() const
{
    auto focalString = SolariumApp::instance().config().getString(std::string("camera.lensFocal"), "F041");
#ifdef USE_UEYE_CAMERA
    return LensFocal(focalString, LensFocal::Series2);
#else
    return LensFocal(focalString, LensFocal::Series3);
#endif
}

// for string delimiter
std::vector<double> split (std::string s, std::string delimiter = " ") {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<double> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (std::stod(token));
    }

    res.push_back (std::stod(s.substr (pos_start)));
    return res;
}

std::tuple<double, double, double> MeasureConfig::getProxNoiseROISizes() const
{
    std::ostringstream oss;
    oss << "calib." << _stripType << ".roi_sizes";
    auto fullname = oss.str();

    auto roiSizesString = SolariumApp::instance().config().getString(fullname, "");
    if (roiSizesString.empty())
        return DEFAULT_ROI_SIZES;

    try
    {
        auto sizes = split(roiSizesString);
        return { sizes.at(0), sizes.at(1), sizes.at(2)};
    }
    catch (const std::exception &ex)
    {
        LOGGER.error("ProxNoise ROI Sizes Bad format, using default values");
        return DEFAULT_ROI_SIZES;
    }
}


