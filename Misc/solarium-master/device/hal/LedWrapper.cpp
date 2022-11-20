//
// Created by Mario Valdivia on 2020/06/24.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "LedWrapper.h"
#include "../peripherals/i2c_mux.h"
#include "../../tools/Exception.h"
#include "SolariumApp.h"


LedWrapper::LedWrapper(LedDriver::Type type, const std::shared_ptr<I2CMux> &muxPtr) : LedDriver(type, muxPtr) {
    auto ret = Open(m_mux->GetFd());
    if(ret < 0)
    {
        std::ostringstream  oss;
        oss << "Failed to open LedDriver " << __xstr__(type) << " " << ret;
        setErrorStatus(oss.str());
        poco_debug(LOGGER, oss.str());
    }
    else
        setOnlineStatus();
}

LedWrapper::~LedWrapper() {

}

void LedWrapper::enable(uint8_t intensityGroup1, uint8_t intensityGroup2) {
    if (isOnline())
    {
        auto ret = LedDriver::Enable(intensityGroup1, intensityGroup2);
        if (ret != 0)
            THROWEX("Could not set intensity for Led, reason " << ret);
    }
    else THROWEX("Could not enable Led, peripheral is in invalid state");
}

void LedWrapper::disable() {
    if (isOnline())
    {
        auto ret = LedDriver::Disable();
        if (ret != 0)
            THROWEX("Something went horribly wrong disabling the LED : " << ret);
    }
    else THROWEX("Could not disable Led, peripheral is in invalid state");
}

void LedWrapper::setIntensity(uint8_t group1, uint8_t group2) {
    if (isOnline())
    {
        auto ret = LedDriver::SetIntensity(group1, group2);
        if (ret != 0)
            THROWEX("Could not set intensity for Led, reason " << ret);
    }
    else THROWEX("Peripheral is in invalid state");
}

std::tuple<uint8_t, uint8_t> LedWrapper::getIntensity() {
    THROWEX("Not implemented");
    return std::tuple<uint8_t, uint8_t>();
}

LedWrapperInvertedIntensity::LedWrapperInvertedIntensity(LedDriver::Type type, const std::shared_ptr<I2CMux> &muxPtr)
        : LedWrapper(type, muxPtr) {}

LedWrapperInvertedIntensity::~LedWrapperInvertedIntensity() {
}

void LedWrapperInvertedIntensity::setIntensity(uint8_t group1, uint8_t group2) {
    LedWrapper::setIntensity(255 - group1, 255 - group2);
}

void LedWrapperInvertedIntensity::enable(uint8_t intensityGroup1, uint8_t intensityGroup2) {
    LedWrapper::enable(255 - intensityGroup1, 255 - intensityGroup2);
}
