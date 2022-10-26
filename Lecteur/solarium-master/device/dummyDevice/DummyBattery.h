//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DUMMYBATTERY_H
#define SOLARIUM_DUMMYBATTERY_H

#include "../IBattery.h"
#include <cstdlib>

class DummyBattery : public IBattery {
public:
    TReading values() override;

};


#endif //SOLARIUM_DUMMYBATTERY_H
