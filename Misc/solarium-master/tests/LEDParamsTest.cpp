//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/20.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "models/LEDParams.h"

SCENARIO("LEDParams", "[models]") {
    GIVEN("An empty string") {
        std::string s = "";

        THEN("TLEDParam has default value") {
            auto led = LEDParams::parse(s, LEDParams::UV);
            REQUIRE_FALSE(led.enabled.has_value());
            REQUIRE(led.intensity == 0);
        }
    }
    GIVEN("{\"intensity\":12,\"status\":\"enable\"}") {
        std::string s = "{\"intensity\":12,\"status\":\"enable\"}";

        THEN("TLEDParam has valid values") {
            auto led = LEDParams::parse(s, LEDParams::UV);
            REQUIRE(led.enabled.has_value());
            REQUIRE(led.enabled.value());
            REQUIRE(led.intensity == 12);
        }
    }
    GIVEN("{\"intensity\":256,\"status\":\"enabled\"}") {
        std::string s = "{\"intensity\":255,\"status\":\"enabled\"}";

        THEN("TLEDParam has valid values") {
            auto led = LEDParams::parse(s, LEDParams::UV);
            REQUIRE(led.enabled.has_value());
            REQUIRE(led.enabled.value());
            REQUIRE(led.intensity == 255);
        }
    }
    GIVEN("{\"intensity\":12,\"status\":\"disabled\"}") {
        std::string s = "{\"intensity\":12,\"status\":\"disabled\"}";

        THEN("TLEDParam has valid values") {
            auto led = LEDParams::parse(s, LEDParams::UV);
            REQUIRE(led.enabled.has_value());
            REQUIRE_FALSE(led.enabled.value());
            REQUIRE(led.intensity == 12);
        }
    }
}
