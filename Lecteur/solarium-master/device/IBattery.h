//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_IBATTERY_H
#define SOLARIUM_IBATTERY_H

#include <tuple>
#include <memory>

class IBattery {
public:

    typedef struct {
        bool charging;
        uint8_t charge;
    } TReading;

    virtual void init() {}
    virtual TReading values() = 0;

    typedef std::unique_ptr<IBattery> UPtr;
};

#endif //SOLARIUM_IBATTERY_H
