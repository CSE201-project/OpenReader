//
// Created by mario on 30/04/2021.
//

#include "V4L2Camera.h"

#include <cctype>
#include <tuple>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <system_error>
#include <sys/mman.h>
#include <cstring>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <tools/ImgTools.h>
#include "SolariumApp.h"
#include "tools/Converter.h"

//#define LOG(x) std::cout << x << std::endl
#define LOG(x) do { \
    std::ostringstream oss; \
    oss << x;    \
    LOGGER.debug(oss.str()); \
} while (0)

/*
class Logger {
public:
    static Logger instance;

    void information(const std::string & message) {
        std::cout << "INFO: " << message << std::endl;
    }
};
Logger Logger::instance = Logger();
#define LOGGER Logger::instance
*/

// #####################################################################################################################
std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
    );
    return s;
}

// #####################################################################################################################
static int xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = ::ioctl(fh, request, arg);
    } while (-1 == r && (EINTR == errno || EAGAIN == errno));

    return r;
}
// #####################################################################################################################
static void displayFormat(const v4l2_format & imageFormat)
{
    LOG("V4L2 Pix Format :");
    LOG("\tSize   : " << imageFormat.fmt.pix.width << " x " << imageFormat.fmt.pix.height);
    LOG("\tFormat : " << imageFormat.fmt.pix.pixelformat);
}


// #####################################################################################################################
V4L2Camera::V4L2Camera(const std::string & deviceNode)
    : _deviceNode(deviceNode)
    , _fd(-1)
{
}

V4L2Camera::~V4L2Camera()
{
    this->close();
}


void V4L2Camera::mioctl(int request, void *arg, const std::string &message)
{
    if (xioctl(_fd, request, arg) < 0)
        throw std::system_error(errno, std::generic_category(), message.c_str());
}


