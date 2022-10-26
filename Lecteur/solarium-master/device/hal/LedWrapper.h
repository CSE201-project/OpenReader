//
// Created by Mario Valdivia on 2020/06/24.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LEDWRAPPER_H
#define SOLARIUM_LEDWRAPPER_H


#include "../ILedDriver.h"
#include "../IPeripheralStatus.h"
#include "../peripherals/led_driver.h"

class LedWrapper : public ILedDriver, public LedDriver, public IPeripheralStatus {
public:
    LedWrapper(Type type, const std::shared_ptr<I2CMux> &muxPtr);

    virtual ~LedWrapper();

    void enable(uint8_t intensityGroup1, uint8_t intensityGroup2) override;

    void disable() override;

    void setIntensity(uint8_t group1, uint8_t group2) override;

    std::tuple<uint8_t, uint8_t> getIntensity() override;
};

class LedWrapperInvertedIntensity : public LedWrapper {
public:
    LedWrapperInvertedIntensity(Type type, const std::shared_ptr<I2CMux> &muxPtr);

    virtual ~LedWrapperInvertedIntensity();

    void enable(uint8_t intensityGroup1, uint8_t intensityGroup2) override;

    void setIntensity(uint8_t group1, uint8_t group2) override;
};

#endif //SOLARIUM_LEDWRAPPER_H
