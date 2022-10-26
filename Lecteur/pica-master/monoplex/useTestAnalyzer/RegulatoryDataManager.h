//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/22.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef PICA_REGULATORYDATAMANAGER_H
#define PICA_REGULATORYDATAMANAGER_H

#include "ExcelWriter.h"

class RegulatoryDataManager{
public:
    static void processRegulatoryData(OpenXLSX::XLWorkbook& workbook,
                                      std::vector<ResultsByAlgo> allTLResultsByAlgo);
private:
    static RegulatoryDataByAlgo determineRegulatoryData(ResultsByAlgo resultsByAlgo);
    static void writeAllRegulatoryData(OpenXLSX::XLWorkbook& workbook, const std::vector<RegulatoryDataByAlgo>& allRegulatoryData);
    static std::tuple<double, double> computeMeanAndStandardDeviation(std::vector<double> numbers);
    static RegulatoryData determineOneAlgoRegulatoryData(std::map<double, std::vector<double>> results);
};

#endif //PICA_REGULATORYDATAMANAGER_H
