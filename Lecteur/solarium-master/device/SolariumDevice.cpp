//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "SolariumDevice.h"
#include "configure.h"

#include "dummyDevice/DummyBattery.h"
#include "dummyDevice/DummyCamera.h"
#include "dummyDevice/DummyLightSensor.h"
#include "dummyDevice/DummyTemperatureSensor.h"
#include "dummyDevice/DummyLed.h"

#ifdef USE_HW_LMX03
#include "peripherals-lmx3/Lmx3Battery.h"
#include "peripherals-lmx3/Lmx3LED.h"
#include "peripherals-lmx3/Lmx3LightSensor.h"
#include "peripherals-lmx3/Lmx3TemperatureSensor.h"
#else
#ifndef USE_DUMMY_PERIPHERALS
#include "peripherals/i2c_mux.h"
#include "hal/LedWrapper.h"
#include "hal/LightSensorWrapper.h"
#include "hal/TemperatureSensorWrapper.h"
#include "hal/BatteryWrapper.h"
#endif
#endif

#include "hal/CameraWrapper.h"

#include <Poco/Environment.h>

#include "../tools/DebugInfo.h"
#include "../SolariumApp.h"
#include "IPeripheralStatus.h"

#include <thread>
#include <chrono>
#include <Poco/Timestamp.h>
#include <inttypes.h>
#include <future>
#include <Poco/DateTimeFormatter.h>
#include <repository/AnalysisRepository.h>
#include <repository/StudiesRepository.h>

#include "measures/MeasureConfig.h"

enum SensorIdx {
    IDX_TOP,
    IDX_BOTTOM
};

// #####################################################################################################################
int getDeviceSetting(const std::string &section, const std::string &paramName, int defaultValue)
{
    if (section.empty())
        return SolariumApp::instance().config().getInt(paramName, defaultValue);
    else
    {
        auto paramFullName = section + "." + paramName;
        return SolariumApp::instance().config().getInt(paramFullName, defaultValue);
    }
}

std::string getDeviceSetting(const std::string &section, const std::string &paramName, const std::string &defaultValue)
{
    if (section.empty())
        return SolariumApp::instance().config().getString(paramName, defaultValue);
    else
    {
        auto paramFullName = section + "." + paramName;
        return SolariumApp::instance().config().getString(paramFullName, defaultValue);
    }
}

int getOptionalParam(const ICamera::TCameraParams & params, const std::string &paramName, int defaultValue)
{
    if (auto param = params.find(paramName); param != params.end())
    {
        try
        {
            return std::stoi(param->second);
        }
        catch (...)
        {
        }
    }
    return defaultValue;
}

int getOptionalParam(const SolariumDevice::TMeasureParams & params, const std::string &paramName, int defaultValue)
{
    return getOptionalParam(params.cameraParams, paramName, defaultValue);
}

int getFromParamsOrSettings(const ICamera::TCameraParams & params, const std::string & section, const std::string &paramName, int defaultValue)
{
    if (auto param = params.find(paramName); param != params.end())
    {
        try
        {
            return std::stoi(param->second);
        }
        catch (...)
        {
        }
    }
    return getDeviceSetting(section, paramName, defaultValue);
}

int getFromParamsOrSettings(const SolariumDevice::TMeasureParams & params, const std::string & section, const std::string &paramName, int defaultValue)
{
    return getFromParamsOrSettings(params.cameraParams, section, paramName, defaultValue);
}

std::string getOptionalParamString(const ICamera::TCameraParams & params, const std::string &paramName, std::string defaultValue)
{
    if (auto param = params.find(paramName); param != params.end())
    {
        try
        {
            return param->second;
        }
        catch (...)
        {
        }
    }
    return defaultValue;
}

std::string getOptionalParamString(const SolariumDevice::TMeasureParams & params, const std::string &paramName, std::string defaultValue)
{
    return getOptionalParamString(params.cameraParams, paramName, defaultValue);
}
// #####################################################################################################################

