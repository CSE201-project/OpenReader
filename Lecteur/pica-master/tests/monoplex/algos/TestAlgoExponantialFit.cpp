//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "monoplex/algos/AlgoExponantialFit.h"
#include "tests/TestTools.h"
#include "tests/common/imageProcessing/resources/Profiles.h"

SCENARIO("Test AlgoExponantialFit ", " [algo expoFit]") {

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

    AlgoExponantialFit exponantialFit;
    exponantialFit.setConfig(processorConfig);
    exponantialFit.setROILengths(50, 25, 25);

    GIVEN("profile of croppedProtoV1 with padding of 38") {
        WHEN("lines are correct") {
            auto [expoFitProfileFitted, expoFitCoefficient] = exponantialFit.transformProfileForProcessing(
                    ProfileProtoV1, testAreas);
            auto [result, coefficient] = exponantialFit.processImageProfile(expoFitProfileFitted, testAreas);

            THEN("should return something positive") {
                REQUIRE(result > 0);
                REQUIRE(expoFitCoefficient < 1);
            }
        }
    }

    GIVEN("blank profile") {
        WHEN("lines are correct") {
            auto [expoFitProfileFitted, expoFitCoefficient] = exponantialFit.transformProfileForProcessing(
                    ProfileProtoV1Blank, testAreas);
            auto [result, coefficient] = exponantialFit.processImageProfile(expoFitProfileFitted, testAreas);

            THEN("should return zero") {
                REQUIRE(result > -0.0000001);
                REQUIRE(result < 0.0000001);
                REQUIRE(expoFitCoefficient < 1);
            }
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
            AlgoExponantialFit algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.847") {
                REQUIRE(resultRaw > 0.846);
                REQUIRE(resultRaw < 0.848);
            }
            THEN("convol result should be 0.832") {
                REQUIRE(resultConvol > 0.831);
                REQUIRE(resultConvol < 0.833);
            }
        }
    }

    GIVEN("cropped 302 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 302 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels302, 1, 2.3);
            AlgoExponantialFit algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.95") {
                REQUIRE(resultRaw > 0.949);
                REQUIRE(resultRaw < 0.951);
            }
            THEN("convol result should be 0.886") {
                REQUIRE(resultConvol > 0.885);
                REQUIRE(resultConvol < 0.887);
            }
        }
    }

    GIVEN("cropped 304 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 304 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels304, 1, 2.3);
            AlgoExponantialFit algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.688") {
                REQUIRE(resultRaw > 0.687);
                REQUIRE(resultRaw < 0.689);
            }
            THEN("convol result should be 0.750") {
                REQUIRE(resultConvol > 0.749);
                REQUIRE(resultConvol < 0.751);
            }
        }
    }

    GIVEN("cropped 305 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 305 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels305, 1, 2.3);
            AlgoExponantialFit algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.937") {
                REQUIRE(resultRaw > 0.936);
                REQUIRE(resultRaw < 0.938);
            }
            THEN("convol result should be 0.836") {
                REQUIRE(resultConvol > 0.855);
                REQUIRE(resultConvol < 0.857);
            }
        }
    }
}
