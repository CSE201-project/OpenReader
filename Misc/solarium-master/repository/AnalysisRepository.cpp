//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/10.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include <fstream>
#include <filesystem>
#include <Poco/Path.h>
#include <Poco/DateTimeFormatter.h>

#include "configure.h"
#include "ReaderDB.h"
#include "AnalysisRepository.h"
#include "tools/FSTools.h"
#include "SolariumApp.h"

namespace fs = std::filesystem;

#ifdef USE_DUMMY_PERIPHERALS
#define REPOSITORY_PATH "./data"
#else
#define REPOSITORY_PATH "/data/analysis"
#endif

#define ANALYSIS_FILENAME "analysis_results.json"

AnalysisRepository::AnalysisRepository(const std::string &repositoryPath)
    : IRepository(!repositoryPath.empty() ? repositoryPath : REPOSITORY_PATH)
{
    // Create Path
    fs::path analysisPath(_repositoryPath);
    fs::create_directories(analysisPath);
}

void AnalysisRepository::save(const CalibrationResult &result)
{
    // Create Path
    fs::path analysisPath(_repositoryPath);
    analysisPath /= "calibration";
    fs::create_directories(analysisPath);

    //
    fs::path analysisFilePath(analysisPath);
    analysisFilePath /= "calibration_results.json";

    std::ofstream analysisFile(analysisFilePath);
    analysisFile << result.toJSON();
    analysisFile.close();

    fs::path stripPicturePath(analysisPath);
    stripPicturePath /= CROPPED_STRIP_FILENAME;

    save(stripPicturePath.string(), result.StripImage);

    fs::path rawPicturePath(analysisPath);
    rawPicturePath /= "raw_picture.png";

    save(rawPicturePath.string(), result.RawImage);
}

void AnalysisRepository::save(const AnalysisResult &result)
{
    // Create Path
    fs::path analysisPath(_repositoryPath);
    analysisPath /= result.Id.toString();
    fs::create_directory(analysisPath);

    //
    fs::path analysisFilePath(analysisPath);
    analysisFilePath /= ANALYSIS_FILENAME;

    std::ofstream analysisFile(analysisFilePath);
    analysisFile << result.toJSON();
    analysisFile.close();

    fs::path stripPicturePath(analysisPath);
    stripPicturePath /= CROPPED_STRIP_FILENAME;

    save(stripPicturePath.string(), result.StripImage);

    fs::path rawPicturePath(analysisPath);
    rawPicturePath /= "raw_picture.png";

    save(rawPicturePath.string(), result.RawImage);

    fs::path pdfReportPath(analysisPath);
    pdfReportPath /= REPORT_FILENAME;
    save(pdfReportPath.string(), result.PdfReport);

    ReaderDB db(_repositoryPath);
    db.insertAnalysis(result);
}

void AnalysisRepository::save(const std::string &path, const std::vector<uint8_t> &imageData)
{
    fs::create_directories("/tmp/Solarium");
    std::ofstream rawPicture(path);
    rawPicture.write((const char *)imageData.data(), imageData.size());
    rawPicture.close();
}

void AnalysisRepository::deleteAnalysis(const std::string &id)
{
    ReaderDB db(_repositoryPath);
    Poco::Path folderPath(_repositoryPath);

    folderPath.append(id);
    std::filesystem::remove_all(folderPath.toString());
    db.deleteAnalysis(id);
}

PatientInfo AnalysisRepository::getPatient(const std::string &id)
{
    ReaderDB db(_repositoryPath);
    PatientInfo patient = db.getPatient(id);
    return patient;
}

void AnalysisRepository::deletePatient(const std::string &patientId)
{
    ReaderDB db(_repositoryPath);

    if (auto patient = db.getPatient(patientId); patient.Id.empty())
        return;

    for (auto & analysis : getAnalysis(patientId))
    {
        fs::path analysisPath(_repositoryPath);
        analysisPath /= analysis.Id.toString();
        fs::remove_all(analysisPath);
    }
    db.deletePatient(patientId);
}

std::vector<AnalysisResult> AnalysisRepository::getAnalysis(const std::string &id)
{
    ReaderDB db(_repositoryPath);
    std::vector<AnalysisResult> analysisList = db.getAnalysis(id);
    return analysisList;
}

std::vector<PatientInfo> AnalysisRepository::searchPatient(const std::string &query, int limit, int offset)
{
    ReaderDB db(_repositoryPath);
    std::vector<PatientInfo> patientList = db.searchPatient(query, limit, offset);
    return patientList;
}

void AnalysisRepository::ensureDataBaseExists()
{
    std::string rootFolder = REPOSITORY_PATH;
    auto dbPath = ReaderDB::getDBPath(rootFolder);
    if (!fs::exists(dbPath))
        ReaderDB::initializeEmptyDB(rootFolder);
}

