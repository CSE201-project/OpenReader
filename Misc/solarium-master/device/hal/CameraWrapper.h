//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/07/07.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_CAMERAWRAPPER_H
#define SOLARIUM_CAMERAWRAPPER_H

#include "../ICamera.h"
#include "../IPeripheralStatus.h"
#include <mutex>

class UEyeCamera;

class CameraWrapper : public ICamera, public IPeripheralStatus {
public:
    CameraWrapper(const std::string & deviceNode);

    virtual ~CameraWrapper();

    void initialize() override;

    void uninitialize() override;


    void setupCapture(const TCameraParams &inputParams, const TRectangle &roi) override;
    cv::Mat takePictureAsMat(FLedLightning ledLightning) override;

    std::vector<uint8_t> takePicture(const TCameraParams &params, const TRectangle &roi) override;

    TCameraParams getCurrentParams() const override;

private:
#ifdef USE_UEYE_CAMERA
    std::unique_ptr<UEyeCamera> _camera;
#else
    std::unique_ptr<ICamera> _camera;
#endif
};


#endif //SOLARIUM_CAMERAWRAPPER_H
