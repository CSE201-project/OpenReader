#include "PatientInfo.h"
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeParser.h>

#include <sstream>
#include <iostream>

PatientInfo::PatientInfo()
{
}

PatientInfo::PatientInfo(std::string id, std::string date, std::string nbAnalysis)
    : Id(id)
    , NbAnalysis(std::atoi(nbAnalysis.c_str()))
{
    int timeZoneDifferential;
    auto dateTime = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, date, timeZoneDifferential);
    CreationDate = dateTime.timestamp();
}

PatientInfo::PatientInfo(std::string nbAnalysis)
    : NbAnalysis(std::atoi(nbAnalysis.c_str()))
{
}

std::string PatientInfo::toJSON() const
{
        std::ostringstream oss;

        oss << "{\"id\":\"" << Id << "\",";
        oss << "\"nb_analysis\":\"" << NbAnalysis << "\",";
        oss << "\"date\":\"" << Poco::DateTimeFormatter::format(CreationDate, Poco::DateTimeFormat::ISO8601_FORMAT) << "\"}";
        return oss.str();
}