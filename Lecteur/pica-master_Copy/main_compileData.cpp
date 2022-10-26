//
// Created by KarimSadki on 09/12/2020.
//
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>

#ifdef WIN32
    #include <OpenXLSX.hpp>
#endif

#include <fstream>
#include <regex>
#include <numeric>

#include "common/cropper/StripCropperRectangleDetection.h"
#include "monoplex/algos/CurveFitter.h"

void print_usage(const char * applicationName){
    std::cout << "Usage:" << std::endl;
    std::cout << applicationName << " imagePath cropFolder ratioNumberPixelsMm [--Visible] [--Cassette] [--V2] [--12bits]" << std::endl;
}

void print_usage(const wchar_t * applicationName){
    std::cout << "Usage:" << std::endl;
    std::wcout << applicationName << L" imagePath cropFolder ratioNumberPixelsMm [--Visible] [--Cassette] [--V2] [--12bits]" << std::endl;
}

enum Proto {Proto301, Proto304, Proto305, Unknown};
enum Algo {AlgoBrut, AlgoExpoFit, AlgoPoly2, AlgoConvolBrut, AlgoConvolExpoFit, AlgoConvolPoly2};

struct AlgoData{
    std::map<double, std::tuple<double, double>> meansAndSigmaByConcentration;
    double RegulatoryLoB;
    double RegulatoryLoDValue;
    double RegulatoryLoD;
    double LoD;
    double LoDValue;
};

struct Data{
    AlgoData brut;
    AlgoData expoFit;
    AlgoData poly2;
    AlgoData convolBrut;
    AlgoData convolExpoFit;
    AlgoData convolPoly2;
};

struct DataRSquares{
    std::tuple<double, double, double> brut;
    std::tuple<double, double, double> expoFit;
    std::tuple<double, double, double> poly2;
    std::tuple<double, double, double> convolBrut;
    std::tuple<double, double, double> convolExpoFit;
    std::tuple<double, double, double> convolPoly2;
};

Proto searchProtoNumber(const std::filesystem::path& filePath)
{
    std::string name = filePath.filename().string();
    if(name.find("301") != -1)
        return Proto301;
    else if(name.find("304") != -1)
        return Proto304;
    else if(name.find("305") != -1)
        return Proto305;

    return Unknown;
}

void readSummaryData(OpenXLSX::XLWorkbook& workbook, int numLineSummary, AlgoData& algoData)
{
    workbook.setFullCalculationOnLoad();
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Summary");

    algoData.RegulatoryLoDValue = worksheet.cell(numLineSummary, 23).value().get<double>();
    algoData.RegulatoryLoB = worksheet.cell(numLineSummary, 24).value().get<double>();
}

std::map<double, std::vector<double>> readAlgoResults(OpenXLSX::XLWorkbook& workbook, int numColResults)
{
    std::map<double, std::vector<double>> results;

    OpenXLSX::XLWorksheet worksheetResults = workbook.worksheet("Calculs T2a");
    OpenXLSX::XLWorksheet worksheetConcentrations = workbook.worksheet("Calculs T1a");

    int numLine = 6;
    std::string resultStr = worksheetResults.cell(numLine, numColResults).value().asString();

    while(resultStr != "")
    {
        std::string concentrationStr = worksheetConcentrations.cell(numLine, 3).value().asString();
        if(concentrationStr != "x")
        {
            double concentration = std::stod(concentrationStr);
            double value = std::stod(resultStr);

            results[concentration].push_back(value);
        }

        numLine++;
        resultStr = worksheetResults.cell(numLine, numColResults).value().asString();
    }

    return results;
}

std::tuple<double, double> computeMeanAndStandardDeviation(std::vector<double> numbers)
{
    double accumulator = std::accumulate(numbers.begin(), numbers.end(), 0.);
    double mean = accumulator / numbers.size();

    double squareSum = std::inner_product(numbers.begin(), numbers.end(), numbers.begin(), 0.0);
    double standardDeviation = std::sqrt(squareSum / numbers.size() - mean * mean);

    return {mean, standardDeviation};
}

