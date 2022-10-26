//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/01.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_REPORTS_H
#define SOLARIUM_REPORTS_H

#define TXT_PATIENT_ID "Patient ID:"
#define TXT_STRIP_TYPE "Strip Type:"
#define TXT_STRIP_BATCH "Strip Batch:"
#define TXT_DATE "Date:"
#define TXT_DEVICE_SERIAL "Device S/N:"
#define TXT_SW_VERSION "Software version:"
#define TXT_RESULT "Result:"
#define TXT_CONCENTRATION1 "Estimate of Concentration of"
#define TXT_CONCENTRATION2 "Nucleocapsid protein:"
#define TXT_POSITIVE "POSITIVE"
#define TXT_NEGATIVE "NEGATIVE"

#define TXT_QUALITY_CONTROL "Internal Quality Control ( For Lumedix use only )"

#define LEFT_BORDER 230

#include <vector>
#include <string>

class AnalysisResult;

class Reports {
public:
    static std::vector<uint8_t> createReport(const AnalysisResult * result = nullptr);

    static std::vector<std::string> createFullExport();
};


#endif //SOLARIUM_REPORTS_H
