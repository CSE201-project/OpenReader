//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/18.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "monoplex/algos/AlgoTestMinusProxNoise.h"
#include "tests/TestTools.h"

#include <cmath>

SCENARIO("Algo TestMinusProxNoise", " [algo testminusproxnoise]") {

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
            AlgoTestMinusProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 71.442") {
                REQUIRE(resultRaw > 71.441);
                REQUIRE(resultRaw < 71.443);
            }
            THEN("convol result should be 5.693") {
                REQUIRE(resultConvol > 5.692);
                REQUIRE(resultConvol < 5.694);
            }
        }
    }

    GIVEN("cropped 302 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 302 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels302, 1, 2.3);
            AlgoTestMinusProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 97.93") {
                REQUIRE(resultRaw > 97.92);
                REQUIRE(resultRaw < 97.94);
            }
            THEN("convol result should be 6.209") {
                REQUIRE(resultConvol > 6.208);
                REQUIRE(resultConvol < 6.210);
            }
        }
    }

    GIVEN("cropped 304 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 304 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels304, 1, 2.3);
            AlgoTestMinusProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 60.883") {
                REQUIRE(resultRaw > 60.882);
                REQUIRE(resultRaw < 60.884);
            }
            THEN("convol result should be 18.574") {
                REQUIRE(resultConvol > 18.573);
                REQUIRE(resultConvol < 18.575);
            }
        }
    }

    GIVEN("cropped 305 F060.png") {

        std::string filePath = getResourcePath("monoplex/algos/resources/cropped 305 F060.png");
        const cv::Mat image = cv::imread(filePath, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("ProcessorConfig is correct") {

            ProcessorConfig processorConfig(image, false, Red, 2, 6, 1, ratioNumberPixels305, 1, 2.3);
            AlgoTestMinusProxNoise algo;

            ProfileData profileDataRaw = algo.computeProfileForAnalyze(processorConfig, false);
            ProfileData profileDataConvol = algo.computeProfileForAnalyze(processorConfig, true);

            double resultRaw = algo.compute(processorConfig, profileDataRaw, proxNoiseDefaultSizes);
            double resultConvol = algo.compute(processorConfig, profileDataConvol, proxNoiseDefaultSizes);

            THEN("raw result should be 84.449") {
                REQUIRE(resultRaw > 84.448);
                REQUIRE(resultRaw < 84.45);
            }
            THEN("convol result should be 4.008") {
                REQUIRE(resultConvol > 4.007);
                REQUIRE(resultConvol < 4.009);
            }
        }
    }
}

