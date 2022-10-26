//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/20.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LEDPARAMS_H
#define SOLARIUM_LEDPARAMS_H

#include <optional>

struct LEDParams {
    enum LEDType {
        UV,
        Visible
    };

    LEDType ledType;
    std::optional<bool> enabled = {};
    uint8_t intensity;

    static LEDParams parse(const std::string & params, LEDType type);
    static LEDParams parse(std::istream & params, LEDType type);
    static LEDParams parse(std::istream & params, const std::string & type);
};


#endif //SOLARIUM_LEDPARAMS_H
