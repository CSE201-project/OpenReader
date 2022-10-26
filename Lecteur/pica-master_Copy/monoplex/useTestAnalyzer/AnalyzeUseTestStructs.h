//
// Created by KarimSadki on 17/12/2020.
//

#ifndef PICA_ANALYZEUSETESTSTRUCTS_H
#define PICA_ANALYZEUSETESTSTRUCTS_H

#include <string>
#include <vector>
#include <filesystem>

#include "common/imageProcessing/ImageAnalyzer.h"

struct ParamsToAnalyze{
    bool isVisible;
    std::string channelStr;
    ColorChannel channel;

    double lengthROIMm;
    double rectangleWidthMm;
    double maskLengthMm;
    uint32_t padding;
    uint32_t rectangleWidth;
    uint32_t maskLength;

    uint32_t minCLSearchArea;
    uint32_t maxCLSearchArea;

    double theoricDistanceCLtoTLMm;
    double TLLengthMm;
    double proxNoiseTLSize;
    double proxNoiseNoiseSize;

    double referenceNumberOfPixels;

    std::string appVersion;
    std::string currentDate;

    std::string measure;
    std::string proto;
    std::string cassette;
    std::string gamme;
    std::string nanoParticules;

    std::filesystem::path croppedFolderPath;
    std::filesystem::path templateExcelFilePath;
    std::filesystem::path excelFilePath;
    std::filesystem::path excelFileCalibrationCurves;
};

struct Results{
    std::tuple<double, double> proxNoise;
    std::tuple<double, double> expoFitProxNoise;
    std::tuple<double, double> poly2ProxNoise;
    std::tuple<double, double> testMinusProxNoise;
    std::tuple<double, double> convolProxNoise;
    std::tuple<double, double> convolExpoFitProxNoise;
    std::tuple<double, double> convolPoly2ProxNoise;
    std::tuple<double, double> convolTestMinusProxNoise;

    std::tuple<double, double> proxNoiseBestParams;
    std::tuple<double, double> expoFitProxNoiseBestParams;
    std::tuple<double, double> poly2ProxNoiseBestParams;
    std::tuple<double, double> testMinusProxNoiseBestParams;
    std::tuple<double, double> convolProxNoiseBestParams;
    std::tuple<double, double> convolExpoFitProxNoiseBestParams;
    std::tuple<double, double> convolPoly2ProxNoiseBestParams;
    std::tuple<double, double> convolTestMinusProxNoiseBestParams;

    double rawControl;
    double rawTest;
    double rawNoise;

    uint32_t numImage;
    uint32_t numTL;
    std::string nameImage;

    bool const operator<(const Results &result) const{
        return (numImage < result.numImage) || ((numImage == result.numImage) && (numTL < result.numTL));
    }
};

struct ProxNoiseParams{
    std::tuple<uint32_t, uint32_t> proxNoiseLenghts;
    std::tuple<double, double> proxNoiseLenghtsMm;
    std::tuple<double, double> localisationInExcel;
    double LODToWrite;
    double percentageLODToWrite;

    bool const operator<(const ProxNoiseParams &params) const{
        return (std::get<0>(proxNoiseLenghtsMm) < std::get<0>(params.proxNoiseLenghtsMm))
               || ((std::get<0>(proxNoiseLenghtsMm) == std::get<0>(params.proxNoiseLenghtsMm))
                   && (std::get<1>(proxNoiseLenghtsMm) < std::get<1>(params.proxNoiseLenghtsMm)));
    }
};

struct CalibrationCurvesCoefficients{
    SigmoideCoefficients proxNoiseBrutSigmoide;
    SigmoideCoefficients expoFitSigmoide;
    SigmoideCoefficients poly2Sigmoide;
    SigmoideCoefficients testMinusProxNoiseSigmoide;
    SigmoideCoefficients convolTestMinusProxNoiseSigmoide;

    LinearRegressionCoefficients proxNoiseBrutLinear;
    LinearRegressionCoefficients expoFitLinear;
    LinearRegressionCoefficients poly2Linear;

