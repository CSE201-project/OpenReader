#ifndef SOLARIUM_PATIENTINFO_H
#define SOLARIUM_PATIENTINFO_H

#include <string>
#include <Poco/Timestamp.h>

class PatientInfo
{
public:
    std::string Id;
    Poco::Timestamp CreationDate;
    int NbAnalysis;

    PatientInfo();
    PatientInfo(
        std::string id,
        std::string date,
        std::string nbAnalysis);
    PatientInfo(
        std::string nbAnalysis
    );
    
    std::string toJSON() const;
};

#endif //SOLARIUM_PATIENTINFO_H