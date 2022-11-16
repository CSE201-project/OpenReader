
#include "ROCCurveManager.h"
#include "monoplex/algos/IImageProcessor.h"

#include <algorithm>

void ROCCurveManager::processROCCurves(OpenXLSX::XLWorkbook& workbook,
                                       ResultsByAlgo resultsByAlgoT2)
{
    ROCCurveValues rocCurveValues = determineROCCurveValues(resultsByAlgoT2);
    writeAllAlgosROCCurves(workbook, rocCurveValues);
}

std::tuple<double, double> ROCCurveManager::searchMinAndMaxResults(std::map<double, std::vector<double>> algoResults)
{
    double min, max;
    for(auto& [concentration, results] : algoResults)
    {
        auto [minPtr, maxPtr] = std::minmax_element(results.begin(), results.end());
        if(concentration < 0.001)
        {
            min = *minPtr;
            max = *maxPtr;
        }
        else
        {
            if(*minPtr < min)
                min = *minPtr;
            if(*maxPtr > max)
                max = *maxPtr;
        }
    }
    return {min, max};
}

std::vector<ROCCurveParams> ROCCurveManager::determineOneAlgoROCCurveValues(std::map<double, std::vector<double>> algoResults)
{
    std::vector<ROCCurveParams> rocCurveParams;

    auto [min, max] = searchMinAndMaxResults(algoResults);
    double stepCutoff = (max - min) / 1000.;

    for(double cutoff = min; cutoff < max; cutoff+=stepCutoff)
    {
        ROCCurveParams params = {
          .cutoff = cutoff,
          .TruePositive = 0,
          .TrueNegative = 0,
          .FalsePositive = 0,
          .FalseNegative = 0
        };

        for(auto& [concentration, results] : algoResults)
        {
            for(auto& res : results)
            {
                if(res <= cutoff)
                {
                    if(concentration < 0.001)
                        params.TrueNegative++;
                    else
                        params.FalseNegative++;
                }
                else
                {
                    if(concentration < 0.001)
                        params.FalsePositive++;
                    else
                        params.TruePositive++;
                }
            }
        }
        rocCurveParams.push_back(params);
    }

    return rocCurveParams;
}

ROCCurveValues ROCCurveManager::determineROCCurveValues(ResultsByAlgo resultsByAlgo)
{
    ROCCurveValues rocCurveValues;

    rocCurveValues.brutValues = determineOneAlgoROCCurveValues(resultsByAlgo.brutResults);
    rocCurveValues.expoFitValues = determineOneAlgoROCCurveValues(resultsByAlgo.expoFitResults);
    rocCurveValues.poly2Values = determineOneAlgoROCCurveValues(resultsByAlgo.poly2Results);
    rocCurveValues.testMinusNoiseValues = determineOneAlgoROCCurveValues(resultsByAlgo.testMinusNoiseResults);
    rocCurveValues.convolBrutValues = determineOneAlgoROCCurveValues(resultsByAlgo.convolBrutResults);
    rocCurveValues.convolExpoFitValues = determineOneAlgoROCCurveValues(resultsByAlgo.convolExpoFitResults);
    rocCurveValues.convolPoly2Values = determineOneAlgoROCCurveValues(resultsByAlgo.convolPoly2Results);
    rocCurveValues.convolTestMinusNoiseValues = determineOneAlgoROCCurveValues(resultsByAlgo.convolTestMinusNoiseResults);

    return rocCurveValues;
}

void ROCCurveManager::writeAllAlgosROCCurves(OpenXLSX::XLWorkbook& workbook, const ROCCurveValues& rocCurveValues)
{
    int numFirstAlgoFirstCol = 2;
    int spaceBetweenCols = 9;

    writeROCCurveValues(workbook, rocCurveValues.brutValues, numFirstAlgoFirstCol);
    writeROCCurveValues(workbook, rocCurveValues.expoFitValues, numFirstAlgoFirstCol + spaceBetweenCols);
    writeROCCurveValues(workbook, rocCurveValues.poly2Values, numFirstAlgoFirstCol + spaceBetweenCols * 2);
    writeROCCurveValues(workbook, rocCurveValues.testMinusNoiseValues, numFirstAlgoFirstCol + spaceBetweenCols * 3);
    writeROCCurveValues(workbook, rocCurveValues.convolBrutValues, numFirstAlgoFirstCol + spaceBetweenCols * 4);
    writeROCCurveValues(workbook, rocCurveValues.convolExpoFitValues, numFirstAlgoFirstCol + spaceBetweenCols * 5);
    writeROCCurveValues(workbook, rocCurveValues.convolPoly2Values, numFirstAlgoFirstCol + spaceBetweenCols * 6);
    writeROCCurveValues(workbook, rocCurveValues.convolTestMinusNoiseValues, numFirstAlgoFirstCol + spaceBetweenCols * 7);
}