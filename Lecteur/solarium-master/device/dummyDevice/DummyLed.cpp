//
// Created by Mario Valdivia on 2020/06/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "DummyLed.h"
#include "DummyOutput.h"
#include <cstdlib>

void DummyLed::enable(uint8_t intensityGroup1, uint8_t intensityGroup2) {
    //info("DummyLed::enable(group1 " << unsigned(intensityGroup1) << ", group2 " << unsigned(intensityGroup2) << ")");
}

void DummyLed::disable() {
    //info("DummyLed::disable()");
}

void DummyLed::setIntensity(uint8_t group1, uint8_t group2) {
    //info("DummyLed::setIntensity(group1 " << unsigned(group1) << ", group2 " << unsigned(group2) << ")");
}

std::tuple<uint8_t, uint8_t> DummyLed::getIntensity() {
    //info("DummyLed::getIntensity()");
    return std::tuple<uint8_t, uint8_t>(10, 20);
}