    LinearRegressionCoefficients convolBrutLinear;
    LinearRegressionCoefficients convolExpoFitLinear;
    LinearRegressionCoefficients convolPoly2Linear;

    double proxNoiseBrutLinearFitScore;
    double expoFitLinearFitScore;
    double poly2LinearFitScore;

    double convolBrutLinearFitScore;
    double convolExpoFitLinearFitScore;
    double convolPoly2LinearFitScore;
};

struct CalibrationCutoffsValues{
    std::vector<double> proxNoiseCutoffs;
    std::vector<double> expoFitCutoffs;
    std::vector<double> poly2Cutoffs;
    std::vector<double> testMinusProxNoiseCutoffs;

    std::vector<double> convolBrutCutoffs;
    std::vector<double> convolExpoFitCutoffs;
    std::vector<double> convolPoly2Cutoffs;
    std::vector<double> convolTestMinusProxNoiseCutoffs;
};

struct Predictions{
    double realConcentrations;

    double concentrationSigmoideBrut;
    double concentrationSigmoideExpoFit;
    double concentrationSigmoidePoly2;
    double concentrationSigmoideTestMinusProxNoise;
    double concentrationSigmoideConvolTestMinusProxNoise;

    double concentrationLinearBrut;
    double concentrationLinearExpoFit;
    double concentrationLinearPoly2;

    double concentrationLinearConvolBrut;
    double concentrationLinearConvolExpoFit;
    double concentrationLinearConvolPoly2;
};

struct AllBestParamsData{
    std::vector<ProxNoiseParams> allProxNoiseBestParams;
    std::vector<ProxNoiseParams> allExpoFitBestParams;
    std::vector<ProxNoiseParams> allPoly2BestParams;
    std::vector<ProxNoiseParams> allTestMinusNoiseBestParams;

    std::vector<ProxNoiseParams> allConvolProxNoiseBestParams;
    std::vector<ProxNoiseParams> allConvolExpoFitBestParams;
    std::vector<ProxNoiseParams> allConvolPoly2BestParams;
    std::vector<ProxNoiseParams> allConvolTestMinusNoiseBestParams;
};

struct RegulatoryData{
    double RegulatoryLOB;
    double RegulatoryLOD;
    double RegulatoryLODConcentration;
    std::tuple<double, double> lowLevelsConcentrationsBoundaries;
};

struct RegulatoryDataByAlgo{
    RegulatoryData brut;
    RegulatoryData expoFit;
    RegulatoryData poly2;
    RegulatoryData testMinusNoise;

    RegulatoryData convolBrut;
    RegulatoryData convolExpoFit;
    RegulatoryData convolPoly2;
    RegulatoryData convolTestMinusNoise;
};

struct ROCCurveParams{
    double cutoff;
    uint32_t TruePositive;
    uint32_t TrueNegative;
    uint32_t FalsePositive;
    uint32_t FalseNegative;
};

struct AllProfiles{
    std::vector<cv::Mat> croppedImages;

    std::vector<std::vector<double>> rawProfiles;
    std::vector<std::vector<double>> expoFitProfilesFitted;
    std::vector<std::vector<double>> poly2ProfilesFitted;
    std::vector<uint32_t> controlLinesCuttedPos;

    std::vector<std::vector<double>> convolProfiles;
    std::vector<std::vector<double>> convolProfilesExpoFit;
    std::vector<std::vector<double>> convolProfilesPoly2;
    std::vector<uint32_t> convolControlLinesCuttedPos;

    std::vector<uint32_t> controlLinesNotCuttedPos;
    std::vector<std::vector<uint32_t>> distancesCLtoTL;

    std::vector<double> expoFitCoefficients;
    std::vector<double> poly2Coefficients;
    std::vector<double> convolExpoFitCoefficients;
    std::vector<double> convolPoly2Coefficients;
};

#endif //PICA_ANALYZEUSETESTSTRUCTS_H