std::map<double, std::tuple<double, double>> computeAllMeanAndSigma(const std::map<double, std::vector<double>>& resultsByConcentration)
{
    std::map<double, std::tuple<double, double>> meansAndSigma;

    for(auto [concentration, results] : resultsByConcentration)
        meansAndSigma[concentration] = computeMeanAndStandardDeviation(results);

    return meansAndSigma;
}

void deduceLODsConcentrations(AlgoData& algoData)
{
    auto [negMean, negSigma] = algoData.meansAndSigmaByConcentration[0];
    algoData.LoDValue = negMean + negSigma * 3;

    for (auto it = algoData.meansAndSigmaByConcentration.rbegin(); it != algoData.meansAndSigmaByConcentration.rend(); it++)
    {
        double concentration = it->first;
        auto [mean, sigma] = it->second;

        if(mean <= algoData.LoDValue)
            break;

        algoData.LoD = concentration;
    }

    for (auto it = algoData.meansAndSigmaByConcentration.rbegin(); it != algoData.meansAndSigmaByConcentration.rend(); it++)
    {
        double concentration = it->first;
        auto [mean, sigma] = it->second;

        double potentialLoDValue = algoData.RegulatoryLoB + 1.645 * sigma;
        double difference = potentialLoDValue - algoData.RegulatoryLoDValue;

        if((difference < 0.00001) && (difference > -0.00001))
        {
            algoData.RegulatoryLoD = concentration;
            break;
        }
    }
}

AlgoData readAlgoData(OpenXLSX::XLWorkbook& workbook, Algo algoName)
{
    std::map<Algo, int> numLineSummary = {
            {AlgoBrut, 27},
            {AlgoExpoFit, 28},
            {AlgoPoly2, 29},
            {AlgoConvolBrut, 31},
            {AlgoConvolExpoFit, 32},
            {AlgoConvolPoly2, 33}
    };

    std::map<Algo, int> numColData = {
            {AlgoBrut, 7},
            {AlgoExpoFit, 8},
            {AlgoPoly2, 9},
            {AlgoConvolBrut, 11},
            {AlgoConvolExpoFit, 12},
            {AlgoConvolPoly2, 13}
    };

    AlgoData algoData;

    readSummaryData(workbook, numLineSummary[algoName], algoData);
    std::map<double, std::vector<double>> resultsByConcentration = readAlgoResults(workbook, numColData[algoName]);
    algoData.meansAndSigmaByConcentration = computeAllMeanAndSigma(resultsByConcentration);

    deduceLODsConcentrations(algoData);

    return algoData;
}

void writeConcentrations(OpenXLSX::XLWorkbook& workbook,
                         int numLine,
                         const std::map<double, std::tuple<double, double>>& meansAndSigmaByConcentration,
                         const std::string& nameFile)
{
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Data");
    int numCol = 3;
    for(auto element : meansAndSigmaByConcentration)
    {
        double concentration = element.first;
        worksheet.cell(numLine, numCol).value() = concentration;
        worksheet.cell(numLine, numCol + 21).value() = concentration;
        numCol++;
    }
    worksheet.cell(numLine, 2).value() = nameFile;
    worksheet.cell(numLine, 23).value() = nameFile;
}

