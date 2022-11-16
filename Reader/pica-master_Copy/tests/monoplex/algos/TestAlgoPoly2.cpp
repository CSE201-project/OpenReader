//
// Created by KarimSadki on 04/02/2021.
//

#include "catch2/catch.hpp"
#include "monoplex/algos/AlgoPoly2.h"
#include "tests/TestTools.h"
#include "tests/common/imageProcessing/resources/Profiles.h"

SCENARIO("Test AlgoPoly2 ", " [algo poly2]") {

    uint32_t rectangleWidth = 30;

    uint32_t controlLine = 63 + (rectangleWidth / 2);//matlab value - 1
    uint32_t firstTestLine = 284 + (rectangleWidth / 2);//matlab value - 1

    std::vector<uint32_t> testAreas = { controlLine, firstTestLine };

    ProcessorConfig processorConfig(cv::Mat(),
                                    false,
                                    Red,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    2.3);
    AlgoPoly2 poly2;
    poly2.setConfig(processorConfig);
    poly2.setROILengths(50, 25, 25);

    GIVEN("profile of croppedProtoV1 with padding of 38") {
        WHEN("lines are correct") {
            auto [poly2ProfileFitted, poly2Coefficient] = poly2.transformProfileForProcessing(ProfileProtoV1, testAreas);
            auto [result, coefficient] = poly2.processImageProfile(poly2ProfileFitted, testAreas);

            THEN("should return something positive") {
                REQUIRE(result > 0);
                REQUIRE(poly2Coefficient < 1);
            }
        }
    }

    GIVEN("blank profile") {
        WHEN("lines are correct") {
            auto [poly2ProfileFitted, poly2Coefficient] = poly2.transformProfileForProcessing(ProfileProtoV1Blank,
                                                                                              testAreas);
            auto [result, coefficient] = poly2.processImageProfile(poly2ProfileFitted, testAreas);

            THEN("should return something negative") {
                REQUIRE(result > -0.0000001);
                REQUIRE(result < 0.0000001);
                REQUIRE(poly2Coefficient < 1);            }
        }
    }

    ProxNoiseROISizeMm proxNoiseDefaultSizes = {
            .TLLength = 1.3,
            .noiseBeforeTL = 1.,
            .noiseAfterTL = 1.};

    double ratioNumberPixels301 = 139.66;
    double ratioNumberPixels302 = 137.76;
    double ratioNumberPixels304 = 136.31;
    double ratioNumberPixels305 = 135.21;

    GIVEN("cropped 301 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 301 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels301, 1, 2.3);
            AlgoPoly2 algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.836") {
                REQUIRE(resultRaw > 0.835);
                REQUIRE(resultRaw < 0.837);
            }
            THEN("convol result should be 0.836") {
                REQUIRE(resultConvol > 0.835);
                REQUIRE(resultConvol < 0.837);
            }
        }
    }

    GIVEN("cropped 302 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 302 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels302, 1, 2.3);
            AlgoPoly2 algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 1.183") {
                REQUIRE(resultRaw > 1.182);
                REQUIRE(resultRaw < 1.184);
            }
            THEN("convol result should be 0.951") {
                REQUIRE(resultConvol > 0.950);
                REQUIRE(resultConvol < 0.952);
            }
        }
    }

    GIVEN("cropped 304 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 304 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels304, 1, 2.3);
            AlgoPoly2 algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.858") {
                REQUIRE(resultRaw > 0.857);
                REQUIRE(resultRaw < 0.859);
            }
            THEN("convol result should be 0.797") {
                REQUIRE(resultConvol > 0.796);
                REQUIRE(resultConvol < 0.798);
            }
        }
    }

    GIVEN("cropped 305 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 305 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels305, 1, 2.3);
            AlgoPoly2 algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 1.112") {
                REQUIRE(resultRaw > 1.111);
                REQUIRE(resultRaw < 1.113);
            }
            THEN("convol result should be 0.859") {
                REQUIRE(resultConvol > 0.858);
                REQUIRE(resultConvol < 0.86);
            }
        }
    }
}