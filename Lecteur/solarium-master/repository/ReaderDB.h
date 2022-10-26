#ifndef READERDB_H
#define READERDB_H

#include <vector>

#include "../measures/AnalysisResult.h"
#include "../measures/PatientInfo.h"

class sqlite3;

class ReaderDB
{
public:
    ReaderDB(const std::string &folderPath);
    ~ReaderDB();

    void insertAnalysis(AnalysisResult analysis);
    void deletePatient(const std::string &patientId);
    void deleteAnalysis(const std::string &analysisId);
    PatientInfo getPatient(const std::string &patientId);
    std::vector<AnalysisResult> getAnalysis(const std::string &patientId);
    std::vector<PatientInfo> searchPatient(const std::string &query, int limit = 100, int offset = 0);

    static void initializeEmptyDB(const std::string &repositoryPath);
    static std::string getDBPath(const std::string & rootFolder = std::string());

private:
    std::string _dbPath;
    sqlite3 *_db;

    bool _patientExist(const std::string &patientId);
    void _insertPatient(const std::string &patientId);
    void _deleteAllPatientAnalysis(const std::string &patientId);
};

#endif
