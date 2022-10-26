//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/08/03.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_UEYECAMERA_H
#define SOLARIUM_UEYECAMERA_H

#include "../ICamera.h"
#include <ueye.h>
#include "UEyeCameraParams.h"

class UEyeCamera : public ICamera {
public:
    UEyeCamera();
    virtual ~UEyeCamera();

    void initialize() override;
    void uninitialize() override;

    void setupCapture(const TCameraParams & params, const TRectangle &roi) override;
    cv::Mat takePictureAsMat(FLedLightning ledLightning) override;

    std::vector<uint8_t> takePicture(const TCameraParams &params, const TRectangle &roi) override;

    TCameraParams getCurrentParams() const;

private:
    HCAM            m_hCam = 0;
    CAMINFO         m_CameraInfo = {};
    SENSORINFO      m_SensorInfo = {};
    TRectangle      m_ROI;
    INT		        m_lMemoryId = 0;	// grabber memory - buffer ID
    char*	        m_pcImageMemory = nullptr;// grabber memory - pointer to buffer
    int             m_nColorMode = 0;
    int             m_nBitsPerPixel = 0;
    int             m_ledWarmUpDuration = DEFAULT_LED_WARMUP_DURATION * 1000;

    void initCamera();
    void setupCapture(const UEyeCameraParams & params);
    void setupCaptureInternal(ICamera::TRectangle roi = TRectangle());
    void cleanUpBuffers();

    void showExposureCapabilities() const;
    double getExposureCurrentValueInMS() const;
    void setExposure(double timeInMS);

    void disableAutoParams() const;
};


#endif //SOLARIUM_UEYECAMERA_H
