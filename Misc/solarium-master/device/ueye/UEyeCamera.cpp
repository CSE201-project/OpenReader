//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/08/03.
// Copyright (c) 2020 LumediX. All rights reserved.
//
#include "UEyeCamera.h"
#include "UEyeCameraParams.h"

#include <cstring>
#include <sstream>
#include <iostream>
#include <exception>
#include <string>
#include <tuple>
#include <thread>

#include <ueye_readable_errors.h>
#include <opencv2/opencv.hpp>
#include <Poco/Timestamp.h>

#include "SolariumApp.h"
#include "tools/Converter.h"

#define PRINT(msg) std::cout << msg << std::endl
#define LOG_INFO(msg) std::cout << "[INFO] [UEyeCamera::" << __FUNCTION__ <<" l." << __LINE__ << "] : " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] [UEyeCamera::" << __FUNCTION__ <<" l." << __LINE__ << "] : " << msg << std::endl


#define CHECKRET(ret) do { if (ret != IS_SUCCESS) {                     \
    std::ostringstream oss;                                             \
    oss << "[ERROR][UEyeCamera::" << __FUNCTION__ <<" l." << __LINE__ << "] : " << ret << " " << uEyeReadableErrors.at(ret); \
    PRINT(oss.str());                                                                    \
    throw std::runtime_error(oss.str());                                \
} } while(0)

// #####################################################################################################################
bool isCameraReachable()
{
    bool cameraReachable = false;
    // At least one camera must be available
    INT nNumCam;
    if (is_GetNumberOfCameras(&nNumCam) == IS_SUCCESS)
    {
        if (nNumCam >= 1)
        {
            // Create new list with suitable size
            UEYE_CAMERA_LIST *pucl;
            pucl = (UEYE_CAMERA_LIST *) new BYTE[sizeof(DWORD) + nNumCam * sizeof(UEYE_CAMERA_INFO)];
            pucl->dwCount = nNumCam;

            //Retrieve camera info
            if (is_GetCameraList(pucl) == IS_SUCCESS)
            {
                if (!pucl->uci[0].dwInUse)
                    cameraReachable = true;
                else
                    throw std::runtime_error("Camera already in use");
            }
            delete[] pucl;
        }
    }
    return cameraReachable;
}

void ensureCameraIsReachable()
{
    bool isDaemonInitialized = false;
    for (int i = 0; i < 3; i++)
    {
        isDaemonInitialized = isCameraReachable();
        if (!isDaemonInitialized)
        {
            system("/etc/init.d/ueyeusbdrc stop");
            sleep(1);
            system("/etc/init.d/ueyeusbdrc start");
        }
        else
        {
            break;
        }
    }
    if (!isDaemonInitialized)
        throw std::runtime_error("Could not initialize the camera daemon");
}

void setBlackLevelMode(HCAM hCam, int32_t autoMode)
{
    INT param = autoMode;
    INT nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_SET_MODE, (void*)&param, sizeof(param));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not set Camera BlackLevel");
}

void setBlackLevelOffset(HCAM hCam, int32_t offset)
{
    INT param = offset;
    INT nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_SET_OFFSET, (void*)&param, sizeof(param));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not set Camera BlackLevel Offset");
}
// #####################################################################################################################

UEyeCamera::UEyeCamera()
{
    ::memset(&m_CameraInfo, 0, sizeof(m_CameraInfo));
    ::memset(&m_SensorInfo, 0, sizeof(m_SensorInfo));
    ::memset(&m_ROI, 0, sizeof(m_ROI));
}

UEyeCamera::~UEyeCamera()
{
    uninitialize();
}


void UEyeCamera::initialize()
{
    m_hCam = (HCAM) 0; // open next camera

    ensureCameraIsReachable();

    initCamera();

    int nRet = is_GetCameraInfo(m_hCam, &m_CameraInfo);
    CHECKRET(nRet);

    // retrieve original image size
    nRet = is_GetSensorInfo(m_hCam, &m_SensorInfo);
    CHECKRET(nRet);

    nRet = is_SetExternalTrigger(m_hCam, IS_SET_TRIGGER_SOFTWARE);
    CHECKRET(nRet);

    // Set this pixel clock
    UINT nPixelClock = 20;
    nRet = is_PixelClock(m_hCam, IS_PIXELCLOCK_CMD_SET,
                         (void*)&nPixelClock, sizeof(nPixelClock));
    CHECKRET(nRet);

    //showExposureCapabilities();
}

