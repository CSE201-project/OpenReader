//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/10.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include <Poco/UUIDGenerator.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

#include "CalibrationResult.h"

#include <sstream>
#include <cmath>

CalibrationResult::CalibrationResult()
    : Id(Poco::UUIDGenerator::defaultGenerator().createOne())
{

}

CalibrationResult::CalibrationResult(std::string stripType, std::vector<uint8_t> stripImage, double result)
        : Id(Poco::UUIDGenerator::defaultGenerator().createOne())
        , StripType(stripType)
        , StripImage(stripImage)
        , Result(result)
{

}

void profileToJson(std::ostringstream & oss, std::vector<double> StripProfile) {
    oss<< "[" ;
    if (!StripProfile.empty())
    {
        oss << *StripProfile.begin();
        for (auto elem = StripProfile.begin() + 1; elem != StripProfile.end(); ++elem)
        {
            oss << "," << *elem;
        }
    }
    oss << "]";
}

std::string CalibrationResult::toJSON() const
{
    std::ostringstream oss;

    oss << "{\"id\":\"calibration\",";
    oss << "\"strip_type\":\"" << StripType << "\",";
    oss << "\"analysis_date\":\"" << Poco::DateTimeFormatter::format(Date, Poco::DateTimeFormat::ISO8601_FORMAT) << "\",";
    oss << "\"status\":\"" << Status << "\",";

    oss.precision(6);
    if (!std::isinf(Result) && !std::isnan(Result))
        oss << "\"result\":" << Result << ",";
    else
        oss << "\"result\":\"#err\",";
    oss << "\"profile\":{";
    oss << "\"red\":";
    profileToJson(oss, StripProfileRed);
    oss << ",\"green\":";
    profileToJson(oss, StripProfileGreen);
    oss << ",\"blue\":";
    profileToJson(oss, StripProfileBlue);
    oss << "}}";
    return oss.str();
}
