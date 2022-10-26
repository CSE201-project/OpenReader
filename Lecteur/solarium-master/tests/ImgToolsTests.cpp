//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/09/21.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "tools/ImgTools.h"
#include "tools/FSTools.h"
#include "TestTools.h"

#include <fstream>
#include <filesystem>
#include <opencv2/imgcodecs.hpp>

#include <Poco/Timestamp.h>

namespace fs = std::filesystem;

void save(const std::string &path, const std::vector<uint8_t> &imageData)
{
    fs::create_directories("/tmp/Solarium");
    std::ofstream rawPicture(path);
    rawPicture.write((const char *)imageData.data(), imageData.size());
    rawPicture.close();
}

SCENARIO("ImgTools", "[reports imgTools]") {
    GIVEN("A cropped trip image") {
        auto croppedImagePath = getResourcePath("cropped_strip.png");
        auto croppedImage = FSTools::getBinaryContentAsUInt8(croppedImagePath);

        WHEN("extract ROI for report") {
            uint32_t controlLinePos = 327;
            uint32_t testLinePos = 1131;
            double pixelsToMm = 139.66;
            double lengthROIMm = 2.0;
            auto imageForReport = ImgTools::extractROIForReport(croppedImage, controlLinePos, testLinePos, pixelsToMm, lengthROIMm);

            THEN("the image is valid") {
                REQUIRE(!imageForReport.empty());
                save("./cropped_strip_roi.png", imageForReport);
            }
        }
    }

    GIVEN("A camera capture") {
        WHEN("the capture is pink") {
            auto capturePath = getResourcePath("camera_capture_pink.png");
            auto capture = cv::imread(capturePath, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            THEN("capture should be detected as bad") {
                REQUIRE_FALSE(ImgTools::isCapture8BitsOK(capture));
            }
        }

        WHEN("the capture looks OK") {
            auto capturePath = getResourcePath("k7_precropped.png");
            auto capture = cv::imread(capturePath, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            THEN("capture should be detected as OK") {
                REQUIRE(ImgTools::isCapture8BitsOK(capture));
            }
        }

        WHEN("the full capture looks OK") {
            auto capturePath = getResourcePath("full_image.png");
            auto capture = cv::imread(capturePath, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            THEN("capture should be detected as OK") {
                REQUIRE(ImgTools::isCapture8BitsOK(capture));
            }
        }
    }
}
