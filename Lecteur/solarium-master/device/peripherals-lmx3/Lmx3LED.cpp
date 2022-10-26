//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "Lmx3LED.h"
#include "inc/pca9624_drv.h"
#include "imx8_defs.h"
#include <exception>
#include "SolariumApp.h"

Lmx3LED::Lmx3LED(Lmx3LED::LEDType ledType, bool useMaxBrightnessLimit)
    : _driver(new Led_driver())
{
    _type = ledType;

    if (!useMaxBrightnessLimit)
    {
        _driver->ledMaximumBrightness = 255;
        LOGGER.information("LED Driver : removed Maximum Brightness Limit");
    }

    int ret;
    if (_type == LED_UV)
        ret = _driver->Init(UV_LED_GPIO, UV_LED_I2C_BUS, UV_LED_I2C_ADDR);
    else
        ret = _driver->Init(VISIBLE_LED_GPIO, VISIBLE_LED_I2C_BUS, VISIBLE_LED_I2C_ADDR);

    if (ret != 0)
        throw std::runtime_error("LED Driver init failed !");

    _driver->Enable();
    _driver->Off();

}

Lmx3LED::~Lmx3LED()
{
    _driver->Disable();
}

void Lmx3LED::enable(uint8_t intensityGroup1, uint8_t intensityGroup2)
{
    setIntensity(intensityGroup1, intensityGroup2);
}

void Lmx3LED::setIntensity(uint8_t group1, uint8_t group2)
{
    //LOGGER.debug("Lmx3LED %s setIntensity %u", std::string((_type == LED_UV) ? "UV" : "Visible"), (uint32_t)group1);
    for (auto i = (int)Led_driver::LED_CHAN0; i < (int)Led_driver::LED_CHAN_COUNT; i++)
        _driver->SetState((Led_driver::Led_channel)i, Led_driver::LED_IND_PWM);

    for (auto i = (int)Led_driver::LED_CHAN0; i < (int)Led_driver::LED_CHAN_COUNT; i++)
        _driver->SetIntensity((Led_driver::Led_channel)i, group1);

    _driver->Refresh();
}

void Lmx3LED::disable()
{
    _driver->Off();
}

std::tuple<uint8_t, uint8_t> Lmx3LED::getIntensity()
{
    return std::tuple<uint8_t, uint8_t>();
}
