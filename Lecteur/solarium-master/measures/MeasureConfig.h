//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/12.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_MEASURECONFIG_H
#define SOLARIUM_MEASURECONFIG_H

#include <string>
#include <cstdint>
#include "device/ICamera.h"
#include "models/LEDParams.h"
#include "models/LensFocal.h"
#include <common/imageProcessing/ImageAnalyzer.h>

class MeasureConfig {
public:
    explicit MeasureConfig(const std::string &measureType);
    explicit MeasureConfig(const std::map<std::string, std::string> &params);

    const ICamera::TCameraParams & getCameraParams() const;
    const LEDParams & getLEDParams(LEDParams::LEDType type) const;

    const std::string & getStripType() const;

    ColorChannel getColorChannel() const;
    uint8_t getColorDepth() const;
    bool getUsingCassette() const;
    double getPadding() const;
    std::string getCassetteName() const;

    double getDistanceCL2TL() const;
    double getTLLength() const;
    double getPixelsToMm() const;
    double getMaskLength() const;

    std::tuple<double, double, double> getProxNoiseROISizes() const;
    std::string getAlgoInfos(const std::string & path) const;
    bool isCroppingParam() const;
    bool isImageInAmbientLight() const;

    LensFocal getLensFocal() const;


    std::string ToJSON() const;

    static MeasureConfig LoadDefaults(const std::string & type);

private:
    std::string _stripType;
    ICamera::TCameraParams _cameraParams;
    LEDParams _ledsParams[2];

    ICamera::TCameraParams getCameraParamsForType(const std::string & type = "default_params") const;
};


#endif //SOLARIUM_MEASURECONFIG_H
