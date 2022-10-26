//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "../tools/Converter.h"

#include <string>
#include <sstream>

SCENARIO("Converter translate objects", "[Converter]") {

    GIVEN("A vector<double>") {
        std::vector<double> v;

        WHEN("the vector is empty") {
            auto res = Converter::toJson(v);

            THEN("the resulting string should be an empty array") {

                REQUIRE(res.size() > 0);
                REQUIRE(res == std::string("[]"));
            }
        }
        WHEN("the vector has 1 element") {
            v.push_back(42);
            auto res = Converter::toJson(v);

            THEN("the resulting string should contain 1 element") {
                REQUIRE(v.size() == 1);
                REQUIRE(res == std::string("[42]"));
            }
        }
        WHEN("the vector has several elements") {
            v.push_back(42);
            v.push_back(123.15);
            v.push_back(424242424224);
            auto res = Converter::toJson(v);

            THEN("the resulting string should contain several elements") {
                REQUIRE(v.size() == 3);
                REQUIRE(res == std::string("[42,123.15,424242424224]"));
            }
        }
    }
    GIVEN("a Json Params Stream") {

        WHEN("the stream is empty") {
            std::istringstream iss;
            auto res = Converter::jsonParamsStream2StringsMap(iss);

            THEN("the resulting map should be empty") {

                REQUIRE(res.size() == 0);
            }
        }
        WHEN("the stream contains invalid values") {
            std::istringstream iss("}--");
            auto res = Converter::jsonParamsStream2StringsMap(iss);

            THEN("the resulting map should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("the stream has one json object with one property") {
            std::istringstream iss("{\"coucou\":1}");
            auto res = Converter::jsonParamsStream2StringsMap(iss);

            THEN("the resulting map should contain one element") {
                REQUIRE(res.size() == 1);
                REQUIRE(res["coucou"] == std::string("1"));
            }
        }
        WHEN("the stream has one json list with one item") {
            std::istringstream iss("[\"coucou\"]");
            auto res = Converter::jsonParamsStream2StringsMap(iss);

            THEN("the resulting map should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
    }
    GIVEN("Array of Objects Params") {
        WHEN("string is empty")
        {
            auto res = Converter::jsonArrayParams2StringsMapVector("");
            THEN("the resulting vector should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("string is list of 1 empty object")
        {
            std::string tmp = "[{}]";
            auto res = Converter::jsonArrayParams2StringsMapVector(tmp);
            THEN("the resulting vector has 1 elem") {
                REQUIRE(res.size() == 1);
                REQUIRE(res.at(0).size() == 0);
            }
        }
        WHEN("string is list of 1 valid object")
        {
            std::string tmp = "[{\"valid\":true}]";
            auto res = Converter::jsonArrayParams2StringsMapVector(tmp);
            THEN("the resulting vector has 1 elem") {
                REQUIRE(res.size() == 1);
                REQUIRE(res.at(0).find("valid") != res.at(0).end());
            }
        }
        WHEN("string is list of 2 valid object")
        {
            std::string tmp = "[{\"valid\":true},{\"valid\":\"also_true\"}]";
            auto res = Converter::jsonArrayParams2StringsMapVector(tmp);
            THEN("the resulting vector has 2 elem") {
                REQUIRE(res.size() == 2);
                REQUIRE(res.at(0)["valid"] == std::string("true"));
                REQUIRE(res.at(1)["valid"] == std::string("also_true"));
            }
        }
    }
    GIVEN("Array of Int") {
        WHEN("string is empty")
        {
            auto res = Converter::jsonArray2IntVector("");
            THEN("the resulting vector should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("string is empty array")
        {
            std::string tmp = "[]";
            auto res = Converter::jsonArray2IntVector(tmp);
            THEN("the resulting vector should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("string is list of 1 valid int")
        {
            std::string tmp = "[1]";
            auto res = Converter::jsonArray2IntVector(tmp);
            THEN("the resulting vector has 1 elem") {
                REQUIRE(res.size() == 1);
                REQUIRE(res.at(0) == 1);
            }
        }
        WHEN("string is list of 1 invalid int")
        {
            std::string tmp = "[a]";
            auto res = Converter::jsonArray2IntVector(tmp);
            THEN("the resulting vector should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("string is list of 2 valid int")
        {
            std::string tmp = "[1,-12]";
            auto res = Converter::jsonArray2IntVector(tmp);
            THEN("the resulting vector has 2 elem") {
                REQUIRE(res.size() == 2);
                REQUIRE(res.at(0) == 1);
                REQUIRE(res.at(1) == -12);
            }
        }
    }
    GIVEN("ParamsFinder Params") {
        WHEN("string is empty")
        {
            auto res = Converter::jsonObjectToParamsValues("");
            THEN("the resulting map should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("string is empty array")
        {
            std::string tmp = "[]";
            auto res = Converter::jsonObjectToParamsValues(tmp);
            THEN("the resulting map should be empty") {
                REQUIRE(res.size() == 0);
            }
        }
        WHEN("string is 1 valid key with no values")
        {
            std::string tmp = "{\"key\":null}";
            auto res = Converter::jsonObjectToParamsValues(tmp);
            THEN("the resulting map has 1 elem without values") {
                REQUIRE(res.size() == 1);
                REQUIRE(res["key"].size() == 0);
            }
        }
        WHEN("string is 1 valid key with invalid values")
        {
            std::string tmp = "{\"key\":12}";
            auto res = Converter::jsonObjectToParamsValues(tmp);
            THEN("the resulting map has 1 elem without values") {
                REQUIRE(res.size() == 1);
                REQUIRE(res["key"].size() == 0);
            }
        }
        WHEN("string is 1 valid key with valid values")
        {
            std::string tmp = "{\"key\":[12]}";
            auto res = Converter::jsonObjectToParamsValues(tmp);
            THEN("the resulting map has 1 elem with 1 value") {
                REQUIRE(res.size() == 1);
                REQUIRE(res["key"].size() == 1);
            }
        }
        WHEN("string is 2 valid key with valid values")
        {
            std::string tmp = "{\"key\":[12],\"key2\":[12,-2]}";
            auto res = Converter::jsonObjectToParamsValues(tmp);
            THEN("the resulting map has 1 elem with 1 value") {
                REQUIRE(res.size() == 2);
                REQUIRE(res["key"].size() == 1);
                REQUIRE(res["key2"].size() == 2);
            }
        }
    }
}
