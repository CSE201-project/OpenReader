//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/27.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_LENSFOCAL_H
#define SOLARIUM_LENSFOCAL_H

#include <set>
#include <string>

class LensFocal {
public:
    enum DeviceSeries {
        Series2,
        Series3
    };

    static const std::set<std::string> AvailableFocals() {
        return {
            "F041",
            "F060",
            "F080"
        };
    }

    explicit LensFocal();
    explicit LensFocal(const std::string &focal, DeviceSeries series);
    int type() const;

private:
    int _type;
};


#endif //SOLARIUM_LENSFOCAL_H
