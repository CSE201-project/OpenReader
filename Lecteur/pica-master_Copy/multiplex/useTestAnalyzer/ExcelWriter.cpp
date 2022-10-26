//
// Created by KarimSadki on 08/04/2021.
//

#include <filesystem>

#include <OpenXLSX.hpp>
#include "ExcelWriter.h"

double searchConcentration(const std::string& name)
{
    int lastUnderscorePosition = name.find_last_of('_');
    std::string nameWithoutImageNumber(name.begin(), name.begin() + lastUnderscorePosition);
    int beforeLastUnderscorePosition = nameWithoutImageNumber.find_last_of('_');
    int sizeOfUnit = 4;

    std::string concentration(name.begin() + beforeLastUnderscorePosition + 1,
                              name.begin() + lastUnderscorePosition - sizeOfUnit);

    return std::stod(concentration);
}

std::map<double, std::vector<std::string>> initConcentrationListMultiplex(std::vector<std::string> croppedImagesNames)
{
    std::map<double, std::vector<std::string>> sortedNamesMap;

    for(auto& name : croppedImagesNames)
        sortedNamesMap[searchConcentration(name)].push_back(name);

    return sortedNamesMap;
}

void writeConcentrationListMultiplex(OpenXLSX::XLWorkbook& workbook, const std::map<double, std::vector<std::string>>& sortedNamesMap)
{
    std::string sheetName = "Spot Type 1";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    uint8_t maxNumberConcentrationInExcel = 10;
    uint8_t acc = 0;
    for(auto& element : sortedNamesMap)
    {
        if(acc + 1 > maxNumberConcentrationInExcel)
            break;
        int lineNumber = 6 + acc;
        std::string cellName = "S"+ std::to_string(lineNumber);

        worksheet.cell(OpenXLSX::XLCellReference(cellName)).value() = element.first;
        acc++;
    }
}

void writeSummaryHeaderMultiplex(OpenXLSX::XLWorkbook& workbook,
                        const std::filesystem::path& croppedFolderPath,
                        const std::string& appVersion,
                        const std::string& currentDate,
                        const std::set<std::string> spotsSpecies)
{
    std::string sheetName = "Summary";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    worksheet.cell(OpenXLSX::XLCellReference("C4")).value() = croppedFolderPath.string();
    worksheet.cell(OpenXLSX::XLCellReference("C3")).value() = currentDate;
    worksheet.cell(OpenXLSX::XLCellReference("G3")).value() = appVersion;

    uint32_t acc = 0;
    for(auto species : spotsSpecies)
    {
        worksheet.cell(OpenXLSX::XLCellReference(6 + acc * 13, 1)).value() = species;
        acc++;
    }
}

void writeCalibrationParamsMultiplex(OpenXLSX::XLWorkbook& workbook, ParamsMultiplex params)
{
    std::string sheetName = "Params";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    worksheet.cell(OpenXLSX::XLCellReference("B4")).value() = params.measure;
    worksheet.cell(OpenXLSX::XLCellReference("B5")).value() = std::filesystem::path(params.proto).string();
    worksheet.cell(OpenXLSX::XLCellReference("B6")).value() = params.cassette;

    if(params.isVisible)
        worksheet.cell(OpenXLSX::XLCellReference("B7")).value() = "Visible";
    else
        worksheet.cell(OpenXLSX::XLCellReference("B7")).value() = "UV";

    worksheet.cell(OpenXLSX::XLCellReference("B8")).value() = params.focal;
    worksheet.cell(OpenXLSX::XLCellReference("B9")).value() = params.nanoParticules;
    worksheet.cell(OpenXLSX::XLCellReference("B10")).value() = params.channelStr;

    worksheet.cell(OpenXLSX::XLCellReference("B13")).value() = params.referenceNumberOfPixels;
}

void insertResult(OpenXLSX::XLWorksheet& worksheet, int numLine, int numCol, double result)
{

    if(std::isnan(result) || std::isinf(result))
        worksheet.cell(OpenXLSX::XLCellReference(numLine, numCol)).value() = "";
    else
        worksheet.cell(OpenXLSX::XLCellReference(numLine, numCol)).value() = result;
}

