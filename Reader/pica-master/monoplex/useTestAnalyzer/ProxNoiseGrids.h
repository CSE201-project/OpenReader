//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/04/22.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef PICA_PROXNOISEGRIDS_H
#define PICA_PROXNOISEGRIDS_H

#include <numeric>

#include "ExcelWriter.h"
#include "monoplex/algos/AlgoProxNoise.h"
#include "monoplex/algos/AlgoTestMinusProxNoise.h"

class ProxNoiseGrids{
public:
    static AllBestParamsData computeAndWriteAllGrids(std::map<double, std::vector<std::string>>& sortedNamesMap,
                                                     ParamsToAnalyze params,
                                                     AllProfiles& allProfiles,
                                                     OpenXLSX::XLWorkbook& workbook,
                                                     int biggestConcentration,
                                                     bool isLightVersion);
private:
    static std::vector<std::map<ProxNoiseParams, double>> computeAllProxNoiseLOD(std::vector<std::map<ProxNoiseParams, std::map<double, std::vector<double>>>> allDataProxNoiseTable,
                                                                          ParamsToAnalyze params);
    static std::vector<ProxNoiseParams> searchBestProxNoiseParams(std::vector<std::map<ProxNoiseParams, double>> allProxNoiseLOD);

    static std::map<ProxNoiseParams, std::map<double, std::vector<double>>> processProxNoiseValuesForAllParams(ParamsToAnalyze params,
                                                                                                               double concentration,
                                                                                                               uint32_t numTL,
                                                                                                               uint32_t numGridFirstColumn,
                                                                                                               uint32_t numFirstLineFirstGrid,
                                                                                                               std::vector<double>& profile,
                                                                                                               uint32_t controlLine,
                                                                                                               uint32_t distanceCLtoTL,
                                                                                                               std::map<ProxNoiseParams,
                                                                                                               std::map<double, std::vector<double>>> proxNoiseData,
                                                                                                               bool useOnlyTestLine);

    static std::tuple<std::vector<std::map<ProxNoiseParams, double>>,std::vector<ProxNoiseParams>>
    processOneSetOfGrid(std::map<double, std::vector<std::string>>& sortedNamesMap,
                        ParamsToAnalyze params,
                        uint32_t numGridFirstColumn,
                        uint32_t numFirstLineFirstGrid,
                        std::vector<std::vector<double>>& profiles,
                        std::vector<uint32_t>& allCL,
                        std::vector<std::vector<uint32_t>>& allDistanceCltoTL,
                        bool useOnlyTestLine);

    static double computeLOD(std::map<double, std::vector<double>> data, bool computeLODInhibition);

};

#endif //PICA_PROXNOISEGRIDS_H
