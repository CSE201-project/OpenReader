//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/28.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include <catch2/catch.hpp>
#include "models/LensFocal.h"
#include <common/cropper/CropperStructsAndData.h>

SCENARIO("LensFocal", "[param]") {
    GIVEN("F041, series2") {
        LensFocal focal("F041", LensFocal::Series2);
        THEN("should get the right enum") {
            REQUIRE(focal.type() == DeviceFocal::ProtoV2_DefaultFocal);
        }
    }
    GIVEN("F041, series3") {
        LensFocal focal("F041", LensFocal::Series3);
        THEN("should get the right enum") {
            REQUIRE(focal.type() == DeviceFocal::ProtoV3_F041);
        }
    }
    GIVEN("F060, series3") {
        LensFocal focal("F060", LensFocal::Series3);
        THEN("should get the right enum") {
            REQUIRE(focal.type() == DeviceFocal::ProtoV3_F060);
        }
    }
    GIVEN("F080, series3") {
        LensFocal focal("F080", LensFocal::Series3);
        THEN("should get the right enum") {
            REQUIRE(focal.type() == DeviceFocal::ProtoV3_F080);
        }
    }
    GIVEN("unknown, series3") {
        LensFocal focal("unknown", LensFocal::Series3);
        THEN("should get the right enum") {
            REQUIRE(focal.type() == DeviceFocal::UnknownFocal);
        }
    }
    GIVEN("A valid LensFocal(F060, series3)") {
        LensFocal focal("F060", LensFocal::Series3);
        THEN("the copy should be the same") {
            LensFocal copyFocal = focal;
            REQUIRE(focal.type() == copyFocal.type());
        }
    }
}
