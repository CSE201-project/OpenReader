//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/24.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "catch2/catch.hpp"
#include "../repository/StudiesRepository.h"
#include "../measures/StudySnapshotResult.h"

#include <filesystem>

namespace fs = std::filesystem;

SCENARIO("Studies Repository", "[repository]")
{
    std::string currentStudiesPath = "./data/studies";
    StudiesRepository repository(currentStudiesPath);

    std::string studyName = "StudyTest";
    std::string studyDate = "20210101";

    GIVEN("A Study name")
    {
        std::string studyName = "ZeStudy";
        WHEN("Testing a valid folder name belongs to the study")
        {
            std::string folderName = "ZeStudy-p0";
            THEN("Should return true")
            {
                REQUIRE(StudiesRepository::isStudyNameDerived(studyName, folderName));
            }
        }
        WHEN("Testing an invalid folder name belongs to the study")
        {
            std::string folderName = "ZeStudyeza-p0";
            THEN("Should return true")
            {
                REQUIRE_FALSE(StudiesRepository::isStudyNameDerived(studyName, folderName));
            }
        }
        WHEN("Testing an invalid folder name with a similar name")
        {
            std::string folderName = "ZeStudy-pa0";
            THEN("Should return true")
            {
                REQUIRE_FALSE(StudiesRepository::isStudyNameDerived(studyName, folderName));
            }
        }
    }

    GIVEN("A Study 'StudyTest'")
    {
        const StudySnapshotResult defaultSnapshot = {
                .StudyName = studyName,
                .FileName = studyName + "_test",
                .Params = R"({"param1":"value1})",
                .SensorsInfo = R"({"sensor1":"value1})"
        };

        WHEN("Saving a StudySnapshotResult with ParamIndex 0")
        {
            repository.save(studyName, studyDate, 0, defaultSnapshot);

            THEN("A Folder 'StudyTest-p0' should exist and contains the Snapshot")
            {

                fs::path destFolder(currentStudiesPath);
                destFolder /= studyDate;
                destFolder /= studyName + "-p0";

                REQUIRE(fs::exists(destFolder));
                REQUIRE(fs::exists(destFolder / (defaultSnapshot.FileName + ".json")));
                REQUIRE(fs::exists(destFolder / (defaultSnapshot.FileName + ".png")));
            }
        }

        WHEN("Saving a StudySnapshotResult with ParamIndex 1")
        {
            repository.save(studyName, studyDate, 1, defaultSnapshot);

            THEN("A Folder 'StudyTest-p1' should exist and contains the Snapshot")
            {

                fs::path destFolder(currentStudiesPath);
                destFolder /= studyDate;
                destFolder /= studyName + "-p1";

                REQUIRE(fs::exists(destFolder));
                REQUIRE(fs::exists(destFolder / (defaultSnapshot.FileName + ".json")));
                REQUIRE(fs::exists(destFolder / (defaultSnapshot.FileName + ".png")));
            }
        }

        WHEN("Deleting a StudySnapshotResult")
        {
            repository.remove(studyName, studyDate, defaultSnapshot.FileName);

            THEN("A Folder 'StudyTest-p0' should exist and NOT contains the Snapshot")
            {
                fs::path destFolder(currentStudiesPath);
                destFolder /= studyDate;
                destFolder /= studyName + "-p0";

                REQUIRE(fs::exists(destFolder));
                REQUIRE_FALSE(fs::exists(destFolder / (defaultSnapshot.FileName + ".json")));
                REQUIRE_FALSE(fs::exists(destFolder / (defaultSnapshot.FileName + ".png")));
            }
            THEN("A Folder 'StudyTest-p1' should exist and NOT contains the Snapshot")
            {
                fs::path destFolder(currentStudiesPath);
                destFolder /= studyDate;
                destFolder /= studyName + "-p1";

                REQUIRE(fs::exists(destFolder));
                REQUIRE_FALSE(fs::exists(destFolder / (defaultSnapshot.FileName + ".json")));
                REQUIRE_FALSE(fs::exists(destFolder / (defaultSnapshot.FileName + ".png")));
            }
        }
    }


}
