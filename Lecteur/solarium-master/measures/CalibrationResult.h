//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/10.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_CALIBRATIONRESULT_H
#define SOLARIUM_CALIBRATIONRESULT_H

#include <vector>
#include <Poco/UUID.h>
#include <Poco/Timestamp.h>

class CalibrationResult {
public:
    Poco::UUID Id;
    Poco::Timestamp Date;
    std::string StripType;
    std::vector<uint8_t> StripImage;
    std::vector<uint8_t> RawImage;
    std::vector<double> StripProfileRed;
    std::vector<double> StripProfileGreen;
    std::vector<double> StripProfileBlue;
    double Result = 0;
    std::string Status;

    CalibrationResult();
    CalibrationResult(
            std::string stripType,
            std::vector<uint8_t> stripImage,
            double result);

    std::string toJSON() const;
};


#endif //SOLARIUM_ANALYSISRESULT_H