SolariumDevice::SolariumDevice()
 : setLEDs(this, &SolariumDevice::setLEDsImpl)
 , setAllLEDsOff(this, &SolariumDevice::setAllLEDsOffImpl)
 , getSensorsInfos(this, &SolariumDevice::getSensorsInfoImpl)
 , getSensorsDiagnostics(this, &SolariumDevice::getSensorsDiagnosticsImpl)
 , takePicture(this, &SolariumDevice::takePictureImpl)
 , runLightSensorsDiagnostics(this, &SolariumDevice::runLightSensorsDiagnosticsImpl)
 , takePictureForMeasure(this, &SolariumDevice::takePictureForMeasureImpl)
 , takePictureAndLightSensorsForMeasure(this, &SolariumDevice::takePictureAndLightSensorsForMeasureImpl)
{

}

SolariumDevice &SolariumDevice::instance() {
    static SolariumDevice device;
    return device;
}

void SolariumDevice::buildWithDummyPeripherals() {
    _battery = std::make_unique<DummyBattery>();

    _camera = std::make_unique<CameraWrapper>(getDeviceSetting("camera", "deviceNode", "/dev/video0"));

    _lightSensors[IDX_TOP] = std::make_unique<DummyLightSensor>();
    _lightSensors[IDX_BOTTOM] = std::make_unique<DummyLightSensor>();

    _temperatureSensors[IDX_TOP] = std::make_unique<DummyTemperatureSensor>();
    _temperatureSensors[IDX_BOTTOM] = std::make_unique<DummyTemperatureSensor>();

    _uvLeds = std::make_unique<DummyLed>();
    _visibleLeds = std::make_unique<DummyLed>();
}


void SolariumDevice::buildWithRealPeripherals() {

#ifdef USE_HW_LMX03

    bool useMaxBrightnessLimit = true;
    if (getHostname() == "lmx-301")
        useMaxBrightnessLimit = false;

    LOGGER.information("building lmx3LED UV");
    _uvLeds = std::make_unique<Lmx3LED>(Lmx3LED::LEDType::LED_UV, useMaxBrightnessLimit);
    LOGGER.information("building lmx3LED visible");
    _visibleLeds = std::make_unique<Lmx3LED>(Lmx3LED::LEDType::LED_Visible);
    LOGGER.information("LED build");
    //
    _lightSensors[IDX_TOP] = std::make_unique<Lmx3LightSensor>(true);
    _lightSensors[IDX_BOTTOM] = std::make_unique<Lmx3LightSensor>(false);

    LOGGER.information("Lmx3TemperatureSensor build");
    _temperatureSensors[IDX_TOP] = std::make_unique<Lmx3TemperatureSensor>(true);
    _temperatureSensors[IDX_BOTTOM] = std::make_unique<Lmx3TemperatureSensor>(false);

    LOGGER.information("Lmx3Battery build");
    _battery = std::make_unique<Lmx3Battery>();

#elif !defined(USE_DUMMY_PERIPHERALS)

    _i2cMux = std::make_shared<I2CMux>();

    _visibleLeds = std::make_unique<LedWrapperInvertedIntensity>(LedDriver::E_LED_TYPE_VIS, _i2cMux);
    _uvLeds = std::make_unique<LedWrapper>(LedDriver::E_LED_TYPE_UV, _i2cMux);

//    _lightSensors[IDX_TOP] = std::make_unique<LightSensorWrapper>(IDX_TOP, _i2cMux);
//    _lightSensors[IDX_BOTTOM] = std::make_unique<LightSensorWrapper>(IDX_BOTTOM, _i2cMux);

    _lightSensors[IDX_TOP] = std::make_unique<DummyLightSensor>();
    _lightSensors[IDX_BOTTOM] = std::make_unique<DummyLightSensor>();


    _temperatureSensors[IDX_TOP] = std::make_unique<TemperatureSensorWrapper>(TempSensor::E_TEMP_SNS_TYPE_LOCAL, _i2cMux);
    _temperatureSensors[IDX_BOTTOM] = std::make_unique<TemperatureSensorWrapper>(TempSensor::E_TEMP_SNS_TYPE_AMBIENTAL, _i2cMux);

    _battery = std::make_unique<BatteryWrapper>(_i2cMux);

#endif

    _camera = std::make_unique<CameraWrapper>(getDeviceSetting("camera", "deviceNode", "/dev/video0"));

}

