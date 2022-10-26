//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "Lmx3LightSensor.h"
#include "inc/uvs_drv.h"

#include "imx8_defs.h"
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

Lmx3LightSensor::Lmx3LightSensor(bool firstBoard)
    : _sensor(new Uvs())
    , _sensorVisible(new Vis())
{
    auto ret = _sensor->Init(firstBoard ? UVS_SENSOR_I2C_ID : UVS_SENSOR2_I2C_ID, UVS_SENSOR_ADDR);
    if (ret != EXIT_SUCCESS)
    {
        setErrorStatus("Failed to open UV Sensor");
    }
    _sensor->SetResolution(Uvs::RES_UV_100MS);
    _sensor->SetRange(Uvs::RANGE_UV_32x);

    auto ret2 = _sensorVisible->Init(firstBoard ? VISIBLE_SENSOR_I2C_ID : VISIBLE_SENSOR2_I2C_ID, VISIBLE_SENSOR_I2C_ADDR);
    if (ret2 != EXIT_SUCCESS)
    {
        if (ret != EXIT_SUCCESS)
            setErrorStatus("Failed to open UV & Ambient Light Sensor");
        else
            setErrorStatus("Failed to open Ambient Light Sensor");
    }

    if (ret == EXIT_SUCCESS && ret2 == EXIT_SUCCESS)
        setOnlineStatus();
}

ILightSensor::TResult Lmx3LightSensor::values()
{
    if (isOnline())
    {
        try
        {
            _sensor->ConfigSensor();
            _sensorVisible->ConfigSensor();

            std::this_thread::sleep_for(100ms);

            TResult result;
            result.uv = _sensor->ReadUVB();
            result.visible = _sensorVisible->ReadAls();
            return result;
        }
        catch (const std::exception & ex) {
            setErrorStatus(ex.what());
        }
    }

    return {
            .uv = -255,
            .visible = -255
    };
}

