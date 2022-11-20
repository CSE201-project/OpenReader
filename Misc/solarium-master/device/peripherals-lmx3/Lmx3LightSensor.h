//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LMX3LIGHTSENSOR_H
#define SOLARIUM_LMX3LIGHTSENSOR_H

#include "../ILightSensor.h"
#include "../IPeripheralStatus.h"
#include "inc/uvs_drv.h"

class Lmx3LightSensor : public ILightSensor, public IPeripheralStatus {
public:

    Lmx3LightSensor(bool firstBoard = true);

    TResult values() override;

private:
    std::unique_ptr<Uvs> _sensor;
    std::unique_ptr<Vis> _sensorVisible;
};


#endif //SOLARIUM_LMX3LIGHTSENSOR_H
