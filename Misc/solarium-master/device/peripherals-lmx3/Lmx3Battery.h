//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/27.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LMX3BATTERY_H
#define SOLARIUM_LMX3BATTERY_H

#include <memory>
#include "../IBattery.h"
#include "../IPeripheralStatus.h"
#include "inc/psy_drv.h"

class Lmx3Battery : public IBattery, public IPeripheralStatus {
public:
    Lmx3Battery();

    void init() override;

    TReading values() override;

private:
    std::unique_ptr<PsyControler> _battery;
};


#endif //SOLARIUM_LMX3BATTERY_H
