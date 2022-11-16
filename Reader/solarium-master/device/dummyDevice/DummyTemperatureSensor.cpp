//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "DummyTemperatureSensor.h"
#include <cstdlib>

ITemperatureSensor::TResult DummyTemperatureSensor::values() {
    return  {
        .temperature = static_cast<float>(rand() % 90),
        .humidity = static_cast<float>(rand() % 99)
    };
}