void writeAlgoData(OpenXLSX::XLWorkbook& workbook, int numLine, int numLineLoB, const AlgoData& algoData)
{
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Data");
    int numCol = 3;

    for(auto& element : algoData.meansAndSigmaByConcentration)
    {
        auto [mean, sigma] = element.second;
        worksheet.cell(numLine, numCol).value() = mean;
        worksheet.cell(numLine, numCol + 21).value() = sigma;
        numCol++;
    }

    uint32_t spaceBetweenParamGrid = 8;
    int shift = (numLine - numLineLoB) / spaceBetweenParamGrid;

    worksheet.cell(numLine, 13).value() = algoData.LoD;
    worksheet.cell(numLine, 14).value() = algoData.LoDValue;
    worksheet.cell(numLine, 15).value() = algoData.RegulatoryLoD;
    worksheet.cell(numLine, 16).value() = algoData.RegulatoryLoDValue;
    worksheet.cell(numLineLoB, 18 + shift).value() = algoData.RegulatoryLoB;
}

Data compileData(OpenXLSX::XLWorkbook& workbook, const std::filesystem::path& filePath, uint32_t firstLineToWrite, uint32_t firstLineProto)
{
    OpenXLSX::XLDocument file;
    file.open(filePath.string());

    OpenXLSX::XLWorkbook workbookDataToRead = file.workbook();

    Data data;
    data.brut = readAlgoData(workbookDataToRead, AlgoBrut);
    data.expoFit = readAlgoData(workbookDataToRead, AlgoExpoFit);
    data.poly2 = readAlgoData(workbookDataToRead, AlgoPoly2);
    data.convolBrut = readAlgoData(workbookDataToRead, AlgoConvolBrut);
    data.convolExpoFit = readAlgoData(workbookDataToRead, AlgoConvolExpoFit);
    data.convolPoly2 = readAlgoData(workbookDataToRead, AlgoConvolPoly2);

    writeConcentrations(workbook, firstLineToWrite, data.brut.meansAndSigmaByConcentration, filePath.filename().stem().string());

    writeAlgoData(workbook, firstLineToWrite + 1, firstLineProto + 1, data.brut);
    writeAlgoData(workbook, firstLineToWrite + 2, firstLineProto + 2, data.expoFit);
    writeAlgoData(workbook, firstLineToWrite + 3, firstLineProto + 3, data.poly2);
    writeAlgoData(workbook, firstLineToWrite + 4, firstLineProto + 4, data.convolBrut);
    writeAlgoData(workbook, firstLineToWrite + 5, firstLineProto + 5, data.convolExpoFit);
    writeAlgoData(workbook, firstLineToWrite + 6, firstLineProto + 6, data.convolPoly2);

    return data;
}

std::vector<std::vector<std::string>> sortFileNames(std::map<std::string, Data> allData)
{
    std::vector<std::vector<std::string>> allFilesName = {{}, {}, {}};

    for(const auto& firstFileData : allData)
    {
        std::string fileName = firstFileData.first;
        Proto protoNumber = searchProtoNumber(std::filesystem::path(fileName));

        if(protoNumber == Proto301)
            allFilesName[0].push_back(fileName);
        else if(protoNumber == Proto304)
            allFilesName[1].push_back(fileName);
        else if(protoNumber == Proto305)
            allFilesName[2].push_back(fileName);
    }

    return allFilesName;
}

double computeRSquares(const std::vector<double>& curve1, const std::vector<double>& curve2)
{
    double sumNumerator = 0, sumDenominator = 0;

    double accumulator = std::accumulate(curve1.begin(), curve1.end(), 0.);
    double mean = accumulator / curve1.size();

    for(uint32_t i = 0; i < curve1.size(); i++)
    {
        sumNumerator += std::pow(curve1[i] - curve2[i], 2);
        sumDenominator += std::pow(curve1[i] - mean, 2);
    }

    return 1 - (sumNumerator / sumDenominator);
}

