//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/18.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "monoplex/algos/AlgoProxNoise.h"
#include "tests/TestTools.h"

#include "tests/common/imageProcessing/resources/Profiles.h"
#include <cmath>

SCENARIO("Algo ProxNoise", " [algo nearbynoise]") {

    AlgoProxNoise algoProxNoise;
    algoProxNoise.setROILengths(10, 5, 5);

    std::vector<uint32_t> testLines = {30, 70};

    GIVEN("ProfileBlank") {

        WHEN("noise and test length are correct") {
            auto [result, coefficient] = algoProxNoise.processImageProfile(ProfileBlank100, testLines);

            THEN("should return zero and NaN") {
                REQUIRE(result > -0.0000001);
                REQUIRE(result < 0.0000001);
                REQUIRE(std::isnan(coefficient));
            }
        }
    }

    GIVEN("ProfileNearbyNoise") {

        WHEN("noise and test length are correct") {
            auto [result, coefficient] = algoProxNoise.processImageProfile(ProfileNearbyNoise100, testLines);

            THEN("should return 0.25") {
                REQUIRE(result > 0.24999999);
                REQUIRE(result < 0.25000001);
                REQUIRE(std::isnan(coefficient));
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
            AlgoProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.833") {
                REQUIRE(resultRaw > 0.832);
                REQUIRE(resultRaw < 0.834);
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
            AlgoProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.953") {
                REQUIRE(resultRaw > 0.952);
                REQUIRE(resultRaw < 0.954);
            }
            THEN("convol result should be 0.949") {
                REQUIRE(resultConvol > 0.948);
                REQUIRE(resultConvol < 0.950);
            }
        }
    }

    GIVEN("cropped 304 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 304 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels304, 1, 2.3);
            AlgoProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.789") {
                REQUIRE(resultRaw > 0.788);
                REQUIRE(resultRaw < 0.790);
            }
            THEN("convol result should be 0.786") {
                REQUIRE(resultConvol > 0.785);
                REQUIRE(resultConvol < 0.787);
            }
        }
    }

    GIVEN("cropped 305 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 305 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels305, 1, 2.3);
            AlgoProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 0.854") {
                REQUIRE(resultRaw > 0.853);
                REQUIRE(resultRaw < 0.855);
            }
            THEN("convol result should be 0.856") {
                REQUIRE(resultConvol > 0.855);
                REQUIRE(resultConvol < 0.857);
            }
        }
    }
}

