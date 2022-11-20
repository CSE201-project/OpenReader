//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DUMMYLIGHTSENSOR_H
#define SOLARIUM_DUMMYLIGHTSENSOR_H

#include "../ILightSensor.h"

class DummyLightSensor : public ILightSensor {
public:

    TResult values() override;

};


#endif //SOLARIUM_DUMMYLIGHTSENSOR_H