std::string SolariumDevice::getHostname() const {
    return Poco::Environment::nodeName();
}

void SolariumDevice::initialize() {

    LOGGER.information("device initializing");

#ifdef USE_DUMMY_PERIPHERALS
    buildWithDummyPeripherals();
#else
    buildWithRealPeripherals();
#endif
    _currentMeasureConfig = std::make_unique<MeasureConfig>("covid");

    _camera->initialize();
#ifdef USE_HW_LMX03
    /*
     * _uvLeds->enable();
    _visibleLeds->enable();
     */
    _battery->init();
    LOGGER.information("device initializing done");
#endif
}


void SolariumDevice::uninitialize()
{
    _camera->uninitialize();
}


Poco::Void SolariumDevice::setLEDsImpl(const LEDParams & params)
{
    ILedDriver *ledDriver = nullptr;
    if (params.ledType == LEDParams::UV)
        ledDriver = _uvLeds.get();
    else
        ledDriver = _visibleLeds.get();

    try
    {
        //debugLog("[LEDs] type " << (params.ledType == UV ? "uv" : "visible") << " setIntensity " << unsigned(params.intensity) << " enabled :" << (params.enabled.has_value() && params.enabled.value())) ;
        if (params.enabled.has_value())
        {
            if (params.enabled.value())
                ledDriver->enable(params.intensity, params.intensity);
            else
                ledDriver->disable();
        }
        else
            ledDriver->setIntensity(params.intensity, params.intensity);

    }
    catch (const std::exception &ex)
    {
        poco_debug_f1(LOGGER, "[Error playing with the LEDS : %s]", std::string(ex.what()));
    }

    return Poco::Void();
}

void SolariumDevice::turnAllLedsOff()
{
    _uvLeds.get()->disable();
    _visibleLeds.get()->disable();
}

Poco::Void SolariumDevice::setAllLEDsOffImpl(const Poco::Void &)
{
    turnAllLedsOff();
    return Poco::Void();
}

// #####################################################################################################################
// Sensors
// #####################################################################################################################

SolariumDevice::TLightResult readOneLightSensor(ILightSensor * sensor) {
    try
    {
        return sensor->values();
    }
    catch (const std::exception & ex)
    {
        return SolariumDevice::TLightResult();
    }
};

SolariumDevice::TTemperatureResult readOneTempSensor(ITemperatureSensor * sensor) {
    try
    {
        return sensor->values();
    }
    catch (const std::exception & ex)
    {
        return SolariumDevice::TTemperatureResult();
    }
};

std::pair<SolariumDevice::TLightResult, SolariumDevice::TTemperatureResult> readSensorsBoard(ILightSensor *lightSensor, ITemperatureSensor *temperatureSensor)
{
    auto lightValues = readOneLightSensor(lightSensor);
    auto tempValues = readOneTempSensor(temperatureSensor);
    return {lightValues, tempValues};
}

