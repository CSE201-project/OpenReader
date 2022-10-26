//
// Created by Mario Valdivia on 2020/06/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_ILEDDRIVER_H
#define SOLARIUM_ILEDDRIVER_H

#include <memory>

#define LED_MAX_INTENSITY   255

class ILedDriver {
public:

    void enable() { enable(0, 0); }
    virtual void enable(uint8_t intensityGroup1, uint8_t intensityGroup2) = 0;
    virtual void disable() = 0;

    virtual void setIntensity(uint8_t group1, uint8_t group2) = 0;
    virtual std::tuple<uint8_t, uint8_t> getIntensity() = 0;

    typedef std::unique_ptr<ILedDriver> UPtr;
};

#endif //SOLARIUM_ILEDDRIVER_H
