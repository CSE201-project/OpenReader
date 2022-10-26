//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LMX3LED_H
#define SOLARIUM_LMX3LED_H

#include "../ILedDriver.h"
#include "inc/pca9624_drv.h"

class Lmx3LED : public ILedDriver {
public:

    enum LEDType {
        LED_UV,
        LED_Visible
    };

    explicit Lmx3LED(LEDType ledType, bool useMaxBrightnessLimit = true);

    virtual ~Lmx3LED();

    void enable(uint8_t intensityGroup1, uint8_t intensityGroup2) override;

    void setIntensity(uint8_t group1, uint8_t group2) override;

    std::tuple<uint8_t, uint8_t> getIntensity() override;

    void disable() override;

private:
    std::unique_ptr<Led_driver> _driver;
    LEDType _type;


};


#endif //SOLARIUM_LMX3LED_H
