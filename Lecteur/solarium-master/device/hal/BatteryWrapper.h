//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/08/21.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_BATTERYWRAPPER_H
#define SOLARIUM_BATTERYWRAPPER_H

#include "../IBattery.h"
#include "../IPeripheralStatus.h"
#include "../peripherals/charger.h"

class BatteryWrapper : public IBattery, public Charger, public IPeripheralStatus {
public:
    BatteryWrapper(const std::shared_ptr<I2CMux> &muxPtr);

    TReading values() override;

};


#endif //SOLARIUM_BATTERYWRAPPER_H
