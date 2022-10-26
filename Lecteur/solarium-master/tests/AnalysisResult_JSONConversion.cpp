//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/05.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "TestTools.h"
#include "tools/FSTools.h"
#include "measures/AnalysisResult.h"

#include <Poco/JSON/Parser.h>

SCENARIO("AnalysisResult from/to JSON", "[Converter json]") {

    Poco::JSON::Parser parser;

    GIVEN("A valid analysis_results.json") {
        auto analysisResultsPath = getResourcePath("analysis_results.json");
        auto fileContent = FSTools::getFileContent(analysisResultsPath);
        auto jsonObject = parser.parse(fileContent);

        WHEN("loading from file content") {

            auto result = AnalysisResult::fromJSON(jsonObject.extract<Poco::JSON::Object::Ptr>());

            THEN("AnalysisResult should be also valid") {
                REQUIRE(result.Status == "SUCCESS");
            }
        }
    }

    GIVEN("A valid object AnalysisResult") {
        AnalysisResult result;
        result.StripType = "test";
        result.PatientId = "patientId";
        result.Status = "SUCCESS";
        result.Result = 1.3;
        /*
         * std::string Name;
        double Value;
        double CutOff;
        std::vector<double> Profile;
        TConcentration Quantity;
         */
        result.AllResults = {
                {
                    "expo_fit",
                    0.1, 0.2,
                    {0,1,1.2,2},
                    { 1.2, 2.45, "no value"}
                },{
                        "poly2",
                        0.1, 0.2,
                        {0,1,1.2,2},
                        { std::nan("p1"), std::nan("p2"), "no value"}
                }
        };

        WHEN("loading from file content") {

            auto resultJson = result.toJSON();

            auto jsonObject = parser.parse(resultJson);

            WHEN("loading from file content") {

                auto result = AnalysisResult::fromJSON(jsonObject.extract<Poco::JSON::Object::Ptr>());

                THEN("AnalysisResult should be also valid") {
                    REQUIRE(result.Status == "SUCCESS");
                }
            }
        }
    }

}