std::tuple<double, double, double> computeAlgoRSquares(const AlgoData& algodata1, const AlgoData& algodata2)
{
    std::vector<double> meansAlgo1;
    std::vector<double> meansAlgo2;
    std::vector<double> sigmaAlgo1;
    std::vector<double> sigmaAlgo2;
    std::vector<double> meansAndSigmaAlgo1;
    std::vector<double> meansAndSigmaAlgo2;

    for(auto& element : algodata1.meansAndSigmaByConcentration)
    {
        double concentration =  element.first;
        if(concentration < 400.1)
        {
            auto [mean, sigma] = element.second;
            meansAlgo1.push_back(mean);
            sigmaAlgo1.push_back(sigma);
            meansAndSigmaAlgo1.push_back(mean + 1.645 * sigma);
        }
    }

    for(auto& element : algodata2.meansAndSigmaByConcentration)
    {
        double concentration =  element.first;
        if(concentration < 400.1)
        {
            auto [mean, sigma] = element.second;
            meansAlgo2.push_back(mean);
            sigmaAlgo2.push_back(sigma);
            meansAndSigmaAlgo2.push_back(mean + 1.645 * sigma);
        }
    }

    double RSquaresMeans = computeRSquares(meansAlgo1, meansAlgo2);
    double RSquaresSigma = computeRSquares(sigmaAlgo1, sigmaAlgo2);
    double RSquaresMeanSigma = computeRSquares(meansAndSigmaAlgo1, meansAndSigmaAlgo2);

    return {RSquaresMeans, RSquaresSigma, RSquaresMeanSigma};
}

void writeRSquares(OpenXLSX::XLWorkbook& workbook,
                   const std::tuple<double, double, double>& RSquaresMeanAndSigma,
                   int numLineExcel,
                   int numColExcelMean)
{
    auto [meanRSquare, sigmaRSquare, meanAndSigmaSquare] = RSquaresMeanAndSigma;
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Data");

    worksheet.cell(numLineExcel, numColExcelMean).value() = meanRSquare;
    worksheet.cell(numLineExcel, numColExcelMean + 11).value() = sigmaRSquare;
    worksheet.cell(numLineExcel, numColExcelMean + 19).value() = meanAndSigmaSquare;
}

void computeDevicesRSquares(OpenXLSX::XLWorkbook& workbook,
                            std::map<std::string, Data>& allData,
                            const std::vector<std::string>& namesDevice1,
                            const std::vector<std::string>& namesDevice2,
                            int numLineExcel,
                            int numColExcel,
                            std::map<std::string, DataRSquares>& allDataRSquares)
{
    int spaceBetweenGrids = 12;

    for(auto& name1 : namesDevice1)
    {
        Data data1 = allData[name1];
        int numColExcelCopy = numColExcel;

        for(auto& name2 : namesDevice2)
        {
            Data data2 = allData[name2];
            DataRSquares dataRSquares;

            dataRSquares.brut = computeAlgoRSquares(data1.brut, data2.brut);
            dataRSquares.expoFit = computeAlgoRSquares(data1.expoFit, data2.expoFit);
            dataRSquares.poly2 = computeAlgoRSquares(data1.poly2, data2.poly2);
            dataRSquares.convolBrut = computeAlgoRSquares(data1.convolBrut, data2.convolBrut);
            dataRSquares.convolExpoFit = computeAlgoRSquares(data1.convolExpoFit, data2.convolExpoFit);
            dataRSquares.convolPoly2 = computeAlgoRSquares(data1.convolPoly2, data2.convolPoly2);

            writeRSquares(workbook, dataRSquares.brut, numLineExcel, numColExcelCopy);
            writeRSquares(workbook, dataRSquares.expoFit, numLineExcel + spaceBetweenGrids, numColExcelCopy);
            writeRSquares(workbook, dataRSquares.poly2, numLineExcel + spaceBetweenGrids * 2, numColExcelCopy);
            writeRSquares(workbook, dataRSquares.convolBrut, numLineExcel + spaceBetweenGrids * 3, numColExcelCopy);
            writeRSquares(workbook, dataRSquares.convolExpoFit, numLineExcel + spaceBetweenGrids * 4, numColExcelCopy);
            writeRSquares(workbook, dataRSquares.convolPoly2, numLineExcel + spaceBetweenGrids * 5, numColExcelCopy);
            numColExcelCopy++;

            allDataRSquares[name1+"/"+name2] = dataRSquares;
        }

        numLineExcel++;
    }
}

