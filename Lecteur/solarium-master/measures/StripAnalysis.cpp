//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/06.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "StripAnalysis.h"
#include <Poco/UUIDGenerator.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

StripAnalysis::StripAnalysis(const std::string &patientId, const std::string &stripType)
:   _patientId(patientId),
    _stripType(stripType),
    _timestamp(Poco::Timestamp()),
    _stripProfile()
{
    Poco::UUIDGenerator uuidGenerator;
    _uuid = uuidGenerator.createOne();
}

std::map<std::string, std::string> StripAnalysis::getResults() const
{
    auto results = std::map<std::string, std::string>();
    results["patientID"] = _patientId;
    results["measureID"] = _uuid.toString();
    results["stripType"] = _stripType;

    Poco::DateTimeFormatter dateTimeFormatter;
    results["date"] = dateTimeFormatter.format(_timestamp, Poco::DateTimeFormat::ISO8601_FORMAT);

    return results;
}

const Poco::UUID &StripAnalysis::getUuid() const
{
    return _uuid;
}

void StripAnalysis::setStripProfile(const std::vector<double> &stripProfile)
{
    std::copy(stripProfile.begin(), stripProfile.end(), _stripProfile.begin());
}

