//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/10.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_ANALYSISREPOSITORY_H
#define SOLARIUM_ANALYSISREPOSITORY_H

#define CROPPED_STRIP_FILENAME "cropped_strip.png"

#define REPORT_FILENAME "analysis_report.pdf"

#include "../measures/AnalysisResult.h"
#include "../measures/PatientInfo.h"
#include "../measures/CalibrationResult.h"

#include "IRepository.h"

class AnalysisRepository : public IRepository
{
public:

    void save(const CalibrationResult & result);
    void save(const AnalysisResult &result);
    void save(const std::string &path, const std::vector<uint8_t> &imageData);
    void deleteAnalysis(const std::string &id);
    void deletePatient(const std::string &patientId);
    PatientInfo getPatient(const std::string &id);
    std::vector<AnalysisResult> getAnalysis(const std::string &id);
    AnalysisResult getAnalysisById(const std::string &id);
    std::vector<PatientInfo> searchPatient(const std::string &query, int limit, int offset);
    static void initDataBaseFromRepository();
    static void ensureDataBaseExists();

    static std::vector<std::string> exportAllAnalysis();

    AnalysisRepository(const std::string &repositoryPath = std::string());

};

#endif //SOLARIUM_ANALYSISREPOSITORY_H
