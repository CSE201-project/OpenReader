//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/18.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "common/cropper/StripCropperRectangleDetection.h"
#include "tests/TestTools.h"

void cropperRectangleDetectionUnitTest(CropParams cropParams,
                                       std::string imagePath,
                                       bool expectedCroppingResult,
                                       CroppingValidationMessage expectedMessage)
{
    std::string attempt = "correct";
    if(!expectedCroppingResult)
        attempt = "bad";

    StripCropperRectangleDetection cropper(cropParams);

    std::string filePath = getResourcePath(imagePath);
    auto fullImage = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

    WHEN("with good params") {
        std::vector<cv::Mat> intermediateSteps;
        auto cropped = cropper.crop(fullImage, fullImage);
        auto [isValid, message] = StripCropperRectangleDetection::isValidCroppedImage(cropped,
                                                                                      cropParams);

        THEN("should return " + attempt + " cropped image") {
            if (!intermediateSteps.empty())
                for (int i = 0; i < intermediateSteps.size(); i++)
                {
                    cv::imwrite(filePath + "." + std::to_string(i) + ".png", intermediateSteps.at(i));
                }
            /*if (!cropped.empty())
                cv::imwrite(filePath + ".result.png", cropped);*/
            REQUIRE(!cropped.empty());
            REQUIRE(isValid == expectedCroppingResult);
            REQUIRE(message == expectedMessage);
        }
    }
}

SCENARIO("Cropper RectangleDetection", " [cropper]") {
    GIVEN("the file K701WT_SE 204.png") {
        CropParams cropParams = {
                Proto_v2_PreCropped,
                Bits8,
                UV,
                K701WT,
                43.63,
                Serum,
                ProtoV2_DefaultFocal
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701WT_SE 204.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 204.png") {
        CropParams cropParams = {
                Proto_v2_PreCropped,
                Bits8,
                UV,
                K701BT,
                43.63,
                Serum,
                ProtoV2_DefaultFocal
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 204.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 302 F041.png") {
        CropParams cropParams = {
                Proto_v3,
                Bits8,
                Visible,
                K701BT,
                89.45,
                Serum,
                ProtoV3_F041
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 302 F041.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 302 F041bis.png") {
        CropParams cropParams = {
                Proto_v3,
                Bits8,
                Visible,
                K701BT,
                89.45,
                Serum,
                ProtoV3_F041
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 302 F041bis.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 302 F041ter.png") {
        CropParams cropParams = {
                Proto_v3,
                Bits8,
                UV,
                K701BT,
                89.45,
                Blood,
                ProtoV3_F041
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 302 F041ter.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 205 F060.png") {
        CropParams cropParams = {
                Proto_v2_PreCropped,
                Bits8,
                Visible,
                K701BT,
                66.87,
                Serum,
                ProtoV2_F060
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 205 F060.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 301 F060.png") {
        CropParams cropParams = {
                Proto_v3,
                Bits8,
                Visible,
                K701BT,
                139.66,
                Serum,
                ProtoV3_F060
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 301 F060.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 301 F080.png") {
        CropParams cropParams = {
                Proto_v3,
                Bits8,
                Visible,
                K701BT,
                216.81,
                Serum,
                ProtoV3_F080
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 301 F080.png", true, IsValid);
    }

    GIVEN("the file K701BT_SE 303 F060_L.png") {
        CropParams cropParams = {
                Proto_v3,
                Bits8,
                Visible,
                K701BT,
                151.57,
                Serum,
                ProtoV3_F060_L
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/K701BT_SE 303 F060_L.png", true, IsValid);
    }

    GIVEN("the file crop_error_large.png") {
        CropParams cropParams = {
                Proto_v2_PreCropped,
                Bits8,
                UV,
                K701WT,
                43.63,
                Serum,
                ProtoV2_DefaultFocal
        };
        cropperRectangleDetectionUnitTest(cropParams, "common/cropper/resources/crop_error_large.png", false, TooLarge);
    }
}
