//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DUMMYTEMPERATURESENSOR_H
#define SOLARIUM_DUMMYTEMPERATURESENSOR_H

#include "../ITemperatureSensor.h"

class DummyTemperatureSensor : public ITemperatureSensor {
public:
    TResult values() override;

};


#endif //SOLARIUM_DUMMYTEMPERATURESENSOR_H
