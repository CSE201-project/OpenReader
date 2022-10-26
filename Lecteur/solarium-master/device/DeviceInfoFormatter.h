//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DEVICEINFOFORMATTER_H
#define SOLARIUM_DEVICEINFOFORMATTER_H

#include "SolariumDevice.h"

class DeviceInfoFormatter {
public:
    static std::string toJSON(const std::string & deviceName, const SolariumDevice::TSensorsInfos & deviceSensors, const std::string & appVersion);
    static std::string toJSON(const std::string & deviceName, const SolariumDevice::TSensorsDiagnostics & deviceSensors, const std::string & appVersion);
    static std::string toJSON(const SolariumDevice::TSpaceInfos & info);
};


#endif //SOLARIUM_DEVICEINFOFORMATTER_H
