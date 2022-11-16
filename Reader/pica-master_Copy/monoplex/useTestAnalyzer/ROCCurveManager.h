#ifndef PICA_ROCCURVEMANAGER_H
#define PICA_ROCCURVEMANAGER_H

#include <vector>
#include <map>

#include "ExcelWriter.h"

struct ResultsByAlgo{
    std::map<double, std::vector<double>> brutResults;
    std::map<double, std::vector<double>> expoFitResults;
    std::map<double, std::vector<double>> poly2Results;
    std::map<double, std::vector<double>> testMinusNoiseResults;
    std::map<double, std::vector<double>> convolBrutResults;
    std::map<double, std::vector<double>> convolExpoFitResults;
    std::map<double, std::vector<double>> convolPoly2Results;
    std::map<double, std::vector<double>> convolTestMinusNoiseResults;
};

struct ROCCurveValues{
    std::vector<ROCCurveParams> brutValues;
    std::vector<ROCCurveParams> expoFitValues;
    std::vector<ROCCurveParams> poly2Values;
    std::vector<ROCCurveParams> testMinusNoiseValues;
    std::vector<ROCCurveParams> convolBrutValues;
    std::vector<ROCCurveParams> convolExpoFitValues;
    std::vector<ROCCurveParams> convolPoly2Values;
    std::vector<ROCCurveParams> convolTestMinusNoiseValues;
};

class ROCCurveManager{
public:
    static void processROCCurves(OpenXLSX::XLWorkbook& workbook, ResultsByAlgo resultsByAlgoT2);
private:
    static void writeAllAlgosROCCurves(OpenXLSX::XLWorkbook& workbook, const ROCCurveValues& rocCurveValues);
    static std::vector<ROCCurveParams> determineOneAlgoROCCurveValues(std::map<double, std::vector<double>> algoResults);
    static ROCCurveValues determineROCCurveValues(ResultsByAlgo resultsByAlgo);
    static std::tuple<double, double> searchMinAndMaxResults(std::map<double, std::vector<double>> algoResults);
};

#endif //PICA_ROCCURVEMANAGER_H
