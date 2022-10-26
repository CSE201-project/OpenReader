//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "common/imageProcessing/ImageAnalyzer.h"
#include "tests/TestTools.h"
#include "tests/common/imageProcessing/resources/Profiles.h"

SCENARIO("Find control line", " [findControl]") {

    GIVEN("profile with CL classic") {
        TImageProfile profile = ProfileClassic65;
        WHEN("with no params") {
            auto result = ImageAnalyzer::findControlLine(profile, 33);

            THEN("should have found CL leftBorder on first peak") {
                REQUIRE(result == 33);
            }
        }

        WHEN("correct searchArea") {
            auto result = ImageAnalyzer::findControlLine(profile, 33, 22, 62);

            THEN("should have found the right value") {
                REQUIRE(result == 41);
            }
        }
    }

    GIVEN("profile with CL with hollow") {
        TImageProfile profile = ProfileHollow65;
        WHEN("with no params") {
            auto result = ImageAnalyzer::findControlLine(profile, 31);

            THEN("should have found CL leftBorder on first peak") {
                REQUIRE(result == 32);
            }
        }

        WHEN("correct searchArea") {
            auto result = ImageAnalyzer::findControlLine(profile, 33,22, 62);

            THEN("should have found the right value") {
                REQUIRE(result == 40);
            }
        }
    }

    GIVEN("profile with CL saturated") {
        TImageProfile profile = ProfileSaturated65;
        WHEN("with no params") {
            auto result = ImageAnalyzer::findControlLine(profile, 33);

            THEN("should have found CL leftBorder on first peak") {
                REQUIRE(result == 32);
            }
        }

        WHEN("correct searchArea") {
            auto result = ImageAnalyzer::findControlLine(profile, 33,22, 62);

            THEN("should have found the right value") {
                REQUIRE(result == 40);
            }
        }
    }

    GIVEN("profile with CL with a 'finger' on the right") {
        TImageProfile profile = ProfileFinger65;
        WHEN("with no params") {
            auto result = ImageAnalyzer::findControlLine(profile, 33);

            THEN("should have found CL leftBorder on first peak") {
                REQUIRE(result == 33);
            }
        }

        WHEN("correct searchArea") {
            auto result = ImageAnalyzer::findControlLine(profile, 33, 22, 62);

            THEN("should have found the right value") {
                REQUIRE(result == 41);
            }
        }
    }

    GIVEN("profile with a gaussian CL ") {
        TImageProfile profile = ProfileGaussian71;
        WHEN("correct searchArea") {
            auto result = ImageAnalyzer::findControlLine(profile, 33, 22, 62);

            THEN("should have found the right value") {
                REQUIRE(result == 36);
            }
        }
    }
}