std::vector<std::vector<DataRSquares>> sortRSquaresData(const std::map<std::string, DataRSquares>& allDataRSquares,
                                                        const std::vector<std::vector<std::string>>& fileNames)
{

    std::vector<std::vector<DataRSquares>> allRSquaresTrio;
    std::map<uint32_t, std::string> mapIDParams = {{1, fileNames[0][0]},
                                                   {2, fileNames[0][1]},
                                                   {3, fileNames[0][2]},
                                                   {4, fileNames[1][0]},
                                                   {5, fileNames[1][1]},
                                                   {6, fileNames[1][2]},
                                                   {7, fileNames[2][0]},
                                                   {8, fileNames[2][1]},
                                                   {9, fileNames[2][2]}};

    for(uint32_t firstDeviceID = 1; firstDeviceID <= 3; firstDeviceID++)
    {
        for(uint32_t secondDeviceID = 4; secondDeviceID <= 6; secondDeviceID++)
        {
            for(uint32_t thirdDeviceID = 7; thirdDeviceID <= 9; thirdDeviceID++)
            {
                std::vector<DataRSquares> dataTrio;

                for(auto& [name, dataRSquare] : allDataRSquares)
                {
                    if((name.find(mapIDParams[firstDeviceID]) != -1 && name.find(mapIDParams[secondDeviceID]) != -1) ||
                       (name.find(mapIDParams[firstDeviceID]) != -1 && name.find(mapIDParams[thirdDeviceID]) != -1) ||
                       (name.find(mapIDParams[secondDeviceID]) != -1 && name.find(mapIDParams[thirdDeviceID]) != -1))
                    {
                        dataTrio.push_back(dataRSquare);
                    }
                    if(dataTrio.size()==3)
                    {
                        allRSquaresTrio.push_back(dataTrio);
                        break;
                    }
                }
            }
        }
    }

    return allRSquaresTrio;
}

std::tuple<double, double, double> meanAlgoRSquares(std::tuple<double, double, double> first,
                                                    std::tuple<double, double, double> second,
                                                    std::tuple<double, double, double> third)
{
    auto [meanFirst, sigmaFirst, meanAndSigmaFirst] = first;
    auto [meanSecond, sigmaSecond, meanAndSigmaSecond] = second;
    auto [meanThird, sigmaThird, meanAndSigmaThird] = third;

    double mean = (meanFirst + meanSecond + meanThird) / 3;
    double sigma = (sigmaFirst + sigmaSecond + sigmaThird) / 3;
    double meanAndSigma = (meanAndSigmaFirst + meanAndSigmaSecond + meanAndSigmaThird) / 3;

    return {mean, sigma, meanAndSigma};
}

void writeOneAlgoOneTrio(OpenXLSX::XLWorkbook& workbook,
                         const std::tuple<double, double, double>& data,
                         int numLine,
                         int numColMeans)
{
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Data");
    auto [mean, sigma, meanAndSigma] = data;

    worksheet.cell(numLine, numColMeans).value() = mean;
    worksheet.cell(numLine, numColMeans + 11).value() = sigma;
    worksheet.cell(numLine, numColMeans + 19).value() = meanAndSigma;
}

void writeTrioRSquaresResult(OpenXLSX::XLWorkbook& workbook, const DataRSquares& result, int numLine)
{
    int numColMeansBrut = 3;

    writeOneAlgoOneTrio(workbook, result.brut, numLine, numColMeansBrut);
    writeOneAlgoOneTrio(workbook, result.expoFit, numLine, numColMeansBrut + 1);
    writeOneAlgoOneTrio(workbook, result.poly2, numLine, numColMeansBrut + 2);
    writeOneAlgoOneTrio(workbook, result.convolBrut, numLine, numColMeansBrut + 3);
    writeOneAlgoOneTrio(workbook, result.convolExpoFit, numLine, numColMeansBrut + 4);
    writeOneAlgoOneTrio(workbook, result.convolPoly2, numLine, numColMeansBrut + 5);
}

