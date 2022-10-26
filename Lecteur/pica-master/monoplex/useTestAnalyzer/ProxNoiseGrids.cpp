//
// Created by KarimSadki on 20/04/2021.
//

#include <opencv2/opencv.hpp>
#include "ProxNoiseGrids.h"

double ProxNoiseGrids::computeLOD(std::map<double, std::vector<double>> data, bool computeLODInhibition)
{
    std::map<double, double> allMean;

    bool firstConcentrationDone = false;
    double lowestConcentrationMean3sigma = -1;

    for(auto& element : data)
    {
        std::vector<double> currentConcentrationData = element.second;

        currentConcentrationData.erase(
                std::remove_if(currentConcentrationData.begin(), currentConcentrationData.end(),
                               [](const double nb) { return std::isnan(nb); }),
                currentConcentrationData.end());

        double mean = std::accumulate(currentConcentrationData.begin(), currentConcentrationData.end(), 0.0, [](double sum, double currentResult) {
            return sum + currentResult;
        }) / currentConcentrationData.size();

        allMean[element.first] = mean;

        if(!firstConcentrationDone)
        {
            double accumulator = 0;
            for(auto & nb : currentConcentrationData)
                accumulator += std::pow(nb - mean, 2);

            if(computeLODInhibition)
                lowestConcentrationMean3sigma = mean - std::sqrt(accumulator / currentConcentrationData.size()) * 3;
            else
                lowestConcentrationMean3sigma = mean + std::sqrt(accumulator / currentConcentrationData.size()) * 3;
            firstConcentrationDone = true;
        }
    }

    double result = -1;
    for(auto it = allMean.rbegin(); it != allMean.rend(); ++it)
    {
        if(computeLODInhibition)
        {
            if((it->second >= lowestConcentrationMean3sigma) || (std::isnan(it->second)))
                break;
        }
        else
            if((it->second <= lowestConcentrationMean3sigma) || (std::isnan(it->second)))
                break;
        result = it->first;
    }

    return result;
}

std::vector<std::map<ProxNoiseParams, double>> ProxNoiseGrids::computeAllProxNoiseLOD(std::vector<std::map<ProxNoiseParams, std::map<double, std::vector<double>>>> allDataProxNoiseTable,
                                                                                      ParamsToAnalyze params)
{
    bool computeLODInhibition = false;
    if(params.measure.find("Inhibition") != -1)
        computeLODInhibition = true;

    std::vector<std::map<ProxNoiseParams, double>> allLOD;
    for(int i = 0; i < allDataProxNoiseTable.size(); i++)
    {
        std::map<ProxNoiseParams, double> currentTLLOD;
        std::map<ProxNoiseParams, std::map<double, std::vector<double>>> currentTLData = allDataProxNoiseTable[i];
        for(auto& element : currentTLData)
            currentTLLOD[element.first] = computeLOD(element.second, computeLODInhibition);

        allLOD.push_back(currentTLLOD);
    }

    return allLOD;
}

std::vector<ProxNoiseParams> ProxNoiseGrids::searchBestProxNoiseParams(std::vector<std::map<ProxNoiseParams, double>> allProxNoiseLOD)
{
    std::vector<ProxNoiseParams> proxNoiseParams;
    for(auto& testLineGrid : allProxNoiseLOD)
    {
        ProxNoiseParams choosenLODParams = {
                std::tuple<uint32_t , uint32_t>(50, 25),
                std::tuple<double, double>(AlgoProxNoise::DEFAULT_TL_LENGTH_MM, AlgoProxNoise::DEFAULT_NOISE_LENGTH_MM),
                std::tuple<double, double>(0, 0),
                -1,
                1
        };
        double numberOfBestLOD = 0;

        auto completeTestLineGrid = testLineGrid;

        auto it = testLineGrid.begin();
        while (it != testLineGrid.end())
        {
            if(it->second < 0)
                it = testLineGrid.erase(it);
            else
                it++;
        }

        std::vector<ProxNoiseParams> allBestLODParams;
        double bestLOD = std::min_element(testLineGrid.begin(),testLineGrid.end(),
                                          [] (const std::pair<ProxNoiseParams, double>& a, const std::pair<ProxNoiseParams, double>& b)->bool{ return (a.second < b.second) ; } )->second;

        for(auto & LOD : testLineGrid)
        {
            if((LOD.second - bestLOD < 0.0001) && (LOD.second - bestLOD > -0.0001))
            {
                allBestLODParams.push_back(LOD.first);
                numberOfBestLOD++;
            }
        }

        if(allBestLODParams.size() > 0)
        {
            choosenLODParams = allBestLODParams[allBestLODParams.size() / 2];
            choosenLODParams.LODToWrite = bestLOD;
            choosenLODParams.percentageLODToWrite = (numberOfBestLOD / completeTestLineGrid.size()) * 100;
        }

        proxNoiseParams.push_back(choosenLODParams);
    }
    return proxNoiseParams;
}

