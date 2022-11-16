#include "ReaderDB.h"

#include <sqlite3.h>
#include <Poco/JSON/Parser.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>

#include <sstream>
#include <fstream>
#include <exception>
#include <filesystem>

#include "SolariumApp.h"
#include "tools/FSTools.h"

#define DB_FILENAME "lmx-reader.db"
#define ANALYSIS_FILENAME "analysis_results.json"

using namespace std;
namespace fs = std::filesystem;

const std::string sqlCommandCreateDB = R"(BEGIN TRANSACTION;
DROP TABLE IF EXISTS "Analysis";
CREATE TABLE IF NOT EXISTS "Analysis" (
	"Id"	TEXT NOT NULL,
	"Date"	TEXT,
	"PatientId"	TEXT,
	"StripType"	TEXT,
	"Result"	REAL,
	"ResultMessage"	TEXT,
	"AllResults"	TEXT,
	"Status"	TEXT
);
DROP TABLE IF EXISTS "Patients";
CREATE TABLE IF NOT EXISTS "Patients" (
	"Id"	TEXT NOT NULL,
	"Date"	TEXT
);
COMMIT;)";



ReaderDB::ReaderDB(const string &folderPath)
{
    fs::path dbPath(getDBPath(folderPath));
    if (sqlite3_open(dbPath.string().c_str(), &_db) != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error: can't open DataBase";
        throw runtime_error(oss.str());
    }
    _dbPath = dbPath.string();
}

ReaderDB::~ReaderDB()
{
    sqlite3_close(_db);
}

int _selectAnalysisCallback(void *pData, int numFields, char **pFields, char **pColNames)
{
    vector<AnalysisResult> *records = static_cast<vector<AnalysisResult> *>(pData);
    try
    {
        if (numFields < 6) {
            throw std::runtime_error("invalid");
        }

        AnalysisResult analysis(pFields[0], pFields[1], pFields[2], pFields[3], pFields[4], pFields[5]);

        records->push_back(analysis);
    }
    catch (const std::exception &ex)
    {
        poco_debug_f1(LOGGER, "Select CB : ", ex.what());
        return 1;
    }
    return 0;
}

int _selectPatientCallback(void *pData, int numFields, char **pFields, char **pColNames)
{
    vector<PatientInfo> *records = static_cast<vector<PatientInfo> *>(pData);
    try
    {
        if (numFields < 3) {
            throw std::runtime_error("invalid numfields");
        }
        PatientInfo patient(pFields[0], pFields[1], pFields[2]);
        records->push_back(patient);
    }
    catch (const std::exception &ex)
    {
        poco_debug_f1(LOGGER, "Select Patient CB : ", ex.what());
        return 1;
    }
    return 0;
}

bool ReaderDB::_patientExist(const string &patientId)
{
    auto patient = getPatient(patientId);
    return !patient.Id.empty();
}

void ReaderDB::_insertPatient(const string &patientId)
{
    char *errmsg;
    string dateString = Poco::DateTimeFormatter::format(Poco::Timestamp(), Poco::DateTimeFormat::ISO8601_FORMAT);
    string stmt = "INSERT INTO Patients( Id, Date ) "
                  "VALUES('" + patientId + "', '" + dateString + "')";

    int ret = sqlite3_exec(_db, stmt.c_str(), nullptr, nullptr, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in INSERT statement: " << stmt << " [" << errmsg << "]";
        throw runtime_error(oss.str());
    }
}

