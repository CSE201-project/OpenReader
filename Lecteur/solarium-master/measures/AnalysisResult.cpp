//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/10.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include <Poco/UUIDGenerator.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/Path.h>

#include "AnalysisResult.h"

//#define ANALYSIS_FILENAME "/analysis_results.json"
//#include <cmath>

AnalysisResult::AnalysisResult()
    : Id(Poco::UUIDGenerator::defaultGenerator().createOne())
{

}

AnalysisResult::AnalysisResult(std::string stripType, std::string date, std::vector<uint8_t> stripImage, std::vector<double> stripProfile, double result)
        : Id(Poco::UUIDGenerator::defaultGenerator().createOne())
        , StripType(stripType)
        , StripImage(stripImage)
        , StripProfile(stripProfile)
        , Result(result)
{
    setDate(date);
}

AnalysisResult::AnalysisResult(std::string id, std::string date, std::string patientId, std::string stripType, std::string result, std::string status)
        : Id(id)
        , PatientId(patientId)
        , StripType(stripType)
        , Result(std::atoi(result.c_str()))
        , Status(status)
{
    setDate(date);
}

std::string AnalysisResult::toJSON() const
{
    std::ostringstream oss;

    oss << "{\"id\":\"" << Id.toString() << "\",";
    oss << "\"strip_type\":\"" << StripType << "\",";
    oss << "\"strip_batch_id\":\"" << StripBatchId << "\",";
    oss << "\"analysis_date\":\"" << Poco::DateTimeFormatter::format(Date, Poco::DateTimeFormat::ISO8601_FORMAT) << "\",";
    oss << "\"patient_id\":\"" << PatientId << "\",";
    oss << "\"status\":\"" << Status << "\",";

    oss.precision(6);

    oss << "\"results\":{";
    bool showComma = false;
    for (auto algoResult : AllResults)
    {
        if (!showComma)
            showComma = true;
        else
            oss << ",";

        oss << "\"" << algoResult.Name << "\":{";
        if (!std::isinf(algoResult.Value) && !std::isnan(algoResult.Value))
            oss << "\"result\":" << algoResult.Value << ",";
        else
            oss << "\"result\":\"#err\",";

        if (!std::isinf(algoResult.CutOff) && !std::isnan(algoResult.CutOff))
            oss << "\"cutoff\":" << algoResult.CutOff << ",";
        else
            oss << "\"cutoff\":\"#err\",";

        if (!std::isinf(algoResult.Quantity.p1) && !std::isnan(algoResult.Quantity.p1))
            oss << "\"quantity\":{\"prediction1\":" << algoResult.Quantity.p1 << ",";
        else
            oss << "\"quantity\":{\"prediction1\":null,";

        if (!std::isinf(algoResult.Quantity.p2) && !std::isnan(algoResult.Quantity.p2))
            oss << "\"prediction2\":" << algoResult.Quantity.p2 << ",";
        else
            oss << "\"prediction2\":null,";

        oss << "\"message\":\"" << algoResult.Quantity.level << "\"},";

        oss << "\"profile\":[" ;
        if (!algoResult.Profile.empty())
        {
            oss << *algoResult.Profile.begin();
            for (auto elem = algoResult.Profile.begin() + 1; elem != algoResult.Profile.end(); ++elem)
            {
                oss << "," << *elem;
            }
        }
        oss << "]}";
    }
    oss << "}}";
    return oss.str();
}

double getNullableDouble(const Poco::JSON::Object::Ptr & obj, const std::string & key)
{
    if (obj->isNull(key))
        return std::nan("");
    else
        return obj->getValue<double>(key);
}

AnalysisResult::TAlgoResult algoResultfromJSON(const Poco::JSON::Object::Ptr & allResultsPtr, const std::string & propName)
{
    auto objectPtr = allResultsPtr->getObject(propName);

    AnalysisResult::TAlgoResult result;
    result.Name = propName;
    result.Value = objectPtr->getValue<double>("result");

    auto quantityPtr = objectPtr->getObject("quantity");
    if (!quantityPtr.isNull())
    {
        result.Quantity.p1 = getNullableDouble(quantityPtr, "prediction1");
        result.Quantity.p2 = getNullableDouble(quantityPtr, "prediction2");
        result.Quantity.level = quantityPtr->getValue<std::string>("message");
    }
    for (auto item : *objectPtr->getArray("profile"))
        result.Profile.push_back(item.convert<double>());

    return result;
}

AnalysisResult AnalysisResult::fromJSON(const Poco::JSON::Object::Ptr & objectPtr)
{
    AnalysisResult analysis;

    std::string id = objectPtr->getValue<std::string>("id");
    std::string stripType = objectPtr->getValue<std::string>("strip_type");
    std::string stripBatch = objectPtr->has("strip_batch_id") ? objectPtr->getValue<std::string>("strip_batch_id") : DEFAULT_BATCH;
    std::string date = objectPtr->getValue<std::string>("analysis_date");
    std::string patientId = objectPtr->getValue<std::string>("patient_id");
    std::string status = objectPtr->getValue<std::string>("status");
    std::string result = "0";
    if (objectPtr->has("result"))
        result = objectPtr->getValue<std::string>("result");

    int timeZoneDifferential;
    Poco::DateTime dateTime = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, date, timeZoneDifferential);

    analysis.Id = Poco::UUID(id);
    analysis.Date = dateTime.timestamp();
    analysis.PatientId = "patientTest";
    analysis.StripType = stripType;
    analysis.StripBatchId = stripBatch;
    analysis.Result = std::atof(result.c_str());
    analysis.Status = status;

    if (analysis.Status == "SUCCESS")
    {
        auto allResults = objectPtr->getObject("results");

        for(const auto & propName : allResults->getNames())
            analysis.AllResults.push_back(algoResultfromJSON(allResults, propName));
    }

    return analysis;
}

void AnalysisResult::setDate(const std::string &dateISOString)
{
    int timeZoneDifferential;
    auto dateTime = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FRAC_FORMAT, dateISOString, timeZoneDifferential);
    Date = dateTime.timestamp();
}

