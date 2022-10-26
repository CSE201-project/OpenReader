//
// Created by Mario Valdivia on 2020/06/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DUMMYLED_H
#define SOLARIUM_DUMMYLED_H

#include "../ILedDriver.h"

class DummyLed : public ILedDriver {
public:
    void enable(uint8_t intensityGroup1, uint8_t intensityGroup2) override;

    void disable() override;

    void setIntensity(uint8_t group1, uint8_t group2) override;

    std::tuple<uint8_t, uint8_t> getIntensity() override;

};


#endif //SOLARIUM_DUMMYLED_H