void ReaderDB::_deleteAllPatientAnalysis(const string &patientId)
{
    char *errmsg;
    string stmt = "DELETE FROM Analysis WHERE PatientId = '" + patientId + "'";

    int ret = sqlite3_exec(_db, stmt.c_str(), nullptr, nullptr, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in DELETE statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
}

void ReaderDB::deletePatient(const string &patientId)
{
    _deleteAllPatientAnalysis(patientId);

    char *errmsg;
    string stmt = "DELETE FROM Patients WHERE Id = '" + patientId + "'";

    int ret = sqlite3_exec(_db, stmt.c_str(), nullptr, nullptr, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in DELETE statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
}

void ReaderDB::deleteAnalysis(const string &analysisId)
{
    char *errmsg;
    string stmt = "DELETE FROM Analysis WHERE Id = '" + analysisId + "'";

    int ret = sqlite3_exec(_db, stmt.c_str(), nullptr, nullptr, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in DELETE statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
}

void ReaderDB::insertAnalysis(AnalysisResult analysis)
{
    if (!_patientExist(analysis.PatientId))
        _insertPatient(analysis.PatientId);

    char *errmsg;
    string dateString = Poco::DateTimeFormatter::format(analysis.Date, Poco::DateTimeFormat::ISO8601_FORMAT);

    double result = analysis.AllResults.size() ? analysis.AllResults.at(0).Value : analysis.Result;

    string stmt = "INSERT INTO Analysis( Id, Date, PatientId, StripType, Result, Status ) "
                  "VALUES('" + analysis.Id.toString() + "', '" + dateString + "', '" + analysis.PatientId + "', '" + analysis.StripType + "', " + to_string(result) + ", '" + analysis.Status + "')";

    int ret = sqlite3_exec(_db, stmt.c_str(), nullptr, nullptr, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in INSERT statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
}

vector<AnalysisResult> ReaderDB::getAnalysis(const string &patientId)
{
    char *errmsg;
    vector<AnalysisResult> records;
    string stmt = "SELECT a.Id, a.Date, a.PatientId, a.StripType, a.Result, a.Status FROM Analysis a "
                  "WHERE a.PatientId = '" + patientId + "' ORDER BY a.Date DESC;";

    int ret = sqlite3_exec(_db, stmt.c_str(), _selectAnalysisCallback, &records, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in SELECT statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
    return records;
}

PatientInfo ReaderDB::getPatient(const string &patientId)
{
    char *errmsg;
    vector<PatientInfo> records;
    PatientInfo test;
    string stmt = "SELECT p.*, count(a.id) NbAnalysis "
                  "FROM Patients p LEFT JOIN Analysis a on p.Id = a.PatientId "
                  "WHERE p.Id = '" + patientId + "' GROUP BY p.Id";

    int ret = sqlite3_exec(_db, stmt.c_str(), _selectPatientCallback, &records, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in SELECT statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
    return records.size() > 0 ? records[0] : PatientInfo();
}

vector<PatientInfo> ReaderDB::searchPatient(const string &query, int limit, int offset)
{
    char *errmsg;
    vector<PatientInfo> records;
    std::ostringstream oss;
    oss << "SELECT p.*, count(a.id) NbAnalysis ";
    oss << "FROM Patients p LEFT JOIN Analysis a on p.Id = a.PatientId ";
    if (!query.empty())
        oss << "WHERE p.Id LIKE '%" << query << "%' ";
    oss << "GROUP BY p.Id ORDER BY a.Date DESC ";
    oss << "LIMIT '" << to_string(limit) << "' OFFSET '" << to_string(offset) << "'";

    std::string stmt = oss.str();

    int ret = sqlite3_exec(_db, stmt.c_str(), _selectPatientCallback, &records, &errmsg);

    if (ret != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error in SELECT statement: " << stmt << " [" << errmsg << "]\n";
        throw runtime_error(oss.str());
    }
    return records;
}

void ReaderDB::initializeEmptyDB(const string &repositoryPath)
{
    auto dbPath = getDBPath(repositoryPath);

    sqlite3 *db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
    {
        ostringstream oss;
        oss << "Error: can't create DataBase at " << dbPath;
        throw runtime_error(oss.str());
    }

    sqlite3_exec(db, sqlCommandCreateDB.c_str(), NULL, NULL, NULL);
    sqlite3_close(db);
}

std::string ReaderDB::getDBPath(const string &rootFolder)
{
    fs::path dbPath(rootFolder);
    dbPath /= DB_FILENAME;
    return dbPath.string();
}
