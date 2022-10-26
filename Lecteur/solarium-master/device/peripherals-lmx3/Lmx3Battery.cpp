//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/27.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "Lmx3Battery.h"
#include "inc/psy_drv.h"
#include "imx8_defs.h"
#include "SolariumApp.h"


Lmx3Battery::Lmx3Battery()
//    : _battery(new PsyControler())
{
}

IBattery::TReading Lmx3Battery::values()
{
    TReading results = {
            .charging = false,
            .charge = 0
    };
    if (!isOnline()) return results;
/*
    results.charging = _battery->USBConnectionState() == PsyControler::PWR_PLUG_CONNECTED;

    uint16_t remainingCapacity = 0, fullCapacity = 0, remainingChargePercent = 0;
    _battery->BatteryRemainingCapacity(&remainingCapacity);
    _battery->BatteryFullChargeCapacity(&fullCapacity);
    _battery->BatteryRelativeStateOfCharge(&remainingChargePercent);

    LOGGER.information("==> remainingCapacity %?u, fullCapacity %?u, remainingChargePercent %?u",remainingCapacity, fullCapacity, remainingChargePercent);

    results.charge = remainingChargePercent & 0xFF;
*/


    return results;
}

void Lmx3Battery::init()
{
    LOGGER.information("==> Init SYS_POWER_GPIO, BATTERY_I2C_BUS");

//    if (_battery->Init(SYS_POWER_GPIO, BATTERY_I2C_BUS) != EXIT_SUCCESS)
//        setErrorStatus("Failed to init Battery controller");
//    else
        setOnlineStatus();
}