std::map<ProxNoiseParams, std::map<double, std::vector<double>>>
ProxNoiseGrids::processProxNoiseValuesForAllParams(ParamsToAnalyze params,
                                                   double concentration,
                                                   uint32_t numTL,
                                                   uint32_t numGridFirstColumn,
                                                   uint32_t numFirstLineFirstGrid,
                                                   std::vector<double>& profile,
                                                   uint32_t controlLine,
                                                   uint32_t distanceCLtoTL,
                                                   std::map<ProxNoiseParams, std::map<double, std::vector<double>>> proxNoiseData,
                                                   bool useOnlyTestLine)
{
    uint32_t testLine = controlLine + distanceCLtoTL;

    AlgoProxNoise algoProxNoise;
    AlgoTestMinusProxNoise algoTestMinusProxNoise;

    uint32_t distanceBetweenGrids = (numTL - 1) * 15;

    double stepProxNoise = ImageAnalyzer::DEFAULT_STEP_MM;

    for(double testLineLength = stepProxNoise * 5; testLineLength < stepProxNoise * 25.1; testLineLength += stepProxNoise)
    {
        uint32_t numLine = numFirstLineFirstGrid + distanceBetweenGrids;
        for(double noiseLength = stepProxNoise * 5; noiseLength < stepProxNoise * 15.1; noiseLength += stepProxNoise)
        {
            uint32_t testLineLengthPx = ImageAnalyzer::convertMillimetersToPixels(params.referenceNumberOfPixels, testLineLength);
            uint32_t noiseLengthPx = ImageAnalyzer::convertMillimetersToPixels(params.referenceNumberOfPixels, noiseLength);

            std::tuple<double, double> result;
            if(useOnlyTestLine)
            {
                algoTestMinusProxNoise.setROILengths(testLineLengthPx, noiseLengthPx, noiseLengthPx);
                result = algoTestMinusProxNoise.processImageProfile(profile, { controlLine, testLine});
            }
            else
            {
                algoProxNoise.setROILengths(testLineLengthPx, noiseLengthPx, noiseLengthPx);
                result = algoProxNoise.processImageProfile(profile, { controlLine, testLine});
            }

            ProxNoiseParams paramsProxNoise = {
                    std::tuple<uint32_t , uint32_t>(testLineLengthPx, noiseLengthPx),
                    std::tuple<double, double>(testLineLength, noiseLength),
                    std::tuple<double, double>(numGridFirstColumn, numLine),
                    -1,
                    1
            };

            proxNoiseData[paramsProxNoise][concentration].push_back(std::get<0>(result));
            numLine++;
        }
        numGridFirstColumn++;
    }

    return proxNoiseData;
}

std::tuple<std::vector<std::map<ProxNoiseParams, double>>,std::vector<ProxNoiseParams>>
ProxNoiseGrids::processOneSetOfGrid(std::map<double, std::vector<std::string>>& sortedNamesMap,
                                    ParamsToAnalyze params,
                                    uint32_t numGridFirstColumn,
                                    uint32_t numFirstLineFirstGrid,
                                    std::vector<std::vector<double>>& profiles,
                                    std::vector<uint32_t>& allCL,
                                    std::vector<std::vector<uint32_t>>& allDistanceCltoTL,
                                    bool useOnlyTestLine)
{
    std::vector<std::map<ProxNoiseParams, std::map<double, std::vector<double>>>> proxNoiseTable(allDistanceCltoTL[0].size());
    int acc = 0;

    for(auto& element : sortedNamesMap)
    {
        double concentration = element.first;
        for(auto& name : element.second)
        {
            for(int numTL = 1; numTL <= allDistanceCltoTL[0].size(); numTL++)
            {
                proxNoiseTable[numTL - 1] =
                        processProxNoiseValuesForAllParams(params, concentration, numTL, numGridFirstColumn, numFirstLineFirstGrid, profiles[acc], allCL[acc], allDistanceCltoTL[acc][numTL-1], proxNoiseTable[numTL - 1], useOnlyTestLine);
            }
            acc++;
        }
    }

    std::vector<std::map<ProxNoiseParams, double>> allProxNoiseLOD = computeAllProxNoiseLOD(proxNoiseTable, params);

    return { allProxNoiseLOD, searchBestProxNoiseParams(allProxNoiseLOD) };
}

