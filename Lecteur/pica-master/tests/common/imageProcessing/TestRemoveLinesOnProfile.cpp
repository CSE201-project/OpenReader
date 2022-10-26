//
// Created by KarimSadki on 10/02/2021.
//

#include "catch2/catch.hpp"
#include "common/imageProcessing/ImageAnalyzer.h"
#include "tests/TestTools.h"
#include "tests/common/imageProcessing/resources/Profiles.h"


SCENARIO("Remove lines on profile", " [removeLines]") {

    GIVEN("Profile filled by integers of 1 to 100") {
        uint32_t maskLength = 30;

        WHEN("CL index greater than halfMaskLength") {
            std::vector<uint32_t> linesAreas = {20, 70};
            TImageProfile profileWithoutLines = ImageAnalyzer::removeLinesOnProfile(Profile1To100, linesAreas, maskLength);

            THEN("should return profile with NaN values between indexes 5-35 and 55-85") {
                REQUIRE(profileWithoutLines.size() == Profile1To100.size());
                for(int i = 0; i < 5; i++)
                    REQUIRE(profileWithoutLines[i] == Profile1To100[i]);
                for(int shift = 5, i = shift; i < shift + maskLength; i++)
                    REQUIRE(std::isnan(profileWithoutLines[i]));
                for(int i = 35; i < 55; i++)
                    REQUIRE(profileWithoutLines[i] == Profile1To100[i]);
                for(int shift = 55, i = shift; i < shift + maskLength; i++)
                    REQUIRE(std::isnan(profileWithoutLines[i]));
                for(int i = 85; i < profileWithoutLines.size(); i++)
                    REQUIRE(profileWithoutLines[i] == Profile1To100[i]);
            }
        }

        WHEN("CL index smaller than halfMaskLength") {
            std::vector<uint32_t> linesAreas = {10, 70};
            int negativeMargin = linesAreas[0] - maskLength / 2;
            TImageProfile profileWithoutLines = ImageAnalyzer::removeLinesOnProfile(Profile1To100, linesAreas, maskLength);

            THEN("should return profile with NaN values between indexes 0-25 and 55-85") {
                REQUIRE(profileWithoutLines.size() == Profile1To100.size());
                for(int shift = 0, i = shift; i < shift + maskLength + negativeMargin; i++)
                    REQUIRE(std::isnan(profileWithoutLines[i]));
                for(int i = 25; i < 55; i++)
                    REQUIRE(profileWithoutLines[i] == Profile1To100[i]);
                for(int shift = 55, i = shift; i < shift + maskLength; i++)
                    REQUIRE(std::isnan(profileWithoutLines[i]));
                for(int i = 85; i < profileWithoutLines.size(); i++)
                    REQUIRE(profileWithoutLines[i] == Profile1To100[i]);
            }
        }
    }
}