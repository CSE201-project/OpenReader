//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/06.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_STRIPANALYSIS_H
#define SOLARIUM_STRIPANALYSIS_H

#include <string>
#include <Poco/Timestamp.h>
#include <Poco/UUID.h>
#include <map>
#include <vector>

class StripAnalysis {
private:
    Poco::UUID _uuid;
    std::string _patientId;
    std::string _stripType;
    Poco::Timestamp _timestamp;
    std::vector<double> _stripProfile;

public:
    void setStripProfile(const std::vector<double> &stripProfile);

public:
    StripAnalysis(const std::string &patientId, const std::string &stripType);
    const Poco::UUID &getUuid() const;

    void setAnalysisFile(std::string uuid);

    std::map<std::string, std::string> getResults() const;
};


#endif //SOLARIUM_STRIPANALYSIS_H
