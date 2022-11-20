//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DUMMYCAMERA_H
#define SOLARIUM_DUMMYCAMERA_H

#include "../ICamera.h"

class DummyCamera : public ICamera {
public:

    void initialize() override;

    cv::Mat takePictureAsMat(FLedLightning ledLightning) override;

    std::vector<uint8_t> takePicture(const TCameraParams &params, const TRectangle &roi) override;


};


#endif //SOLARIUM_DUMMYCAMERA_H
