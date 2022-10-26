//
// Created by KarimSadki on 03/02/2021.
//

#ifndef PICA_ALGOPOLY2_H
#define PICA_ALGOPOLY2_H

#include "AlgoProxNoise.h"

class AlgoPoly2 : public IImageProcessor {
public:
    std::tuple<TImageProfile, double> transformProfileForProcessing(const TImageProfile& profile, std::vector<uint32_t> testAreas) override;
    std::tuple<double, double> processImageProfile(const TImageProfile& profile,
                                                   std::vector<uint32_t> testAreas) override;
    void setROILengths(uint32_t newTestLineLength, uint32_t newNoiseBeforeLength, uint32_t newNoiseAfterLength) override;
private:
    AlgoProxNoise _algoProxNoise;
};

#endif //PICA_ALGOPOLY2_H
