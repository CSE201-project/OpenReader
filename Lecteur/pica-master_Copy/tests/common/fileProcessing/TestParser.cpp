//
// Created by KarimSadki on 24/09/2021.
//
#include <filesystem>
#include "catch2/catch.hpp"
#include "tests/TestTools.h"
#include "common/fileProcessing/Parser.h"

template<typename T>
bool compareVectors(const std::vector<T>& vector1, const std::vector<T>& vector2)
{
    if(vector1.size() != vector2.size())
        return false;

    for(int i = 0; i < vector1.size(); i++)
    {
        if(vector1[i] != vector2[i])
            return false;
    }
    return true;
}

SCENARIO("Test Parser ", " [parser]") {

    GIVEN("search device name in a json file") {
        WHEN("is a valid json file") {
            std::string filePathStr = getResourcePath("common/fileProcessing/resources/file.json");
            std::filesystem::path filePath(filePathStr);

            std::string deviceName = Parser::findDeviceName(filePath);
            THEN("should return existing device name") {
                REQUIRE(deviceName == "lmx-301");
            }
        }

        WHEN("is a json file without 'device' field") {
            std::string filePathStr = getResourcePath("common/fileProcessing/resources/file2.json");
            std::filesystem::path filePath(filePathStr);

            std::string deviceName = Parser::findDeviceName(filePath);
            THEN("should return empty string") {
                REQUIRE(deviceName == "");
            }
        }

        WHEN("is not a json file") {
            std::string filePathStr = getResourcePath("common/fileProcessing/resources/file.txt");
            std::filesystem::path filePath(filePathStr);

            std::string deviceName = Parser::findDeviceName(filePath);
            THEN("should return empty string") {
                REQUIRE(deviceName == "");
            }
        }
    }

    GIVEN("parse string with delimiter") {
        WHEN("is a string with the expected delimiters") {
            std::string stringToParse = "Hello,my,name is John";

            std::vector<std::string> parsedString1 = Parser::parseString(stringToParse, ",");
            std::vector<std::string> parsedString2 = Parser::parseString(stringToParse, " is");

            std::vector<std::string> expectedResult1 = {"Hello", "my", "name is John"};
            std::vector<std::string> expectedResult2 = {"Hello,my,name", " John"};

            THEN("should return expected results") {
                REQUIRE(compareVectors(parsedString1, expectedResult1));
                REQUIRE(compareVectors(parsedString2, expectedResult2));
                REQUIRE(!compareVectors(parsedString1, expectedResult2));
            }
        }

        WHEN("is a string without correct delimiter") {
            std::string stringToParse = "Hello,my,name is John";

            std::vector<std::string> parsedString1 = Parser::parseString(stringToParse, ", ");
            std::vector<std::string> parsedString2 = Parser::parseString(stringToParse, "");

            std::vector<std::string> expectedResult = {"Hello,my,name is John"};

            THEN("should return expected results") {
                REQUIRE(compareVectors(parsedString1, expectedResult));
                REQUIRE(compareVectors(parsedString2, expectedResult));
            }
        }
    }

    GIVEN("parse string of double with delimiter") {
        WHEN("is a string of double with correct delimiter") {
            std::string stringToParse = "-5-88.6-4.55";

            std::vector<double> parsedString = Parser::parseStringAsDoubles(stringToParse, "-");
            std::vector<double> expectedResult = {5, 88.6, 4.55};

            THEN("should return expected result") {
                REQUIRE(compareVectors(parsedString, expectedResult));
            }
        }

        WHEN("is a string of double with incorrect delimiter") {
            std::string stringToParse = ";5;88.6;4.55";

            std::vector<double> numbers = Parser::parseStringAsDoubles(stringToParse, " ");
            std::vector<double> expectedResult = {};

            THEN("should return expected result") {
                REQUIRE(compareVectors(numbers, expectedResult));
            }
        }
    }

}