void UEyeCamera::uninitialize()
{
    if (m_hCam != 0)
    {
        cleanUpBuffers();
        // As per uEye documentation, AllocImageMem memories are release by ExitCamera
        auto ret = is_ExitCamera(m_hCam);
        if (ret != IS_SUCCESS)
            LOG_ERROR("is_ExitCamera returned " << ret);

        m_hCam = 0;
    }
}

void UEyeCamera::initCamera()
{
    INT nRet = is_InitCamera(&m_hCam, NULL);
    /************************************************************************************************/
    /*                                                                                              */
    /*  If the camera returns with "IS_STARTER_FW_UPLOAD_NEEDED", an upload of a new firmware       */
    /*  is necessary. This upload can take several seconds. We recommend to check the required      */
    /*  time with the function is_GetDuration().                                                    */
    /*                                                                                              */
    /*  In this case, the camera can only be opened if the flag "IS_ALLOW_STARTER_FW_UPLOAD"        */
    /*  is "OR"-ed to m_hCam. This flag allows an automatic upload of the firmware.                 */
    /*                                                                                              */
    /************************************************************************************************/
    if (nRet == IS_STARTER_FW_UPLOAD_NEEDED)
    {
        // Time for the firmware upload = 25 seconds by default
        INT nUploadTime = 25000;
        is_GetDuration(m_hCam, IS_STARTER_FW_UPLOAD, &nUploadTime);

        std::ostringstream stringStream;
        stringStream << "This camera requires a new firmware. The upload will take about ";
        stringStream << nUploadTime / 1000;
        stringStream << " seconds. Please wait ...";
        LOG_INFO(stringStream.str());

        // Try again to open the camera. This time we allow the automatic upload of the firmware by
        // specifying "IS_ALLOW_STARTER_FIRMWARE_UPLOAD"
        m_hCam = (HIDS) (((INT) m_hCam) | IS_ALLOW_STARTER_FW_UPLOAD);
        nRet = is_InitCamera(&m_hCam, NULL);
    }
    CHECKRET(nRet);

    nRet = is_ClearSequence(m_hCam);
    CHECKRET(nRet);
}

void UEyeCamera::setupCapture(const ICamera::TCameraParams &inputParams, const ICamera::TRectangle &roi)
{
    //poco_debug(LOGGER, "Setting up Capture");
    UEyeCameraParams params(inputParams);
    setupCapture(params);
    setupCaptureInternal(roi);
}

cv::Mat UEyeCamera::takePictureAsMat(FLedLightning ledLightning)
{
    poco_debug(LOGGER, "=> Capturing as cv::Mat");

    if (ledLightning) {
        ledLightning(true);
        std::this_thread::sleep_for(std::chrono::microseconds(m_ledWarmUpDuration));
    }

    for (int nbTries = 3; nbTries > 0; nbTries--)
    {
        auto nRet = is_FreezeVideo(m_hCam, IS_WAIT);
        switch (nRet)
        {
            case IS_SUCCESS:
                nbTries = 0;
                break;
            case IS_TRANSFER_ERROR:
                poco_error(LOGGER, "=> Capture 'Transfer' error, retrying");
                break;
            default:
                CHECKRET(nRet);
        }
    }

    if (ledLightning) ledLightning(false);

    //poco_debug(LOGGER, "Converting to cv::Mat");
    int imgType = m_nBitsPerPixel == 24 ? CV_8UC3 : CV_16UC3;
    cv::Mat img(m_ROI.height, m_ROI.width, imgType, m_pcImageMemory);
    return img;
}

std::vector<uint8_t> UEyeCamera::takePicture(const ICamera::TCameraParams &inputParams, const TRectangle &roi)
{
    setupCapture(inputParams, roi);
    auto pictureMat = takePictureAsMat(nullptr);

    UEyeCameraParams params(inputParams);

    auto colorConvert = params.GetColorConvert();

    poco_debug(LOGGER, "Converting to PNG");
    return Converter::cvMatToPng(pictureMat, colorConvert);
}