SolariumDevice::TSensorsInfos SolariumDevice::getSensorsInfoImpl(const Poco::Void &)
{
    TSensorsInfos infos;

    auto futureRes0 = std::async(readSensorsBoard, _lightSensors[0].get(), _temperatureSensors[0].get());
    auto futureRes1 = std::async(readSensorsBoard, _lightSensors[1].get(), _temperatureSensors[1].get());

    auto sensors0 = futureRes0.get();
    auto sensors1 = futureRes1.get();

    infos.lights[0] = sensors0.first;
    infos.temperatures[0] = sensors0.second;

    infos.lights[1] = sensors1.first;
    infos.temperatures[1] = sensors1.second;

    try
    {
        infos.battery = _battery->values();
    }
    catch (const std::exception & ex)
    {
        poco_debug(LOGGER, std::string(ex.what()));
    }

    return infos;
}

std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult> SolariumDevice::getLightSensorsValues(int delayMS)
{
    if (delayMS > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMS));

    auto futureRes0 = std::async(readOneLightSensor, _lightSensors[0].get());
    auto futureRes1 = std::async(readOneLightSensor, _lightSensors[1].get());
    return { futureRes0.get(), futureRes1.get() };
}


// #####################################################################################################################
// Capture
// #####################################################################################################################

std::vector<uint8_t> SolariumDevice::takePictureImpl(const ICamera::TCameraParams &params)
{
    auto captureROI = ICamera::FullROI;

    auto roiSize = getOptionalParamString(params, CAM_PARAM_ROI_SIZE, "full");
    if (roiSize == std::string("cropped"))
    {
        captureROI = ICamera::DefaultROI;
        captureROI.x = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_X, 384);
        captureROI.y = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_Y, 310);
        captureROI.width = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_WIDTH, 1056);
        captureROI.height = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_HEIGHT, 504);
    }

    return _camera->takePicture(params, captureROI);
}


cv::Mat SolariumDevice::takePictureForMeasureImpl(const TMeasureParams & params)
{
    try
    {
        turnAllLedsOff();
        auto captureROI = ICamera::DefaultROI;
        captureROI.x = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_X, 384);
        captureROI.y = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_Y, 310);
        captureROI.width = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_WIDTH, 1056);
        captureROI.height = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_HEIGHT, 504);

        auto roiSize = getOptionalParamString(params, CAM_PARAM_ROI_SIZE, "cropped");
        if (roiSize == std::string("full"))
        {
            captureROI = ICamera::FullROI;
        }

        _camera->setupCapture(params.cameraParams, captureROI);

        ICamera::FLedLightning ledLightning = [this, &params](bool enable) {
            if (enable) {
                Poco::DateTimeFormatter dtf;

                setLEDsImpl(params.visibleLEDs);
                setLEDsImpl(params.uvLEDs);
                poco_information_f3(LOGGER, "-> %s : ledLightning uv (%b) visible (%b)",
                                    dtf.format(Poco::Timestamp(), "%S.%i"),
                                    params.uvLEDs.enabled.value_or(false),
                                    params.visibleLEDs.enabled.value_or(false)
                                    );

            } else {
                turnAllLedsOff();
            }
        };

        auto res = _camera->takePictureAsMat(ledLightning);
        return res;
    }
    catch (const std::exception & ex)
    {
        poco_error_f1(LOGGER, "TakePicture failed : %s", std::string(ex.what()));
        return cv::Mat();
    }
}

std::pair<cv::Mat,std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult>> SolariumDevice::takePictureAndLightSensorsForMeasureImpl(const TMeasureParams & params)
{
    try
    {
        turnAllLedsOff();
        auto captureROI = ICamera::DefaultROI;
        captureROI.x = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_X, 384);
        captureROI.y = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_Y, 310);
        captureROI.width = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_WIDTH, 1056);
        captureROI.height = getFromParamsOrSettings(params, "camera", CAM_PARAM_ROI_HEIGHT, 504);

        auto roiSize = getOptionalParamString(params, CAM_PARAM_ROI_SIZE, "cropped");
        if (roiSize == std::string("full"))
        {
            captureROI = ICamera::FullROI;
        }

        _camera->setupCapture(params.cameraParams, captureROI);

        std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult> sensorsValues;

        ICamera::FLedLightning ledLightning = [this, &params, &sensorsValues](bool enable) {
            if (enable) {
                setLEDsImpl(params.visibleLEDs);
                setLEDsImpl(params.uvLEDs);
            } else {
                sensorsValues = getLightSensorsValues();
                turnAllLedsOff();
            }
        };

        auto res = _camera->takePictureAsMat(ledLightning);
        return { res, sensorsValues };
    }
    catch (const std::exception & ex)
    {
        poco_error_f1(LOGGER, "TakePicture failed : %s", std::string(ex.what()));
        return { cv::Mat(), { {},{}} };
    }
}

