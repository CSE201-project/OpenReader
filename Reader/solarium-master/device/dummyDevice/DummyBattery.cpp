//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "DummyBattery.h"
#include <cstdlib>

IBattery::TReading DummyBattery::values() {
    return {
        .charging = true,
        .charge = (uint8_t)(rand() % 100 + 1)
    };
}