void UEyeCamera::setupCapture(const UEyeCameraParams &params)
{
    //LOG_INFO("Setting Capture Mode");

    //params.displayCameraParams();

    INT nRet;
    switch (m_SensorInfo.nColorMode)
    {
        case IS_COLORMODE_BAYER:
        {
            // for color camera models use RGB24 mode
            m_nColorMode = params.GetColorDepth();
            m_nBitsPerPixel = params.GetBitsPerPixel();
            break;
        }
        case IS_COLORMODE_CBYCRY:
            // for CBYCRY camera models use RGB32 mode
            m_nColorMode = IS_CM_CBYCRY_PACKED;
            m_nBitsPerPixel = 32;
            break;
        default:
            // for monochrome camera models use Y8 mode
            m_nColorMode = IS_CM_MONO8;
            m_nBitsPerPixel = 8;
    }

    nRet = is_SetColorMode(m_hCam, m_nColorMode);
    CHECKRET(nRet);

    disableAutoParams();

    if (auto exposure = params.GetExposure())
    {
        setExposure(exposure.value());
    }

    if (auto gain = params.GetGain())
    {
        nRet = is_SetHardwareGain(m_hCam, gain.value().master, gain.value().red, gain.value().green, gain.value().blue);
        CHECKRET(nRet);
    }

    if (auto blackLevelMode = params.GetBlackLevelMode())
    {
        setBlackLevelMode(m_hCam, blackLevelMode.value());
    }

    if (auto blackLevelOffset = params.GetBlackLevelOffset())
    {
        setBlackLevelOffset(m_hCam, blackLevelOffset.value());
    }

    // Auto White Balance
    UINT nAWBEnable = params.GetAutoWhiteBalance();
    nRet = is_AutoParameter(m_hCam, IS_AWB_CMD_SET_ENABLE, (void*)&nAWBEnable, sizeof(nAWBEnable));
    CHECKRET(nRet);

    auto ledWarmUpDuration = params.GetOptionalParam<int>(CAM_PARAM_SNAPSHOT_DELAY);
    m_ledWarmUpDuration = ledWarmUpDuration.has_value() ? ledWarmUpDuration.value() : DEFAULT_LED_WARMUP_DURATION;
}

void UEyeCamera::disableAutoParams() const
{
    //LOG_INFO("disableAutoParams()");
    INT nRet;
    // Disabling Auto Params
    double val1 = 0, val2 = 0;
    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_GAIN, &val1, &val2);
    CHECKRET(nRet);

    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_FRAMERATE, &val1, &val2);
    CHECKRET(nRet);

    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SHUTTER, &val1, &val2);
    CHECKRET(nRet);

    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &val1, &val2);
    CHECKRET(nRet);

//    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_FRAMERATE, &val1, &val2);
//    CHECKRET(nRet);

    /*nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &val1, &val2);
    CHECKRET(nRet);

    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN_SHUTTER, &val1, &val2);
    CHECKRET(nRet);

    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &val1, &val2);
    CHECKRET(nRet);

    nRet = is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE, &val1, &val2);
    CHECKRET(nRet);*/
}

void UEyeCamera::setupCaptureInternal(ICamera::TRectangle roi)
{
    if (std::memcmp(&m_ROI, &roi, sizeof(m_ROI)) != 0)
    {
        m_ROI = roi;
        m_ROI.x = (roi.x / 4) * 4;
        m_ROI.y = (roi.y / 2) * 2;
        m_ROI.width = (roi.width / 8) * 8;
        m_ROI.height = (roi.height / 2) * 2;

        IS_RECT rect;
        rect.s32X = m_ROI.x;
        rect.s32Y = m_ROI.y;
        rect.s32Width = m_ROI.width;
        rect.s32Height = m_ROI.height;

        auto nRet = is_AOI(m_hCam, IS_AOI_IMAGE_SET_AOI, (void *) &rect, sizeof(rect));
        CHECKRET(nRet);

        cleanUpBuffers();

        nRet = is_AllocImageMem(m_hCam, m_ROI.width, m_ROI.height, m_nBitsPerPixel, &m_pcImageMemory, &m_lMemoryId);
        CHECKRET(nRet);

        nRet = is_SetImageMem(m_hCam, m_pcImageMemory, m_lMemoryId);
        CHECKRET(nRet);

        nRet = is_SetDisplayMode(m_hCam, IS_SET_DM_DIB);
        CHECKRET(nRet);
    }
}

void UEyeCamera::cleanUpBuffers()
{
    // if some image memory exist already then free it
    auto nRet = is_ClearSequence(m_hCam);
    CHECKRET(nRet);

    if (m_pcImageMemory != nullptr)
    {
        auto ret = is_FreeImageMem(m_hCam, m_pcImageMemory, m_lMemoryId);
        CHECKRET(ret);
        m_pcImageMemory = nullptr;
    }
}

// ######################################################################################
double getExposureDoubleValue(HCAM hCam, UINT nCommand)
{
    double param = 0;
    INT nRet = is_Exposure(hCam, nCommand, (void*)&param, sizeof(param));
    CHECKRET(nRet);

    return param;
}

