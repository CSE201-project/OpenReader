//
// Created by mario on 30/04/2021.
//

#ifndef V4L2CAMERA_V4L2CAMERA_H
#define V4L2CAMERA_V4L2CAMERA_H

#include <string>
#include <linux/videodev2.h>
#include "../ICamera.h"


class V4L2Camera : public ICamera {
public:
    V4L2Camera(const std::string & deviceNode);

    virtual ~V4L2Camera();

    void initialize() override;
    void uninitialize() override;

    cv::Mat takePictureAsMat(FLedLightning ledLightning = nullptr) override;
    std::vector<uint8_t> takePicture(const TCameraParams &params, const TRectangle &roi) override;


    void setupCapture(const TCameraParams & params, const TRectangle &roi) override;

    void displayDeviceInfo();
    void displayAvailableFormat();
    void displayAvailableControls();

private:
    const std::string _deviceNode;
    int _fd;
    v4l2_capability _capability;

    struct Buffer {
        void*   addr;
        size_t  length;
    };
    std::vector<Buffer> _buffers = {{0},{0},{0},{0}};
    std::vector<char> _currentImage;

    TRectangle _roi;
    int _ledWarmUpDuration = DEFAULT_LED_WARMUP_DURATION;

    void mioctl(int request, void *arg, const std::string &message);

    void open();
    void close();

    void setControls(std::map<std::string, int> controls);
    void setFormat(uint32_t width, uint32_t height);
    void capture(FLedLightning ledLightning);

    void applyControlsGroup(const std::vector<std::pair<uint32_t, int32_t>> &controlsToSet, uint32_t controlClass) const;
};


#endif //V4L2CAMERA_V4L2CAMERA_H
