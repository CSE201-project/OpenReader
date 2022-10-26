//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LMX3TEMPERATURESENSOR_H
#define SOLARIUM_LMX3TEMPERATURESENSOR_H

#include "../ITemperatureSensor.h"
#include "../IPeripheralStatus.h"
#include "inc/thm_drv.h"

class Lmx3TemperatureSensor : public ITemperatureSensor, public IPeripheralStatus {
public:

    Lmx3TemperatureSensor(bool firstBoard = true);

    TResult values() override;

private:
    std::unique_ptr<ThmSensor> _sensor;
};


#endif //SOLARIUM_LMX3TEMPERATURESENSOR_H