void setExposureDoubleValue(HCAM hCam, UINT nCommand, double param)
{
    INT nRet = is_Exposure(hCam, nCommand, (void*)&param, sizeof(param));
    CHECKRET(nRet);
}

std::string getExposureCapabilities(HCAM hCam)
{
    UINT nCaps = 0;
    INT nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_GET_CAPS, (void*)&nCaps, sizeof(nCaps));
    CHECKRET(nRet);

    std::ostringstream oss;
    oss << "Camera Exposure Capabilities : " << std::endl;
    if (nCaps & IS_EXPOSURE_CAP_FINE_INCREMENT)
        oss <<  "\tIS_EXPOSURE_CAP_FINE_INCREMENT";
    if (nCaps & IS_EXPOSURE_CAP_LONG_EXPOSURE)
        oss <<  "\tIS_EXPOSURE_CAP_LONG_EXPOSURE";
    if (nCaps & IS_EXPOSURE_CAP_DUAL_EXPOSURE)
        oss <<  "\tIS_EXPOSURE_CAP_DUAL_EXPOSURE";

    oss << std::endl;
    oss << "\tRANGE MIN : " << getExposureDoubleValue(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MIN) << std::endl;
    oss << "\tRANGE MAX : " << getExposureDoubleValue(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MAX) << std::endl;
    oss << "\tRANGE INC : " << getExposureDoubleValue(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_INC) << std::endl;
    oss << "\tCURRENT EXPOSURE : " << getExposureDoubleValue(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE) << " ms";
    return oss.str();
}

void UEyeCamera::showExposureCapabilities() const
{
    LOG_INFO(getExposureCapabilities(m_hCam));
}

double UEyeCamera::getExposureCurrentValueInMS() const
{
    return getExposureDoubleValue(m_hCam, IS_EXPOSURE_CMD_GET_EXPOSURE);
}

void UEyeCamera::setExposure(double timeInMS)
{
    setExposureDoubleValue(m_hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, timeInMS);
}

std::string getBlackLevelCaps(HCAM hCam)
{
    INT param;
    INT nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_CAPS, (void*)&param, sizeof(param));

    if (nRet != IS_SUCCESS)
        throw std::runtime_error("Could not get Camera BlackLevel Capabilities");

    std::ostringstream oss;
    if (param & IS_BLACKLEVEL_CAP_SET_AUTO_BLACKLEVEL)
        oss << " Can change Auto BlackLevel";

    if (param & IS_BLACKLEVEL_CAP_SET_OFFSET)
        oss << " Can change Offset BlackLevel";

    return oss.str();
}

std::string getBlackLevelDefaults(HCAM hCam)
{
    INT param;
    INT nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_MODE_DEFAULT, (void*)&param, sizeof(param));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not get Camera BlackLevel Defaults");

    std::ostringstream oss;

    oss << "Default BlackLevel Mode : " << (param == IS_AUTO_BLACKLEVEL_ON) ? "On" : "Off";

    nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_MODE, (void*)&param, sizeof(param));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not get Camera BlackLevel Mode");

    oss << std::endl << "Current BlackLevel Mode : " << (param == IS_AUTO_BLACKLEVEL_ON) ? "On" : "Off";

    nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_OFFSET_DEFAULT, (void*)&param, sizeof(param));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not get Camera BlackLevel Default Offset");

    oss << std::endl << "Default BlackLevel Offset : " << param;

    IS_RANGE_S32 nRange;
    nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_OFFSET_RANGE, (void*)&nRange, sizeof(nRange));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not get Camera BlackLevel Offset Range");

    oss << std::endl << "Blacklevel OffSet Range : min (" << nRange.s32Min << ") max (" << nRange.s32Max << ") inc (" << nRange.s32Inc << ")";

    nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_OFFSET, (void*)&param, sizeof(param));
    if (nRet != IS_SUCCESS) throw std::runtime_error("Could not get Camera BlackLevel Current Offset");

    oss << std::endl << "Current BlackLevel Offset : " << param;

    return oss.str();
}


ICamera::TCameraParams UEyeCamera::getCurrentParams() const
{
    TCameraParams params;

    params["black_level_capa"] = getBlackLevelCaps(m_hCam);
    params["black_level_defaults"] = getBlackLevelDefaults(m_hCam);
    params["exposure"] = getExposureCapabilities(m_hCam);

    return params;
}












