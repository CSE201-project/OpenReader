//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/01/11.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "common/imageProcessing/ImageAnalyzer.h"
#include "tests/TestTools.h"

#include <cmath>

SCENARIO("Compute Profile", " [profile]") {
    ImageAnalyzer algo;

    GIVEN("Picture Proto 203 (UV 8Bits)") {
        auto img = cv::imread(getResourcePath("/common/imageProcessing/resources/picture_proto_203_2.png"));

        WHEN("raw image is analyzed") {
            auto result = algo.computeImageProfile(img);

            THEN("should return values above 0.00001") {
                for (const auto & r : result)
                    REQUIRE(r > 0.00001);
            }
        }
    }
}

