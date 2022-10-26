//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/10.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_ANALYSISRESULT_H
#define SOLARIUM_ANALYSISRESULT_H

#include <vector>
#include <Poco/UUID.h>
#include <Poco/Timestamp.h>
#include <Poco/JSON/Parser.h>

#define DEFAULT_BATCH "B2101_001"

class AnalysisResult
{
public:
    Poco::UUID Id;
    Poco::Timestamp Date;
    std::string PatientId;
    std::string StripType;
    std::string StripBatchId;
    std::vector<uint8_t> StripImage;
    std::vector<uint8_t> RawImage;
    std::vector<double> StripProfile;
    std::vector<uint8_t> PdfReport;
    double Result = 0;
    uint32_t ControlLinePos;
    uint32_t TestLinePos;

    struct TConcentration {
        double p1;
        double p2;
        std::string level;
    };
    struct TAlgoResult {
        std::string Name;
        double Value;
        double CutOff;
        std::vector<double> Profile;
        TConcentration Quantity;
    };
    std::vector<TAlgoResult> AllResults;
    std::string Status;

    AnalysisResult();
    AnalysisResult(
        std::string stripType,
        std::string date,
        std::vector<uint8_t> stripImage,
        std::vector<double> stripProfile,
        double result);
    AnalysisResult(
        std::string id,
        std::string date,
        std::string patientId,
        std::string stripType,
        std::string result,
        std::string status);

    void setDate(const std::string & dateISOString);

    std::string toJSON() const;
    static AnalysisResult fromJSON(const Poco::JSON::Object::Ptr &objectPtr);
    // AnalysisResult loadFromFile(const std::filesystem::path &filePath);
};

#endif //SOLARIUM_ANALYSISRESULT_H