void AnalysisRepository::initDataBaseFromRepository()
{
    std::string repositoryPath = REPOSITORY_PATH;
    fs::remove(ReaderDB::getDBPath(repositoryPath));

    ReaderDB::initializeEmptyDB(repositoryPath);
    ReaderDB db(repositoryPath);
    Poco::JSON::Parser parser;

    int nbAnalysisInRepository = 0;
    for (const auto &entry : fs::directory_iterator(repositoryPath))
    {
        if (fs::is_directory(entry.path()))
            nbAnalysisInRepository++;
    }

    LOGGER.information("Found %d analysis in repository", nbAnalysisInRepository);

    for (const auto &entry : fs::directory_iterator(repositoryPath))
    {
        auto filePath = entry.path();
        if (fs::is_directory(filePath))
        {
            filePath /= ANALYSIS_FILENAME;
            try {
                auto jsonString = FSTools::getFileContent(filePath.string());
                auto jsonObject = parser.parse(jsonString);
                Poco::JSON::Object::Ptr objectPtr = jsonObject.extract<Poco::JSON::Object::Ptr>();

                AnalysisResult result = AnalysisResult::fromJSON(objectPtr);
                db.insertAnalysis(result);
            }
            catch (const std::exception & ex) {
                LOGGER.error("Error loading file %s : %s", filePath.string(), std::string(ex.what()));
            }

            nbAnalysisInRepository--;
            if (nbAnalysisInRepository % 10 == 0 && nbAnalysisInRepository > 0)
                LOGGER.information("%d analysis remaining to load", nbAnalysisInRepository);
        }
    }
    LOGGER.information("Database rebuild");
}

std::vector<std::string> AnalysisRepository::exportAllAnalysis()
{
    std::vector<std::string> analysisCsv;
    Poco::JSON::Parser parser;
    Poco::DateTimeFormatter dateTimeFormatter;

    analysisCsv.push_back("Date;PatientId;StripType;Result;a1;a2;a3;a4");

    auto extractAlgo = [](const std::vector<AnalysisResult::TAlgoResult> & allResults, uint32_t idx) -> std::string {
        if (allResults.size() >= idx)
            return "";

        auto algo = allResults.at(idx);
        try {
            return std::to_string(algo.Value);
        } catch (const std::exception &) {
            return "";
        }
    };

    for (const auto &entry : fs::directory_iterator(REPOSITORY_PATH))
    {
        auto filePath = entry.path();
        if (fs::is_directory(filePath))
        {
            filePath /= ANALYSIS_FILENAME;
            try {
                auto jsonString = FSTools::getFileContent(filePath.string());
                auto jsonObject = parser.parse(jsonString);
                Poco::JSON::Object::Ptr objectPtr = jsonObject.extract<Poco::JSON::Object::Ptr>();
                AnalysisResult result = AnalysisResult::fromJSON(objectPtr);

                std::ostringstream oss;
                oss << dateTimeFormatter.format(result.Date, Poco::DateTimeFormat::SORTABLE_FORMAT);
                oss << ";" << result.PatientId;
                oss << ";" << result.StripType;
                oss << ";" << std::to_string(result.Result);
                oss << ";" << extractAlgo(result.AllResults, 0);
                oss << ";" << extractAlgo(result.AllResults, 1);
                oss << ";" << extractAlgo(result.AllResults, 2);
                oss << ";" << extractAlgo(result.AllResults, 3);
                analysisCsv.push_back(oss.str());
            }
            catch (const std::exception & ex) {

            }
        }
    }

    return analysisCsv;
}

AnalysisResult AnalysisRepository::getAnalysisById(const std::string &id)
{
    fs::path analysisFolder(_repositoryPath);
    analysisFolder /= id;
    if (!fs::exists(analysisFolder))
        return AnalysisResult();

    Poco::JSON::Parser parser;
    try {
        fs::path filePath(analysisFolder);
        filePath /= ANALYSIS_FILENAME;

        auto jsonString = FSTools::getFileContent(filePath.string());
        auto jsonObject = parser.parse(jsonString);
        Poco::JSON::Object::Ptr objectPtr = jsonObject.extract<Poco::JSON::Object::Ptr>();

        AnalysisResult result = AnalysisResult::fromJSON(objectPtr);

        fs::path stripPicturePath(analysisFolder);
        stripPicturePath /= CROPPED_STRIP_FILENAME;

        result.StripImage = FSTools::getBinaryContentAsUInt8(stripPicturePath.string());

        return result;
    }
    catch (const std::exception & ex) {
        LOGGER.error("Error loading file at %s : %s", analysisFolder.string(), std::string(ex.what()));
    }

    return AnalysisResult();
}
