//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/08/21.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include <climits>
#include "BatteryWrapper.h"
#include "../../tools/Exception.h"
#include "../../tools/DebugInfo.h"

BatteryWrapper::BatteryWrapper(const std::shared_ptr<I2CMux> &muxPtr)
    : Charger(muxPtr)
{
    auto ret = Open(m_mux->GetFd());
    if (ret < 0)
    {
        setErrorStatus("Failed to open the battery sensor");
    }
    else
        setOnlineStatus();
}

IBattery::TReading BatteryWrapper::values()
{
    uint16_t socPercent;
    uint16_t voltage_mV;
    ChargerStatus status;

    if (auto ret = MeasureSync(socPercent, voltage_mV, status))
    {
        THROWEX("Error reading battery sensor, ret:" << ret);
    }

    TReading reading;
    reading.charge = (uint8_t)(socPercent & UCHAR_MAX);

    switch (status.chargerStatus)
    {
        case ChargerStatus::E_CHG_STS_CHG_FROM_IN:
        case ChargerStatus::E_CHG_STS_CHG_FROM_USB:
            reading.charging = true;
            break;
        default:
            reading.charging = false;
            break;
    }

    return reading;
}

