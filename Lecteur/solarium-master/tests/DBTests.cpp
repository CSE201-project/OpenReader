//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/28.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "repository/ReaderDB.h"
#include "TestTools.h"

#include <filesystem>

#define DEFAULT_DB_PATH "DBTests.db"
#define CURRENT_DB_PATH "./data/lmx-reader.db"

namespace fs = std::filesystem;

SCENARIO("ReaderDB Database", "[repository db]")
{
    fs::copy(getResourcePath(DEFAULT_DB_PATH), CURRENT_DB_PATH, fs::copy_options::overwrite_existing);

    ReaderDB db("./data");

    GIVEN("Patients Table with Patients Id's")
    {
        WHEN("Patient Id is empty")
        {
            auto records = db.searchPatient("");
            THEN("All Patients should be found")
            {
                REQUIRE(records.size() > 1);
            }
        }
        WHEN("Patient Id is empty 2")
        {
            auto records = db.searchPatient(std::string());
            THEN("All Patients should be found")
            {
                REQUIRE(records.size() > 1);
            }
        }
        WHEN("Patient Id is in DataBase")
        {
            auto records = db.searchPatient("Patient2");
            THEN("PatientId should be found")
            {
                REQUIRE(records.size() == 1);
            }
        }
        WHEN("Patient Id isn\'t in DataBase")
        {
            auto records = db.searchPatient("Patient777");
            THEN("Patient Id shouldn't be found")
            {
                REQUIRE(records.size() == 0);
            }
        }
        WHEN("Patient Id has been deleted")
        {
            db.deletePatient("Patient1");
            THEN("Patient Id shouldn't be found anymore")
            {
                auto records = db.searchPatient("Patient1");
                REQUIRE(records.size() == 0);
            }
            THEN("All patient analysis should have been deleted")
            {
                auto records = db.getAnalysis("Patient1");
                REQUIRE(records.size() == 0);
            }
        }
    }

    GIVEN("Analysis Table with patients analysis")
    {
        WHEN("Patient Id has one analysis in DataBase")
        {
            auto records = db.getAnalysis("Patient2");
            THEN("One analysis should be found")
            {
                REQUIRE(records.size() == 1);
            }
        }
        WHEN("Patient Id has many analysis in DataBase")
        {
            auto records = db.getAnalysis("Patient1");
            THEN("All analysis from Patient Id should be found")
            {
                REQUIRE(records.size() == 3);
            }
        }
        WHEN("Patient Id has no analysis in DataBase")
        {
            auto records = db.getAnalysis("INVALID_ID");
            THEN("No analysis should be found")
            {
                REQUIRE(records.size() == 0);
            }
        }
        WHEN("Analysis has been deleted")
        {
            db.deleteAnalysis("8c49cf84-7a0f-11eb-9439-0242ac130002");
            THEN("Analysis shouldn't be found anymore")
            {
                auto records = db.getAnalysis("Patient1");
                REQUIRE(records.size() == 2);
            }
        }
        WHEN("New analysis is added to existing patient")
        {
            AnalysisResult dummyAnalysis;
            dummyAnalysis.PatientId = "Patient2";

            db.insertAnalysis(dummyAnalysis);
            THEN("New analysis should be found")
            {
                auto records = db.getAnalysis("Patient2");
                REQUIRE(records.size() == 2);
            }
        }
        WHEN("New analysis is added with a new patient Id")
        {
            AnalysisResult dummyAnalysis;
            dummyAnalysis.PatientId = "Patient123";
            db.insertAnalysis(dummyAnalysis);
            THEN("New patient Id should be added to Patients Table")
            {
                auto records = db.searchPatient("Patient123");
                REQUIRE(records.size() == 1);
            }
        }
    }
}
