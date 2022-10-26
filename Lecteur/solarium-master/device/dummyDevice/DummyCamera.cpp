//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "DummyCamera.h"
#include "../../SolariumApp.h"

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <opencv2/opencv.hpp>
/*
const std::string testImagePathUV = "./data/302_raw_picture.png";
const std::string testImagePathCrop = "./data/302_croppingPicture.png";
*/
const std::string testImagePathUV = "./data/Use Test Sang 302_256pgml_N01.png";
const std::string testImagePathCrop = "./data/Use Test Sang 302_256pgml_N01.png";
std::vector<uint8_t> DummyCamera::takePicture(const TCameraParams &params, const TRectangle &roi)
{
    LOGGER.debug("[DEBUG] Received %?d sized params", params.size());
    for (auto const & iter : params)
    {
        LOGGER.debug("[DEBUG]   Param %s : %s", iter.first, iter.second);
    }

    static bool useCrop = true;
    std::string testImagePath = useCrop ? testImagePathCrop : testImagePathUV;
    useCrop = !useCrop;

    try {
        std::ifstream fileStream(testImagePath, std::ios::in | std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        return data;
    }
    catch (std::exception &ex) {
        LOGGER.warning("Error reading dummy image in dummyCamera : %s", ex.what());
        return std::vector<uint8_t>();
    }
}

void DummyCamera::initialize()
{
}

cv::Mat DummyCamera::takePictureAsMat(FLedLightning ledLightning)
{
    static bool useCrop = true;
    std::string testImagePath = useCrop ? testImagePathCrop : testImagePathUV;
    useCrop = !useCrop;
    return cv::imread(testImagePath);;
}
