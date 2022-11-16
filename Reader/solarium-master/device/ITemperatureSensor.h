//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_ITEMPERATURESENSOR_H
#define SOLARIUM_ITEMPERATURESENSOR_H

#include <memory>

class ITemperatureSensor {
public:

    typedef struct {
        float temperature;
        float humidity;
    } TResult;

    virtual TResult values() = 0;

    typedef std::unique_ptr<ITemperatureSensor> UPtr;
};

#endif //SOLARIUM_ITEMPERATURESENSOR_H
