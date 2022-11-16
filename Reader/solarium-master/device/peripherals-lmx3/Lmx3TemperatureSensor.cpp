//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "Lmx3TemperatureSensor.h"
#include "inc/thm_drv.h"
#include "imx8_defs.h"
#include <thread>
using namespace std::chrono_literals;

Lmx3TemperatureSensor::Lmx3TemperatureSensor(bool firstBoard)
    : _sensor(new ThmSensor())
{
    auto ret = _sensor->Init(firstBoard ? TEMP_SENSOR_I2C_BUS : TEMP_SENSOR2_I2C_BUS, HDC1080_I2C_ADDRESS);
    if (ret != EXIT_SUCCESS)
        setErrorStatus("Could not open Temperature sensor");
    else
    {
        try
        {
            _sensor->ConfigAcquisition();
            setOnlineStatus();
        }
        catch (const std::exception & ex)
        {
            setErrorStatus(ex.what());
        }
    }
}

ITemperatureSensor::TResult Lmx3TemperatureSensor::values()
{
    if (isOnline())
    {
        try
        {
            _sensor->StartAcquisition();
            std::this_thread::sleep_for(50ms);
            _sensor->ReadResults();
            TResult result;
            result.temperature = _sensor->GetTemperature();
            result.humidity = _sensor->GetHumidity();
            return result;
        }
        catch (const std::exception & ex)
        {
            setErrorStatus(ex.what());
        }
    }

    return {
            .temperature = -1,
            .humidity = -1
    };
}

