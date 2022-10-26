//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef PICA_ALGOPROXNOISE_H
#define PICA_ALGOPROXNOISE_H

#include "IImageProcessor.h"
#include <cstdint>

class AlgoProxNoise : public IImageProcessor {
public:
    uint32_t TestLineLength;
    uint32_t NoiseBeforeLength;
    uint32_t NoiseAfterLength;

    constexpr static double DEFAULT_TL_LENGTH_MM = 1.3;
    constexpr static double DEFAULT_NOISE_LENGTH_MM = 1.;

    std::tuple<double, double> processImageProfile(const TImageProfile & profile,
                                                   std::vector<uint32_t> testLines) override;
    void setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength) override;
    double compute(ProcessorConfig config, const ProfileData& profileData, const ProxNoiseROISizeMm& proxNoiseROIMm) override;
private:
    std::tuple<double, double, double> computeNoisesAndLine(const TImageProfile & profile, uint32_t lineIndex);
};


#endif //PICA_ALGOPROXNOISE_H