AllBestParamsData ProxNoiseGrids::computeAndWriteAllGrids(std::map<double, std::vector<std::string>>& sortedNamesMap,
                                                          ParamsToAnalyze params,
                                                          AllProfiles& allProfiles,
                                                          OpenXLSX::XLWorkbook& workbook,
                                                          int biggestConcentration,
                                                          bool isLightVersion)
{

    uint32_t numFirstLineFirstGrid = 77;
    uint32_t numFirstLineFirstGridConvol = 123;
    uint32_t numColFirstGrid = 4;

    auto [allProxNoiseLOD, bestProxNoiseParams] =
    processOneSetOfGrid(sortedNamesMap, params, numColFirstGrid, numFirstLineFirstGrid, allProfiles.rawProfiles,
                        allProfiles.controlLinesNotCuttedPos, allProfiles.distancesCLtoTL, false);
    writeProxNoiseGrids(allProxNoiseLOD, workbook, biggestConcentration);

    auto [allConvolProxNoiseLOD, bestConvolProxNoiseParams] =
    processOneSetOfGrid(sortedNamesMap, params, numColFirstGrid, numFirstLineFirstGridConvol, allProfiles.convolProfiles,
                        allProfiles.controlLinesNotCuttedPos, allProfiles.distancesCLtoTL, false);
    writeProxNoiseGrids(allConvolProxNoiseLOD, workbook, biggestConcentration);

    AllBestParamsData bestParamsData;

    if(!isLightVersion)
    {
        uint32_t numColExpoFit = 28;
        uint32_t numColPoly2 = 52;
        uint32_t numColTestMinusProxNoise = 76;

        auto [allProxNoiseExpoFitLOD, bestProxNoiseExpoFitParams] =
        processOneSetOfGrid(sortedNamesMap, params, numColExpoFit, numFirstLineFirstGrid, allProfiles.expoFitProfilesFitted,
                            allProfiles.controlLinesCuttedPos, allProfiles.distancesCLtoTL, false);
        writeProxNoiseGrids(allProxNoiseExpoFitLOD, workbook, biggestConcentration);

        auto [allProxNoisePoly2LOD, bestProxNoisePoly2Params] =
        processOneSetOfGrid(sortedNamesMap, params, numColPoly2, numFirstLineFirstGrid, allProfiles.poly2ProfilesFitted,
                            allProfiles.controlLinesCuttedPos, allProfiles.distancesCLtoTL, false);
        writeProxNoiseGrids(allProxNoisePoly2LOD, workbook, biggestConcentration);

        auto [allTestMinusProxNoiseLOD, bestTestMinusProxNoiseParams] =
        processOneSetOfGrid(sortedNamesMap, params, numColTestMinusProxNoise, numFirstLineFirstGrid, allProfiles.rawProfiles,
                            allProfiles.controlLinesNotCuttedPos, allProfiles.distancesCLtoTL, true);
        writeProxNoiseGrids(allTestMinusProxNoiseLOD, workbook, biggestConcentration);

        auto [allConvolExpoFitLOD, bestConvolExpoFitParams] =
        processOneSetOfGrid(sortedNamesMap, params, numColExpoFit, numFirstLineFirstGridConvol, allProfiles.convolProfilesExpoFit,
                            allProfiles.convolControlLinesCuttedPos, allProfiles.distancesCLtoTL, false);
        writeProxNoiseGrids(allConvolExpoFitLOD, workbook, biggestConcentration);

        auto [allConvolPoly2LOD, bestConvolPoly2Params] =
        processOneSetOfGrid(sortedNamesMap, params, numColPoly2, numFirstLineFirstGridConvol, allProfiles.convolProfilesPoly2,
                            allProfiles.convolControlLinesCuttedPos, allProfiles.distancesCLtoTL, false);
        writeProxNoiseGrids(allConvolPoly2LOD, workbook, biggestConcentration);

        auto [allConvolTestMinusProxNoiseLOD, bestConvolTestMinusProxNoiseParams] =
        processOneSetOfGrid(sortedNamesMap, params, numColTestMinusProxNoise, numFirstLineFirstGridConvol, allProfiles.convolProfiles,
                            allProfiles.controlLinesNotCuttedPos, allProfiles.distancesCLtoTL, true);
        writeProxNoiseGrids(allConvolTestMinusProxNoiseLOD, workbook, biggestConcentration);

        bestParamsData = {
                .allProxNoiseBestParams = bestProxNoiseParams,
                .allExpoFitBestParams = bestProxNoiseExpoFitParams,
                .allPoly2BestParams = bestProxNoisePoly2Params,
                .allTestMinusNoiseBestParams = bestTestMinusProxNoiseParams,
                .allConvolProxNoiseBestParams = bestConvolProxNoiseParams,
                .allConvolExpoFitBestParams = bestConvolExpoFitParams,
                .allConvolPoly2BestParams = bestConvolPoly2Params,
                .allConvolTestMinusNoiseBestParams = bestConvolTestMinusProxNoiseParams
        };
    }
    else
    {
        bestParamsData = {
                .allProxNoiseBestParams = bestProxNoiseParams,
                .allExpoFitBestParams = {},
                .allPoly2BestParams = {},
                .allTestMinusNoiseBestParams = {},
                .allConvolProxNoiseBestParams = bestConvolProxNoiseParams,
                .allConvolExpoFitBestParams = {},
                .allConvolPoly2BestParams = {},
                .allConvolTestMinusNoiseBestParams = {}
        };
    }

    return bestParamsData;
}