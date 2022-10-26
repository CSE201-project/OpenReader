//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_ILIGHTSENSOR_H
#define SOLARIUM_ILIGHTSENSOR_H

#include <memory>

class ILightSensor {
public:

    typedef struct {
        int uv;
        int visible;
    } TResult;

    virtual TResult values() = 0;

    typedef std::unique_ptr<ILightSensor> UPtr;
};

#endif //SOLARIUM_ILIGHTSENSOR_H
