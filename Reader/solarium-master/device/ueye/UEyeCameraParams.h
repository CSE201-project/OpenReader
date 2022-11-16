//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/08/26.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_UEYECAMERAPARAMS_H
#define SOLARIUM_UEYECAMERAPARAMS_H

#include <optional>
#include <cstdint>
#include "../ICamera.h"

class UEyeCameraParams {
private:
    const ICamera::TCameraParams &_params;

public:
    explicit UEyeCameraParams(const ICamera::TCameraParams &params);

    void displayCameraParams() const;

    int GetColorDepth() const;

    int GetBitsPerPixel() const;

    int GetColorConvert() const;

    uint32_t GetAutoWhiteBalance() const;

    std::optional<double> GetExposure() const;

    typedef struct {
        int master;
        int red;
        int green;
        int blue;
    } TGainParam;

    std::optional<TGainParam> GetGain() const;

    std::optional<int> GetBlackLevelMode() const;
    std::optional<int> GetBlackLevelOffset() const;


    template<class T>
    [[maybe_unused]] std::optional<T> GetOptionalParam(const std::string & paramName) const;


};

#endif //SOLARIUM_UEYECAMERAPARAMS_H
