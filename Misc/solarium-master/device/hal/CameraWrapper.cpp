//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/07/07.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "CameraWrapper.h"
#include "configure.h"
#ifdef USE_UEYE_CAMERA
#include "../ueye/UEyeCamera.h"
#elif defined(USE_V4L2_CAMERA)
#include "../v4l2camera/V4L2Camera.h"
#else
#include "../dummyDevice/DummyCamera.h"
#endif
#include <iomanip>
#include <Poco/Clock.h>
#include "SolariumApp.h"

using namespace std::chrono_literals;

CameraWrapper::CameraWrapper(const std::string & deviceNode)
{
#ifdef USE_UEYE_CAMERA
    _camera = std::make_unique<UEyeCamera>();
#elif defined(USE_V4L2_CAMERA)
    _camera = std::make_unique<V4L2Camera>(deviceNode);
#else
    _camera = std::make_unique<DummyCamera>();
#endif
}

CameraWrapper::~CameraWrapper()
{
#ifdef USE_UEYE_CAMERA
    _camera->uninitialize();
#endif
}

void CameraWrapper::initialize()
{
    try {
        _camera->initialize();
        setOnlineStatus();
    }
    catch (const std::exception & ex)
    {
        LOGGER.error("Error initializing camera : %s", std::string(ex.what()));
        setErrorStatus(ex.what());
    }
}

void CameraWrapper::setupCapture(const ICamera::TCameraParams &inputParams, const ICamera::TRectangle &roi)
{
    if (isOnline()) _camera->setupCapture(inputParams, roi);
}

cv::Mat CameraWrapper::takePictureAsMat(FLedLightning ledLightning)
{
    if (isOnline())
        return _camera->takePictureAsMat(ledLightning);
    else
        return cv::Mat();
}

std::vector<uint8_t> CameraWrapper::takePicture(const ICamera::TCameraParams &params, const TRectangle &roi)
{
    if (isOnline())
    {
        return _camera->takePicture(params, roi);
    }
    else
        return std::vector<uint8_t>();
}

ICamera::TCameraParams CameraWrapper::getCurrentParams() const
{
    return _camera->getCurrentParams();
}

void CameraWrapper::uninitialize()
{
    _camera->uninitialize();
}
