//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "DummyLightSensor.h"
#include <cstdlib>

ILightSensor::TResult DummyLightSensor::values() {
    return {
        .uv = rand() % 200,
        .visible = rand() % 200
    };
}