void writeResults(OpenXLSX::XLWorkbook& workbook, MultiplexResults results, ImageInformations imageInformations)
{
    AlgoMultiplexResult algoBrutResults = results.convolBrutResult;

    uint8_t sheetIndex = 1;
    int numGlobalResultColumn = 7;
    int numLine = 5 + imageInformations.numImage;

    std::string nameOriginalImage(imageInformations.name.begin() + 4, imageInformations.name.end());

    for(auto globalResult : algoBrutResults.GlobalResults)
    {
        std::string sheetName = "Spot Type " + std::to_string(sheetIndex);
        OpenXLSX::XLWorksheet currentWorksheet = workbook.worksheet(sheetName);

        SpotType spotType = globalResult.first;
        std::map<ColumnIndex, double> resultsByCols = algoBrutResults.ColumnsResults[spotType];

        currentWorksheet.cell(OpenXLSX::XLCellReference(numLine, 2)).value() = nameOriginalImage;
        currentWorksheet.cell(OpenXLSX::XLCellReference(numLine, 3)).value() = imageInformations.concentration;

        insertResult(currentWorksheet, numLine, numGlobalResultColumn, globalResult.second);
        for(auto columnResult : resultsByCols)
            insertResult(currentWorksheet, numLine, numGlobalResultColumn + columnResult.first, columnResult.second);

        sheetIndex++;
    }
}

void writePatternInformations(OpenXLSX::XLWorkbook& workbook,
                              double theoricSpotDiameter,
                              double verticalSpacing,
                              double horizontalSpacing,
                              double heightNoiseROI,
                              double widthNoiseROI)
{
    std::string sheetName = "Params";
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet(sheetName);

    worksheet.cell(OpenXLSX::XLCellReference("B15")).value() = theoricSpotDiameter;
    worksheet.cell(OpenXLSX::XLCellReference("B17")).value() = verticalSpacing;
    worksheet.cell(OpenXLSX::XLCellReference("B18")).value() = horizontalSpacing;
    worksheet.cell(OpenXLSX::XLCellReference("B20")).value() = heightNoiseROI;
    worksheet.cell(OpenXLSX::XLCellReference("B21")).value() = widthNoiseROI;

}

void writeImageBrutData(OpenXLSX::XLWorkbook& workbook,
                        const std::vector<SpotLocation>& spotsLocations,
                        const std::vector<uint32_t>& totalSquareIntensities,
                        const std::vector<uint32_t>& spotsIntensities,
                        const std::vector<uint32_t>& noiseIntensities,
                        const std::string& name,
                        int numImage)
{
    int firstLineGridDate = 2;
    int verticalShift = 18;
    int horizontalShift = 7;

    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Brut images data");

    int numLineName = firstLineGridDate + (verticalShift * numImage);
    worksheet.cell(OpenXLSX::XLCellReference(numLineName, 1)).value() = name;

    for(uint32_t i = 0; i < spotsLocations.size(); i++)
    {
        int colIndexInPattern = spotsLocations[i].colIndexInPattern;
        int lineIndexInPattern = spotsLocations[i].lineIndexInPattern;

        int numColumnSpotValue = 1 + colIndexInPattern;
        int numColumnNoiseValue = 1 + colIndexInPattern + horizontalShift;
        int numColumnTotalValue = 1 + colIndexInPattern + 2 * horizontalShift;
        int numLine = numLineName - 1 + lineIndexInPattern;

        worksheet.cell(OpenXLSX::XLCellReference(numLine, numColumnSpotValue)).value() = spotsIntensities[i];
        worksheet.cell(OpenXLSX::XLCellReference(numLine, numColumnNoiseValue)).value() = noiseIntensities[i];
        worksheet.cell(OpenXLSX::XLCellReference(numLine, numColumnTotalValue)).value() = totalSquareIntensities[i];
    }
}