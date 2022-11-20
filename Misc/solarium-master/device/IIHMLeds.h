//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/29.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_IIHMLEDS_H
#define SOLARIUM_IIHMLEDS_H

class IIHMLeds {
public:
    virtual void setColor(uint8_t rgb[3]) = 0;
    virtual void setMode() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
};

#endif //SOLARIUM_IIHMLEDS_H