void computeTrioMeans(OpenXLSX::XLWorkbook& workbook, const std::vector<std::vector<DataRSquares>>& sortedDataTrio)
{
    int numLine = 174;

    for(auto trio : sortedDataTrio)
    {
        DataRSquares result;

        DataRSquares first = trio[0];
        DataRSquares second = trio[1];
        DataRSquares third = trio[2];

        result.brut = meanAlgoRSquares(first.brut, second.brut, third.brut);
        result.expoFit = meanAlgoRSquares(first.expoFit, second.expoFit, third.expoFit);
        result.poly2 = meanAlgoRSquares(first.poly2, second.poly2, third.poly2);
        result.convolBrut = meanAlgoRSquares(first.convolBrut, second.convolBrut, third.convolBrut);
        result.convolExpoFit = meanAlgoRSquares(first.convolExpoFit, second.convolExpoFit, third.convolExpoFit);
        result.convolPoly2 = meanAlgoRSquares(first.convolPoly2, second.convolPoly2, third.convolPoly2);

        writeTrioRSquaresResult(workbook, result, numLine);
        numLine++;
    }
}

#ifdef WIN32
int wmain(int argc, wchar_t ** argv){
#else
int main(int argc, char ** argv){
#endif
    if (argc < 3){
        print_usage(argv[0]);
        return 0;
    }

    std::filesystem::path dataExcelFileFolder(argv[1]);
    std::filesystem::path modelExcelFile(argv[2]);
    std::string pathResult = dataExcelFileFolder.string() + "/Compiled curves.xlsx";

    uint32_t spaceBetweenParamGrid = 8;

    std::map<Proto, uint32_t> firstLinesResultFile = {{Proto301, 2}, {Proto304, 36}, {Proto305, 70}};
    std::map<Proto, uint32_t> accumulatorCompiledFiles = {{Proto301, 0}, {Proto304, 0}, {Proto305, 0}};

    OpenXLSX::XLDocument file;
    file.open(modelExcelFile.string());
    OpenXLSX::XLWorkbook workbook = file.workbook();
    workbook.setFullCalculationOnLoad();

    std::map<std::string, Data> allData;

    auto folderIterator = std::filesystem::directory_iterator(dataExcelFileFolder);
    for(auto& element : folderIterator)
    {
        std::filesystem::path filePath = element.path();

        Proto protoNumber = searchProtoNumber(filePath);
        if(protoNumber == Unknown)
            continue;

        uint32_t firstLineProto = firstLinesResultFile[protoNumber];
        uint32_t firstLineToWrite = firstLineProto + spaceBetweenParamGrid * accumulatorCompiledFiles[protoNumber];

        Data currentFileData = compileData(workbook, filePath, firstLineToWrite, firstLineProto);
        allData[filePath.filename().stem().string()] = currentFileData;

        accumulatorCompiledFiles[protoNumber]++;
    }

    std::vector<std::vector<std::string>> fileNames = sortFileNames(allData);

    std::map<std::string, DataRSquares> allDataRSquares;

    computeDevicesRSquares(workbook, allData, fileNames[0], fileNames[1], 105, 3, allDataRSquares);
    computeDevicesRSquares(workbook, allData, fileNames[0], fileNames[2], 105, 6, allDataRSquares);
    computeDevicesRSquares(workbook, allData, fileNames[1], fileNames[2], 108, 6, allDataRSquares);

    std::vector<std::vector<DataRSquares>> sortedDataTrio = sortRSquaresData(allDataRSquares, fileNames);
    computeTrioMeans(workbook, sortedDataTrio);

    file.saveAs(pathResult);
    file.close();

    return 0;
}