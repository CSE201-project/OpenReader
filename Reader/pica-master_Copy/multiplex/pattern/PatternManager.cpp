//
// Created by KarimSadki on 04/01/2021.
//
#include <iostream>
#include <filesystem>
#include <common/fileProcessing/Parser.h>

#include "PatternManager.h"
#include "common/imageProcessing/ImageAnalyzer.h"

PatternManager::PatternManager(std::filesystem::path patternFilePath)
{
    initPattern(patternFilePath);
}

std::vector<SpotLocation> PatternManager::initAllLandmarks(const std::filesystem::path& gridsLocationsFilePath, double referenceNumberOfPixels)
{
    std::ifstream fileStream(gridsLocationsFilePath);

    std::string content((std::istreambuf_iterator<char>(fileStream)), (std::istreambuf_iterator<char>()));
    std::vector<std::string> lines = Parser::parseString(content, "\n");

    std::vector<SpotLocation> landmarks;
    for(int32_t i = 0; i < lines.size(); i++)
    {
        if(lines[i] == "")
            continue;

        std::vector<std::string> landmarkData = Parser::parseString(lines[i], ",");

        /* If we have a line with an incorrect format (3 elements separate with comma)
         * we consider that the file is not correct, so we can't collect properly landmarks coordinates*/
        if(landmarkData.size() != 3)
            return {};

        uint32_t xCoordinate = ImageAnalyzer::convertMillimetersToPixels(referenceNumberOfPixels,
                                                                         std::stod(landmarkData[0])) - 1;
        uint32_t yCoordinate = ImageAnalyzer::convertMillimetersToPixels(referenceNumberOfPixels,
                                                                         std::stod(landmarkData[1])) - 1;
        ColumnIndex colIndex = std::stoi(landmarkData[2]);
        SpotLocation landmark = {
                .point = cv::Point(xCoordinate, yCoordinate),
                .colIndexInPattern = colIndex,
                .lineIndexInPattern = 1
        };

        landmarks.push_back(landmark);

    }
    return landmarks;
}

void PatternManager::initField(const std::string& informationName, const std::string& numberStr)
{
    if(informationName == "horizontal_pitch")
        _horizontalPitch = std::stod(std::string(numberStr.begin(), numberStr.end()));
    else if(informationName == "vertical_pitch")
        _verticalPitch = std::stod(std::string(numberStr.begin(), numberStr.end()));
    else if(informationName == "number_columns")
        _numberOfCols = std::stoi(std::string(numberStr.begin(), numberStr.end()));
}

void PatternManager::initPattern(const std::filesystem::path& patternFilePath)
{
    std::map<std::string, std::string> patternHeaderInformations
            = Parser::parsePatternFileHeader(patternFilePath);

    for(auto& element : patternHeaderInformations)
    {
        std::string informationName = element.first;
        std::string information = element.second;
        initField(informationName, information);
    }

    Parser::ParsedPattern parsedPattern = Parser::parsePatternFileBody(patternFilePath);

    for(int i = 0; i < parsedPattern.SpotsCASInformations.size(); i++)
    {
        ColumnIndex colIndex = i + 1;
        pattern.SpotsCAS[colIndex] = std::stoi(parsedPattern.SpotsCASInformations[i]);

        std::vector<std::tuple<std::string, std::string>> columnSpotTestInformations = parsedPattern.SpotsTestInformations[i];

        for(auto [lineIndexStr, spotType] : columnSpotTestInformations)
        {
            pattern.SpotsTest[colIndex].push_back({std::stoi(lineIndexStr), spotType});
            pattern.SpotsSpecies.insert(spotType);
        }
    }
}

GridOfSpots PatternManager::buildGridOfSpots(const cv::Size& size,
                                             const SpotLocation& landmark,
                                             double referenceNumberOfPixels)
{
    uint32_t xShift = ImageAnalyzer::convertMillimetersToPixels(referenceNumberOfPixels, _horizontalPitch);
    uint32_t yShift = ImageAnalyzer::convertMillimetersToPixels(referenceNumberOfPixels, _verticalPitch);
    uint32_t xLimit = size.width - ImageAnalyzer::convertMillimetersToPixels(referenceNumberOfPixels, _horizontalPitch / 2.) - 1;

    GridOfSpots grid;

    for(ColumnIndex colIndex = landmark.colIndexInPattern; grid.ColumnsOrderInImage.size() < _numberOfCols; colIndex = (colIndex + 1)%_numberOfCols)
    {
        //Determine the coordinate of the column------------------------------------------
        if(colIndex == 0)
            colIndex = _numberOfCols;

        uint32_t x = landmark.point.x + grid.ColumnsOrderInImage.size() * xShift;

        if(x > xLimit)
            break;

        //Determine coordinates of the current CAS Spot------------------------------------------

        LineIndex CASlineIndexShift = pattern.SpotsCAS[colIndex];

        cv::Point CASPoint(x, landmark.point.y + (CASlineIndexShift * yShift));
        SpotLocation CASSpotLocation = {
                .point = CASPoint,
                .colIndexInPattern = colIndex,
                .lineIndexInPattern = landmark.lineIndexInPattern + CASlineIndexShift
        };

        //Determine coordinates of Test Spots in current column------------------------------------------

        std::vector<SpotTest> currentColumnSpots;
        std::vector<std::tuple<LineIndex, SpotType>> currentColumnPattern = pattern.SpotsTest[colIndex];
        for(auto testSpot : currentColumnPattern)
        {
            LineIndex testLineIndexShift = std::get<0>(testSpot);
            SpotType spotType = std::get<1>(testSpot);

            cv::Point coordinates (x, CASSpotLocation.point.y + (testLineIndexShift * yShift));

            SpotLocation spotLocation = {
                    .point = coordinates,
                    .colIndexInPattern = colIndex,
                    .lineIndexInPattern = CASSpotLocation.lineIndexInPattern + testLineIndexShift
            };
            currentColumnSpots.push_back({spotLocation, spotType});
        }

        grid.ColumnsOrderInImage.push_back(colIndex);
        grid.SpotsCASCoordinates[colIndex] = CASSpotLocation;
        grid.SpotsTestCoordinates[colIndex] = currentColumnSpots;
    }

    return grid;
}

double PatternManager::getHorizontalPitch()
{
    return _horizontalPitch;
}

double PatternManager::getVerticalPitch()
{
    return _verticalPitch;
}

double PatternManager::getDefaultHeightNoiseROI()
{
    return _verticalPitch / 1.5;
}

double PatternManager::getDefaultWidthNoiseROI()
{
    return _horizontalPitch / 1.5;
}