void V4L2Camera::open()
{
    struct stat st;

    if (-1 == stat(_deviceNode.c_str(), &st))
        throw std::system_error(errno, std::system_category(), _deviceNode.c_str());

    if (!S_ISCHR(st.st_mode))
        throw std::runtime_error( _deviceNode + " is not a device");

    _fd = ::open(_deviceNode.c_str(), O_RDWR /* required  | O_NONBLOCK */, 0);

    if (-1 == _fd)
        throw std::system_error(errno, std::system_category(), _deviceNode.c_str());

    if (-1 == xioctl(_fd, VIDIOC_QUERYCAP, &_capability))
        throw std::system_error(errno, std::system_category(), _deviceNode.c_str());

    if (!(_capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        throw std::runtime_error(_deviceNode + " is not a video capture device");

    if (!(_capability.capabilities & V4L2_CAP_STREAMING))
        throw std::runtime_error(_deviceNode + " does not support streaming i/o");

    uint32_t width = 2592, height = 1944;

    setFormat(width, height);
}


void V4L2Camera::close()
{
    for (auto & buffer : _buffers)
    {
        ::free(buffer.addr);
    }

    if (_fd != -1) {
        LOGGER.information("Closing V4L2 Camera");
        ::close(_fd);
        _fd = -1;
    }
}

void V4L2Camera::initialize()
{
    LOG("V4L2Camera::initialize()");
    open();

    std::map<std::string, int> controls = {
            { "white balance temperature, auto", 0 },
            { "exposure, auto", 1 },
            { "exposure (absolute)", 600 },
            { "brightness", 8 },
            { "contrast", 8 },
            { "saturation", 7 },
            { "hue", 0 },
    };

    setControls(controls);
}

void V4L2Camera::uninitialize()
{
    this->close();
}

void V4L2Camera::setFormat(uint32_t width, uint32_t height)
{
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    mioctl(VIDIOC_G_FMT, &fmt, "VIDIOC_G_FMT");

    displayFormat(fmt);

    if (fmt.fmt.pix.width != width || fmt.fmt.pix.height != height
    || fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_MJPEG)
    {
        fmt.fmt.pix.width       = width;
        fmt.fmt.pix.height      = height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        mioctl(VIDIOC_S_FMT, &fmt, "VIDIOC_S_FMT");
    }

    // 4. Request Buffers from the device
    v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = _buffers.size(); // one request addr
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a addr wich we an use for capturing frames
    requestBuffer.memory = V4L2_MEMORY_USERPTR;
    mioctl(VIDIOC_REQBUFS, &requestBuffer, "Request Use of User Pointers");

    if (requestBuffer.count < _buffers.size())
        throw std::runtime_error("Insufficient buffer memory");

    /* Buggy driver paranoia. */
    auto min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    for (int i = 0; i < _buffers.size(); i++)
    {
        _buffers.at(i).addr = ::malloc(fmt.fmt.pix.sizeimage);
        _buffers.at(i).length = fmt.fmt.pix.sizeimage;
        if (_buffers.at(i).addr == nullptr)
            throw std::runtime_error("Out of memory for camera init");
        else
            ::memset(_buffers.at(i).addr, 0, fmt.fmt.pix.sizeimage);
    }
}

void V4L2Camera::setupCapture(const TCameraParams & params, const TRectangle &roi)
{
    //LOG("setup capture");
    _roi = roi;
    /*
     *
     *               brightness 0x00980900 (int)    : min=0 max=15 step=1 default=8 value=8
                       contrast 0x00980901 (int)    : min=0 max=15 step=1 default=8 value=8
                     saturation 0x00980902 (int)    : min=0 max=15 step=1 default=7 value=7
                            hue 0x00980903 (int)    : min=-10 max=10 step=1 default=0 value=0
 white_balance_temperature_auto 0x0098090c (bool)   : default=1 value=1
                          gamma 0x00980910 (int)    : min=1 max=10 step=1 default=7 value=7
                           gain 0x00980913 (int)    : min=0 max=0 step=0 default=0 value=0
           power_line_frequency 0x00980918 (menu)   : min=0 max=2 default=2 value=2
				0: Disabled
				1: 50 Hz
				2: 60 Hz
      white_balance_temperature 0x0098091a (int)    : min=2800 max=6500 step=1 default=2800 value=2800 flags=inactive
                      sharpness 0x0098091b (int)    : min=0 max=15 step=1 default=6 value=6
         backlight_compensation 0x0098091c (int)    : min=0 max=1 step=1 default=0 value=0
                  exposure_auto 0x009a0901 (menu)   : min=0 max=3 default=3 value=3
				1: Manual Mode
				3: Aperture Priority Mode
              exposure_absolute 0x009a0902 (int)    : min=4 max=5000 step=1 default=625 value=625 flags=inactive
                 focus_absolute 0x009a090a (int)    : min=0 max=21 step=1 default=16 value=16 flags=inactive
                     focus_auto 0x009a090c (bool)   : default=1 value=1

     *
     *
     */

    std::map<std::string, std::tuple<int, int>> controls = {
            { "brightness", { 0, 15 } },
            { "contrast", { 0, 15 } },
            { "saturation", { 0, 15 } },
            { "hue", { -10, 10 } },
            { "gamma", { 1, 10 } },
            { "sharpness", { 0, 15 } },
            { "backlight_compensation", { 0, 1 } },
            { "exposure__", { 4, 5000 } }
    };

    std::map<std::string, int> paramsToSet;
    for (auto kv : params)
    {
        if (auto it = controls.find(kv.first); it != controls.end())
        {
            int val = std::stoi(kv.second);
            if (std::get<0>(it->second) <= val && val <= std::get<1>(it->second))
                paramsToSet[it->first] = val;
        } else if (kv.first == "exposure"){
            int val = std::stoi(kv.second);
            if (4 <= val && val <= 5000)
            {
                paramsToSet["exposure (absolute)"] = val;
            }
        }
    }

    setControls(paramsToSet);
}

void V4L2Camera::capture(FLedLightning ledLightning)
{
    LOG("Capture");

    Poco::Timestamp captureStartTime;

    for (int i = 0; i < _buffers.size(); i++)
    {
        v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;
        buf.m.userptr = (unsigned long)_buffers.at(i).addr;
        buf.length = _buffers.at(i).length;

        mioctl(VIDIOC_QBUF, &buf, "Queue Buffer");
    }

    //LOG("Stream On");
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    mioctl(VIDIOC_STREAMON, &type, "Stream ON");

    try
    {
        int count = 0;
        bool continueStream = true;
        bool isFirstFrame = true;

        Poco::Timestamp lastCapture;
        Poco::Timestamp LEDWaitingTime;
        while (continueStream)
        {
            fd_set fds;
            struct timeval tv;
            int r;

            do
            {
                FD_ZERO(&fds);
                FD_SET(_fd, &fds);

                /* Timeout. */
                tv.tv_sec = 8;
                tv.tv_usec = 0;

                r = select(_fd + 1, &fds, NULL, NULL, &tv);

                if (r == 0) throw std::runtime_error("Timeout reading from the camera");

            } while (r == -1 && errno == EINTR);

            v4l2_buffer deQueuedBuffer = {0};
            deQueuedBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            deQueuedBuffer.memory = V4L2_MEMORY_USERPTR;
            mioctl(VIDIOC_DQBUF, &deQueuedBuffer, "Dequeue Buffer");

            if (isFirstFrame && ledLightning != nullptr)
            {
                // turn led on
                isFirstFrame = false;
                ledLightning(true);
                LEDWaitingTime.update();
            }
            else if (LEDWaitingTime.isElapsed(_ledWarmUpDuration) || ledLightning == nullptr)
            {
                continueStream = false;

                uint32_t bufferIndex;
                for (bufferIndex = 0; bufferIndex < _buffers.size(); bufferIndex++)
                    if (deQueuedBuffer.m.userptr == (unsigned long)_buffers.at(bufferIndex).addr)
                        break;

                if (!(bufferIndex < _buffers.size()))
                    throw std::runtime_error("Camera returned bad pointer");

                _currentImage.clear();
		        std::copy((char *) _buffers.at(bufferIndex).addr,
                          (char *) _buffers.at(bufferIndex).addr + deQueuedBuffer.bytesused,
                          std::back_inserter(_currentImage));
            }

            mioctl(VIDIOC_QBUF, &deQueuedBuffer, "Requeue Buffer");
            LOG("Capture : One frame processed");

            lastCapture.update();
        }
    }
    catch (const std::exception & ex) {
        LOG("Capture Error : " << std::string(ex.what()));

        if (ledLightning) ledLightning(false);
        mioctl(VIDIOC_STREAMOFF, &type, "Stream Off");

        throw ex;
    }

    if (ledLightning) ledLightning(false);
    mioctl(VIDIOC_STREAMOFF, &type, "Stream Off");
}

cv::Mat V4L2Camera::takePictureAsMat(FLedLightning ledLightning)
{
    Poco::Timestamp ts;
    capture(ledLightning);

    //LOGGER.debug("takePictureAsMat, capture %?d ms", ts.elapsed() / 1000);

    cv::Mat capture(1, _currentImage.size(), CV_8UC1, (void*)_currentImage.data());
    cv::Mat result = cv::imdecode(capture, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);
    /*
    cv::Mat capture(1944*1.5, 2592, CV_8UC1, (void*)_currentImage.data());
    cv::Mat result(1944, 2592, CV_8UC3);
    cv::cvtColor(capture, result, cv::COLOR_YUV2BGR_YUYV);
    */
    cv::Mat resultCloned;

    //LOGGER.debug("takePictureAsMat, converted to cv::Mat %?d ms", ts.elapsed() / 1000);

    if (_roi.width != result.cols || _roi.height != result.rows)
    {
        cv::Mat cropped = result(cv::Rect(_roi.x, _roi.y, _roi.width, _roi.height));
        resultCloned = cropped.clone();
    }
    else
    {
        resultCloned = result.clone();
    }

    if (ImgTools::isCapture8BitsOK(resultCloned))
        return resultCloned;
    else
        throw std::runtime_error("Capture has an error (pink image)");
}

std::vector<uint8_t> V4L2Camera::takePicture(const ICamera::TCameraParams &params, const ICamera::TRectangle &roi)
{
    setupCapture(params, roi);
    auto picture = takePictureAsMat();

    return Converter::cvMatToPng(picture);
}

#define TO_STR(x) #x
#define HAS_CAPA(x) if (_capability.capabilities & x) oss << TO_STR(x) << " : " << #x << std::endl

void V4L2Camera::displayDeviceInfo()
{
    std::ostringstream oss;
    oss << "Device Capabilities:" << std::endl;

    HAS_CAPA(V4L2_CAP_VIDEO_CAPTURE); /* Is a video capture device */
    HAS_CAPA(V4L2_CAP_VIDEO_OUTPUT); /* Is a video output device */
    HAS_CAPA(V4L2_CAP_VIDEO_OVERLAY); /* Can do video overlay */
    HAS_CAPA(V4L2_CAP_VBI_CAPTURE); /* Is a raw VBI capture device */
    HAS_CAPA(V4L2_CAP_VBI_OUTPUT); /* Is a raw VBI output device */
    HAS_CAPA(V4L2_CAP_SLICED_VBI_CAPTURE); /* Is a sliced VBI capture device */
    HAS_CAPA(V4L2_CAP_SLICED_VBI_OUTPUT); /* Is a sliced VBI output device */
    HAS_CAPA(V4L2_CAP_RDS_CAPTURE); /* RDS data capture */
    HAS_CAPA(V4L2_CAP_VIDEO_OUTPUT_OVERLAY); /* Can do video output overlay */
    HAS_CAPA(V4L2_CAP_HW_FREQ_SEEK); /* Can do hardware frequency seek  */
    HAS_CAPA(V4L2_CAP_RDS_OUTPUT); /* Is an RDS encoder */

    HAS_CAPA(V4L2_CAP_VIDEO_CAPTURE_MPLANE);
    HAS_CAPA(V4L2_CAP_VIDEO_OUTPUT_MPLANE);
    HAS_CAPA(V4L2_CAP_VIDEO_M2M_MPLANE);
    HAS_CAPA(V4L2_CAP_VIDEO_M2M);
    HAS_CAPA(V4L2_CAP_TUNER);  /* has a tuner */
    HAS_CAPA(V4L2_CAP_AUDIO);  /* has audio support */
    HAS_CAPA(V4L2_CAP_RADIO);  /* is a radio device */
    HAS_CAPA(V4L2_CAP_MODULATOR);  /* has a modulator */
    HAS_CAPA(V4L2_CAP_SDR_CAPTURE);  /* Is a SDR capture device */
    HAS_CAPA(V4L2_CAP_EXT_PIX_FORMAT);  /* Supports the extended pixel format */
    HAS_CAPA(V4L2_CAP_SDR_OUTPUT);  /* Is a SDR output device */
    HAS_CAPA(V4L2_CAP_META_CAPTURE);  /* Is a metadata capture device */
    HAS_CAPA(V4L2_CAP_READWRITE);  /* read/write systemcalls */
    HAS_CAPA(V4L2_CAP_ASYNCIO);  /* async I/O */
    HAS_CAPA(V4L2_CAP_STREAMING);  /* streaming I/O ioctls */
    HAS_CAPA(V4L2_CAP_META_OUTPUT);  /* Is a metadata output device */
    HAS_CAPA(V4L2_CAP_TOUCH);  /* Is a touch device */
    HAS_CAPA(V4L2_CAP_DEVICE_CAPS);

    std::cout << oss.str() << std::endl;
}

void V4L2Camera::displayAvailableFormat()
{
    v4l2_fmtdesc fmtDesc = {0};
    fmtDesc.index = 0;
    fmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    LOG("Available Format");
    int res = xioctl(_fd, VIDIOC_ENUM_FMT, &fmtDesc);
    for (; res == 0;res = xioctl(_fd, VIDIOC_ENUM_FMT, &fmtDesc))
    {
        LOG("\t'" << std::string((char*)fmtDesc.description) << "' (" << fmtDesc.pixelformat << ")");
        fmtDesc.index++;
    }
}

void V4L2Camera::displayAvailableControls()
{
    v4l2_queryctrl queryCtrl = {0};
    v4l2_querymenu queryMenu = {0};

    LOG("Available Controls :");

    queryCtrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    while (!xioctl(_fd, VIDIOC_QUERYCTRL, &queryCtrl))
    {
        if (!(queryCtrl.flags & V4L2_CTRL_FLAG_DISABLED))
        {
            LOG("\t" << queryCtrl.name);
            LOG("\t\tmin " << queryCtrl.minimum << " max " << queryCtrl.maximum << " default " << queryCtrl.default_value << " step" << queryCtrl.step);
        }

        queryCtrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}

void V4L2Camera::setControls(std::map<std::string, int> controls)
{
    std::map<std::string, uint32_t> mapControlsToSet = {
            {"brightness", V4L2_CID_BRIGHTNESS},
            {"contrast", V4L2_CID_CONTRAST},
            {"saturation", V4L2_CID_SATURATION},
            {"hue", V4L2_CID_HUE},
            {"gamma", V4L2_CID_GAMMA},
            {"sharpness", V4L2_CID_SHARPNESS},
            {"white balance temperature, auto", V4L2_CID_AUTO_WHITE_BALANCE}
    };

    std::map<std::string, uint32_t> mapControlsCameraToSet = {
            {"exposure (absolute)", V4L2_CID_EXPOSURE_ABSOLUTE},
            {"exposure, auto", V4L2_CID_EXPOSURE_AUTO}
    };

    std::vector<std::pair<uint32_t, int32_t>> controlList, cameraControlList;

    for (auto & param : controls)
    {
        if (const auto & it = mapControlsToSet.find(param.first); it != mapControlsToSet.end())
            controlList.push_back({it->second, param.second});
        else if (const auto & it = mapControlsCameraToSet.find(param.first); it != mapControlsCameraToSet.end())
            cameraControlList.push_back({it->second, param.second});
    }

    if (!controlList.empty())
        applyControlsGroup(controlList, V4L2_CTRL_CLASS_USER);

    if (!cameraControlList.empty())
        applyControlsGroup(cameraControlList, V4L2_CTRL_CLASS_CAMERA);
}

void V4L2Camera::applyControlsGroup(const std::vector<std::pair<uint32_t, int32_t>> &controlsToSet, uint32_t controlClass) const
{
    std::vector<v4l2_ext_control> extControls;
    for (auto ctrlToSet : controlsToSet)
    {
        v4l2_ext_control extCtrl = {0};
        extCtrl.id = ctrlToSet.first;
        extCtrl.value = ctrlToSet.second;
        extControls.push_back(extCtrl);
    }

    v4l2_ext_controls extControlGroup = {0};
    extControlGroup.ctrl_class = controlClass;
    extControlGroup.count = extControls.size();
    auto t = extControls.data();
    extControlGroup.controls = extControls.data();

    for (int nbTries = 3; nbTries>0; nbTries--)
    {
        auto res = xioctl(_fd, VIDIOC_S_EXT_CTRLS, &extControlGroup);
        if (res != 0)
        {
            LOG("Error (" << errno << ":" << strerror(errno)
                          << ") setting extControlGroup " << controlClass
                          << ", faulting control index " << unsigned(extControlGroup.error_idx)
                          << " - will retry");
        }
        else
            break;
    }
}
