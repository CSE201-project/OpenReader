#include <filesystem>

#include "catch2/catch.hpp"
#include "tests/multiplex/pattern/resources/DataToTest.h"
#include "tests/TestTools.h"

bool checkAllLandmarks(std::vector<SpotLocation> expectedLandmarks, std::vector<SpotLocation> landmarks)
{
    if(expectedLandmarks.size() != landmarks.size())
        return false;

    for(uint32_t i = 0; i < expectedLandmarks.size(); i++)
    {
        SpotLocation expectedLandmark = expectedLandmarks[i];
        SpotLocation landmark = landmarks[i];

        if((expectedLandmark.point != landmark.point) || (expectedLandmark.colIndexInPattern != landmark.colIndexInPattern))
            return false;
    }

    return true;
}

bool checkPattern(Pattern expectedPattern, Pattern pattern)
{
    if( (expectedPattern.SpotsSpecies.size() != pattern.SpotsSpecies.size()) ||
        (expectedPattern.SpotsTest.size() != pattern.SpotsTest.size()) ||
        (expectedPattern.SpotsCAS.size() != pattern.SpotsCAS.size()) )
        return false;
    else if(expectedPattern.SpotsSpecies != pattern.SpotsSpecies)
        return false;

    for(auto expectedIt = expectedPattern.SpotsCAS.begin(), it = pattern.SpotsCAS.begin();
        expectedIt != expectedPattern.SpotsCAS.end();
        expectedIt++, it++)
    {
        if((expectedIt->first != it->first) || (expectedIt->second != it->second))
            return false;
    }

    for(auto expectedIt = expectedPattern.SpotsTest.begin(), it = pattern.SpotsTest.begin();
        expectedIt != expectedPattern.SpotsTest.end();
        expectedIt++, it++)
    {
        if((expectedIt->first != it->first) || (expectedIt->second != it->second))
            return false;
    }

    return true;
}

SCENARIO("Test PatternManager ", " [PatternManager]") {

    GIVEN("Verification of landmarks initialization") {
        WHEN("File content is correct + proto204 calibration") {

        std::filesystem::path gridsLocationsFilePath(getResourcePath("multiplex/pattern/resources/Grids Locations.txt"));
        double referenceNumberOfPixels = 43.63;

        std::vector<SpotLocation> landmarks = PatternManager::initAllLandmarks(gridsLocationsFilePath, referenceNumberOfPixels);
        bool checkingResult = checkAllLandmarks(expectedLandmarks204, landmarks);

            THEN("should return true") {
                REQUIRE(checkingResult);
            }
        }

        WHEN("File content is correct + proto301 calibration") {
            std::filesystem::path gridsLocationsFilePath(getResourcePath("multiplex/pattern/resources/Grids Locations.txt"));
            double referenceNumberOfPixels = 139.66;

            std::vector<SpotLocation> landmarks = PatternManager::initAllLandmarks(gridsLocationsFilePath, referenceNumberOfPixels);
            bool checkingResult = checkAllLandmarks(expectedLandmarks301, landmarks);

            THEN("should return true") {
                REQUIRE(checkingResult);
            }
        }

        WHEN("File content is correct + wrong calibration") {
            std::filesystem::path gridsLocationsFilePath(getResourcePath("multiplex/pattern/resources/Grids Locations.txt"));
            double referenceNumberOfPixels = 139.66;

            std::vector<SpotLocation> landmarks = PatternManager::initAllLandmarks(gridsLocationsFilePath, referenceNumberOfPixels);
            bool checkingResult = checkAllLandmarks(expectedLandmarks204, landmarks);

            THEN("should return true") {
                REQUIRE(!checkingResult);
            }
        }

        WHEN("File content is wrong + proto204 calibration") {
            std::filesystem::path gridsLocationsFilePath(getResourcePath("multiplex/pattern/resources/pattern.txt"));
            double referenceNumberOfPixels = 43.63;

            std::vector<SpotLocation> landmarks = PatternManager::initAllLandmarks(gridsLocationsFilePath, referenceNumberOfPixels);
            bool checkingResult = checkAllLandmarks({}, {});

            THEN("should return true") {
                REQUIRE(checkingResult);
            }
        }
    }

    GIVEN("Verification of pattern initialisation") {
        WHEN("File content is correct") {
            std::filesystem::path patternFilePath(getResourcePath("multiplex/pattern/resources/pattern.txt"));

            PatternManager patternManager(patternFilePath);
            Pattern expectedPattern = {
                    .SpotsCAS = expectedSpotsCAS,
                    .SpotsTest = expectedSpotsTest,
                    .SpotsSpecies = expectedSpotsSpecies
            };

            bool checkingResult = checkPattern(expectedPattern, patternManager.pattern);

            THEN("should return true") {
                REQUIRE(checkingResult);
            }
            THEN("horizontal and vertical pitch are correct"){
                REQUIRE(patternManager.getHorizontalPitch() < 0.7500001);
                REQUIRE(patternManager.getHorizontalPitch() > 0.7499999);
                REQUIRE(patternManager.getVerticalPitch() < 0.70000001);
                REQUIRE(patternManager.getVerticalPitch() > 0.69999999);
            }
        }
    }
}
