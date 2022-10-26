//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_SOLARIUMDEVICE_H
#define SOLARIUM_SOLARIUMDEVICE_H

#include <utility>
#include <filesystem>
namespace fs = std::filesystem;

#include "ILightSensor.h"
#include "ITemperatureSensor.h"
#include "ICamera.h"
#include "IBattery.h"
#include "ILedDriver.h"
#include "models/LEDParams.h"

#include <Poco/ActiveDispatcher.h>
#include <Poco/ActiveMethod.h>
#include <Poco/Void.h>
#include <Poco/Timespan.h>
using namespace Poco;

#include <opencv2/core/mat.hpp>
#include <models/SpaceInfo.h>

class I2CMux;
class MeasureConfig;

class SolariumDevice : public ActiveDispatcher {
public:
    static SolariumDevice & instance();

    void initialize();
    void uninitialize();

    std::string getHostname() const;

    ActiveMethod<Poco::Void, LEDParams, SolariumDevice, ActiveStarter<ActiveDispatcher>> setLEDs;
    ActiveMethod<Poco::Void, Poco::Void, SolariumDevice, ActiveStarter<ActiveDispatcher>> setAllLEDsOff;

    typedef std::optional<ITemperatureSensor::TResult> TTemperatureResult;
    typedef std::optional<ILightSensor::TResult> TLightResult;
    typedef std::optional<IBattery::TReading> TBatteryResult;

    typedef struct {
        TTemperatureResult temperatures[2];
        TLightResult lights[2];
        TBatteryResult battery;
    } TSensorsInfos;

    ActiveMethod<TSensorsInfos, Poco::Void, SolariumDevice, ActiveStarter<ActiveDispatcher>> getSensorsInfos;

    struct TSpaceInfos {
        SpaceInfo dataFileSystemInfo;
        SpaceInfo studyFileSystemInfo;
    };

    TSpaceInfos getSpaceInfos();

    typedef struct {
        std::pair<TTemperatureResult, Poco::Timespan> temperatures[2];
        std::pair<TLightResult, Poco::Timespan> lights[2];
        std::pair<TBatteryResult, Poco::Timespan> battery;
    } TSensorsDiagnostics;

    ActiveMethod<TSensorsDiagnostics, Poco::Void, SolariumDevice, ActiveStarter<ActiveDispatcher>> getSensorsDiagnostics;

    typedef struct {
        Poco::Timestamp::TimeDiff timestamp;
        std::optional<ILightSensor::TResult> sensor1;
        std::optional<ILightSensor::TResult> sensor2;
    } TLightSensorsTimestamp;

    ActiveMethod<std::vector<TLightSensorsTimestamp>, std::pair<int, int>, SolariumDevice, ActiveStarter<ActiveDispatcher>> runLightSensorsDiagnostics;

    ActiveMethod<std::vector<uint8_t>, ICamera::TCameraParams, SolariumDevice, ActiveStarter<ActiveDispatcher>> takePicture;

    typedef struct {
        ICamera::TCameraParams cameraParams;
        LEDParams uvLEDs;
        LEDParams visibleLEDs;
    } TMeasureParams;

    ActiveMethod<cv::Mat, TMeasureParams, SolariumDevice, ActiveStarter<ActiveDispatcher>> takePictureForMeasure;
    ActiveMethod<std::pair<cv::Mat,std::pair<TLightResult, TLightResult>>, TMeasureParams, SolariumDevice, ActiveStarter<ActiveDispatcher>> takePictureAndLightSensorsForMeasure;

    void setMeasureConfig(const MeasureConfig & config);
    const MeasureConfig & getMeasureConfig() const;

private:
    ILightSensor::UPtr _lightSensors[2];
    ITemperatureSensor::UPtr _temperatureSensors[2];
    IBattery::UPtr _battery;
    ICamera::UPtr _camera;
    ILedDriver::UPtr _uvLeds;
    ILedDriver::UPtr _visibleLeds;
    std::shared_ptr<I2CMux> _i2cMux;

    std::unique_ptr<MeasureConfig> _currentMeasureConfig;


    SolariumDevice();

    void buildWithDummyPeripherals();
    void buildWithRealPeripherals();

    Poco::Void setLEDsImpl(const LEDParams & params);
    TSensorsInfos getSensorsInfoImpl(const Poco::Void &);
    TSensorsDiagnostics getSensorsDiagnosticsImpl(const Poco::Void &);

    std::vector<uint8_t> takePictureImpl(const ICamera::TCameraParams & params);

    cv::Mat takePictureForMeasureImpl(const TMeasureParams & params);
    std::pair<cv::Mat,std::pair<TLightResult, TLightResult>> takePictureAndLightSensorsForMeasureImpl(const TMeasureParams & params);

    std::pair<TLightResult, TLightResult> getLightSensorsValues(int delayMS = 0);
    std::vector<TLightSensorsTimestamp> runLightSensorsDiagnosticsImpl(const std::pair<int, int> & uvLedIntensityAndDurationInSeconds);

    void turnAllLedsOff();
    Poco::Void setAllLEDsOffImpl(const Poco::Void &);
};


#endif //SOLARIUM_SOLARIUMDEVICE_H
