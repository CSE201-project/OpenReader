//
// Created by KarimSadki on 20/04/2021.
//

#include <numeric>

#include "RegulatoryDataManager.h"

void RegulatoryDataManager::processRegulatoryData(OpenXLSX::XLWorkbook& workbook,
                                                  std::vector<ResultsByAlgo> allTLResultsByAlgo)
{
    std::vector<RegulatoryDataByAlgo> allRegulatoryData;
    for(auto& TLResults : allTLResultsByAlgo)
        allRegulatoryData.push_back(determineRegulatoryData(TLResults));

    writeAllRegulatoryData(workbook, allRegulatoryData);
}

std::tuple<double, double> RegulatoryDataManager::computeMeanAndStandardDeviation(std::vector<double> numbers)
{
    double accumulator = std::accumulate(numbers.begin(), numbers.end(), 0.);
    double mean = accumulator / numbers.size();

    double squareSum = std::inner_product(numbers.begin(), numbers.end(), numbers.begin(), 0.0);
    double standardDeviation = std::sqrt(squareSum / numbers.size() - mean * mean);

    return {mean, standardDeviation};
}

RegulatoryData RegulatoryDataManager::determineOneAlgoRegulatoryData(std::map<double, std::vector<double>> results)
{

    auto [mean0, sigma0] = computeMeanAndStandardDeviation(results[0]);

    double LOB = mean0 + 1.645 * sigma0;
    double upperLimitLowLevels = LOB * 5;

    std::vector<double> lowLevelConcentrations;
    double LODConcentration = -1, LODValue;

    for (auto it = results.rbegin(); it != results.rend(); it++) {
        double concentration = it->first;
        auto [meanCurrentConcentration, sigmaCurrentConcentration] = computeMeanAndStandardDeviation(it->second);

        if(meanCurrentConcentration > upperLimitLowLevels)
            continue;
        if(meanCurrentConcentration < LOB)
            break;

        double potentialLOD = LOB + 1.645 * sigmaCurrentConcentration;
        if(meanCurrentConcentration > potentialLOD)
        {
            LODConcentration = concentration;
            LODValue = potentialLOD;
        }

        lowLevelConcentrations.push_back(concentration);
    }

    if(lowLevelConcentrations.empty())
        lowLevelConcentrations.push_back(0);

    auto [minPtr, maxPtr] = std::minmax_element(lowLevelConcentrations.begin(), lowLevelConcentrations.end());

    RegulatoryData currentAlgoRegulatoryData = {
            .RegulatoryLOB = LOB,
            .RegulatoryLOD = LODValue,
            .RegulatoryLODConcentration = LODConcentration,
            .lowLevelsConcentrationsBoundaries = {*minPtr, *maxPtr}
    };

    return currentAlgoRegulatoryData;
}

RegulatoryDataByAlgo RegulatoryDataManager::determineRegulatoryData(ResultsByAlgo resultsByAlgo)
{
    RegulatoryDataByAlgo regulatoryDataByAlgo;

    regulatoryDataByAlgo.brut = determineOneAlgoRegulatoryData(resultsByAlgo.brutResults);
    regulatoryDataByAlgo.expoFit = determineOneAlgoRegulatoryData(resultsByAlgo.expoFitResults);
    regulatoryDataByAlgo.poly2 = determineOneAlgoRegulatoryData(resultsByAlgo.poly2Results);
    regulatoryDataByAlgo.testMinusNoise = determineOneAlgoRegulatoryData(resultsByAlgo.testMinusNoiseResults);
    regulatoryDataByAlgo.convolBrut = determineOneAlgoRegulatoryData(resultsByAlgo.convolBrutResults);
    regulatoryDataByAlgo.convolExpoFit = determineOneAlgoRegulatoryData(resultsByAlgo.convolExpoFitResults);
    regulatoryDataByAlgo.convolPoly2 = determineOneAlgoRegulatoryData(resultsByAlgo.convolPoly2Results);
    regulatoryDataByAlgo.convolTestMinusNoise = determineOneAlgoRegulatoryData(resultsByAlgo.convolTestMinusNoiseResults);

    return regulatoryDataByAlgo;
}

void RegulatoryDataManager::writeAllRegulatoryData(OpenXLSX::XLWorkbook& workbook,
                                                   const std::vector<RegulatoryDataByAlgo>& allRegulatoryData)
{
    int firstAlgoNumLine = 7;
    int spaceBetweenTLData = 20;
    for(uint32_t i = 0; i < allRegulatoryData.size(); i++)
    {
        RegulatoryDataByAlgo currentTLData = allRegulatoryData[i];

        writeOneAlgoRegulatoryData(workbook, currentTLData.brut, firstAlgoNumLine + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.expoFit, firstAlgoNumLine + 1 + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.poly2, firstAlgoNumLine + 2 + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.testMinusNoise, firstAlgoNumLine + 3 + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.convolBrut, firstAlgoNumLine + 4 + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.convolExpoFit, firstAlgoNumLine + 5 + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.convolPoly2, firstAlgoNumLine + 6 + i * spaceBetweenTLData);
        writeOneAlgoRegulatoryData(workbook, currentTLData.convolTestMinusNoise, firstAlgoNumLine + 7 + i * spaceBetweenTLData);
    }
}