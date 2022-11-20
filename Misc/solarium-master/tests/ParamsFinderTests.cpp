//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/15.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include <sstream>
#include "catch2/catch.hpp"
#include "../measures/ParamsFinder.h"
#include <opencv2/opencv.hpp>
#include "TestTools.h"

void testCombination(const ICamera::TCameraParams & params, int nbParams, std::string correctValues)
{
    REQUIRE(params.size() == nbParams);
    std::ostringstream oss;
    for (const auto & kv : params)
        oss << kv.first << ":" << kv.second << ";";

    REQUIRE(oss.str() == correctValues);
};

SCENARIO("ParamsFinder", "[params]") {
    GIVEN("a list of values to combine") {

        WHEN("2 elems of 1 value each")
        {
            std::map<std::string, std::vector<int>> inputParams = {
                    {"a", {1}},
                    {"b", {2}}
            };

            auto results = computeAcquisitionParamsCombination(inputParams);
            THEN("the resulting vector should contain 1 combination") {
                REQUIRE(results.size() == 1);
            }
        }

        WHEN("1 elem of 1 value and 1 elems of 2 values")
        {
            std::map<std::string, std::vector<int>> inputParams = {
                    {"a", {1}},
                    {"b", {2,3}}
            };

            auto results = computeAcquisitionParamsCombination(inputParams);
            THEN("the resulting vector should contain 2 combinations") {
                REQUIRE(results.size() == 2);
                testCombination(results.at(0), inputParams.size(), "a:1;b:2;");
                testCombination(results.at(1), inputParams.size(), "a:1;b:3;");
            }
        }

        WHEN("2 elems of 2 value each")
        {
            std::map<std::string, std::vector<int>> inputParams = {
                    {"a", {1,2}},
                    {"b", {3,4}}
            };

            auto results = computeAcquisitionParamsCombination(inputParams);
            THEN("the resulting vector should contain 4 combinations") {
                REQUIRE(results.size() == 4);
            }
        }

        WHEN("1 elem of 1 value and 2 elems of 2 values")
        {
            std::map<std::string, std::vector<int>> inputParams = {
                    {"a", {1}},
                    {"b", {2,4}},
                    {"c", {10,20}}
            };

            auto results = computeAcquisitionParamsCombination(inputParams);
            THEN("the resulting vector should contain 4 combinations") {
                REQUIRE(results.size() == 4);
                testCombination(results.at(0), inputParams.size(), "a:1;b:2;c:10;");
                testCombination(results.at(1), inputParams.size(), "a:1;b:2;c:20;");
                testCombination(results.at(2), inputParams.size(), "a:1;b:4;c:10;");
                testCombination(results.at(3), inputParams.size(), "a:1;b:4;c:20;");
            }
        }
    }
}


SCENARIO("ParamsFinder Draw ROI", "[params roi]") {
    GIVEN("A red Channel image") {

        auto redChannelImage = cv::imread(getResourcePath("redChannel.png"), cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

        WHEN("Drawing CL and TL ROIs") {

            auto rgbImage = drawCTAndTLROI(redChannelImage, 139.66, 501, 1363);

            THEN("Picture should be in RGB") {

                REQUIRE(!rgbImage.empty());
                REQUIRE(rgbImage.channels() == 3);
                cv::imwrite("rgbImageROI.png", rgbImage);
            }
        }
    }
}