// #####################################################################################################################
// Diagnostics
// #####################################################################################################################


std::vector<SolariumDevice::TLightSensorsTimestamp> SolariumDevice::runLightSensorsDiagnosticsImpl(const std::pair<int, int> & uvLedIntensityAndDurationInSeconds)
{
    int uvLedIntensity = uvLedIntensityAndDurationInSeconds.first;
    int durationInSeconds = uvLedIntensityAndDurationInSeconds.second;

    turnAllLedsOff();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<SolariumDevice::TLightSensorsTimestamp> results;

    Poco::Timespan diagnosticsDuration( durationInSeconds * Poco::Timespan::SECONDS);
    Poco::Timestamp now, timeTarget = now + diagnosticsDuration, started;

    do
    {
        _uvLeds->setIntensity(uvLedIntensity, uvLedIntensity);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        SolariumDevice::TLightSensorsTimestamp sensorsValues;
        auto [ sensor1, sensor2 ] = getLightSensorsValues();
        _uvLeds->disable();
        sensorsValues.sensor1 = sensor1;
        sensorsValues.sensor2 = sensor2;
        sensorsValues.timestamp = started.elapsed();
        results.push_back(sensorsValues);

        std::this_thread::sleep_for(std::chrono::milliseconds(1400));

        now.update();
    } while(now < timeTarget);

    _uvLeds->disable();

    return results;
}

SolariumDevice::TSensorsDiagnostics SolariumDevice::getSensorsDiagnosticsImpl(const Poco::Void &)
{
    TSensorsDiagnostics infos;
    Poco::Timestamp now;

    try
    {
        now.update();
        infos.temperatures[0].first = _temperatureSensors[0]->values();
        infos.temperatures[0].second = now.elapsed();

        now.update();
        infos.temperatures[1].first = _temperatureSensors[1]->values();
        infos.temperatures[1].second = now.elapsed();
    }
    catch (const std::exception & ex)
    {
        poco_debug(LOGGER, std::string(ex.what()));
    }

    try
    {
        now.update();
        infos.lights[0].first = _lightSensors[0]->values();
        infos.lights[0].second = now.elapsed();

        now.update();
        infos.lights[1].first = _lightSensors[1]->values();
        infos.lights[1].second = now.elapsed();
    }
    catch (const std::exception & ex)
    {
        poco_debug(LOGGER, std::string(ex.what()));
    }

    try
    {
        now.update();
        infos.battery.first = _battery->values();
        infos.battery.second = now.elapsed();
    }
    catch (const std::exception & ex)
    {
        poco_debug(LOGGER, std::string(ex.what()));
    }

    return infos;
}

void SolariumDevice::setMeasureConfig(const MeasureConfig &config)
{
    _currentMeasureConfig = std::make_unique<MeasureConfig>(config);
}

const MeasureConfig &SolariumDevice::getMeasureConfig() const
{
    return *_currentMeasureConfig.get();
}

SolariumDevice::TSpaceInfos SolariumDevice::getSpaceInfos()
{
    TSpaceInfos infos;

    AnalysisRepository dataRepo;
    infos.dataFileSystemInfo = dataRepo.getSpaceInfo();

    StudiesRepository studiesRepo;
    infos.studyFileSystemInfo = studiesRepo.getSpaceInfo();

    return infos;
}

