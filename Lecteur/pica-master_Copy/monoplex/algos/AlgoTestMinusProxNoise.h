//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef PICA_ALGOTESTMINUSPROXNOISE_H
#define PICA_ALGOTESTMINUSPROXNOISE_H

#include "IImageProcessor.h"
#include <cstdint>

class AlgoTestMinusProxNoise : public IImageProcessor {
public:
    uint32_t TestLineLength;
    uint32_t NoiseBeforeLength;
    uint32_t NoiseAfterLength;

    std::tuple<double, double> processImageProfile(const TImageProfile & profile,
                                                   std::vector<uint32_t> testLines) override;
    void setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength) override;
    double compute(ProcessorConfig config, const ProfileData& profileData, const ProxNoiseROISizeMm& proxNoiseROIMm) override;
    double computeFirstPrediction(double result, const std::string& coefficients) override;
    double computeSecondPrediction(double result, const std::string& coefficients) override;
private:
    std::tuple<double, double, double> computeNoisesAndLine(const TImageProfile & profile, uint32_t lineIndex);
};


#endif //PICA_ALGOTESTMINUSPROXNOISE_H
