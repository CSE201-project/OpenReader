//
// Created by KarimSadki on 19/08/2021.
//
#include <iostream>
#include <fstream>

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

#include "Parser.h"

std::vector<std::string> Parser::parseByLines(const std::filesystem::path& filePath)
{
    std::ifstream fileStream(filePath);
    std::string content((std::istreambuf_iterator<char>(fileStream)), (std::istreambuf_iterator<char>()));

    return parseString(content, "\n");
}

std::vector<std::string> Parser::parseString(const std::string& content, const std::string& delimiter)
{
    if(delimiter == "" || content.find(delimiter) == -1)
        return {content};

    std::vector<std::string> allParsedStrings;
    int start = 0;
    int end = content.find(delimiter);

    while (end != -1) {
        std::string subStr(content.begin() + start, content.begin() + end);
        allParsedStrings.push_back(subStr);

        start = end + delimiter.size();
        end = content.find(delimiter, start);
    }
    allParsedStrings.push_back(std::string(content.begin() + start, content.end()));

    return allParsedStrings;
}

std::vector<double> Parser::parseStringAsDoubles(const std::string& content, const std::string& delimiter)
{
    std::vector<std::string> valuesAsStrings = parseString(content, delimiter);

    std::vector<double> values;
    for(auto valueAsString : valuesAsStrings)
    {
        try {
            values.push_back(std::stod(valueAsString));
        }
        catch (const std::invalid_argument& exception) {
            continue;
        }
    }


    return values;
}

std::map<std::string, std::string>
Parser::parsePatternFileHeader(const std::filesystem::path& patternFilePath)
{
    std::vector<std::string> lines = parseByLines(patternFilePath);
    int indexEndHeader = find(lines.begin(), lines.end(), "") - lines.begin();

    std::map<std::string, std::string> patternHeaderInformations;

    for(int i = 0; i < indexEndHeader; i++)
    {
        /*The numeric information need to have the following format:
         * name_of_the_information: number-*/
        std::vector<std::string> parsedInformations = Parser::parseString(lines[i], ": ");
        if(parsedInformations.size() == 2)
        {
            std::string informationName = parsedInformations[0];
            std::string information(parsedInformations[1].begin(), parsedInformations[1].end());
            patternHeaderInformations[informationName] = information;
        }
    }

    return patternHeaderInformations;
}

Parser::ParsedPattern Parser::parsePatternFileBody(const std::filesystem::path& patternFilePath)
{
    std::vector<std::string> lines = parseByLines(patternFilePath);
    int indexStartCodedNames = 1 + find(lines.begin(), lines.end(), "") - lines.begin();
    int indexStartPatternBody = 1 + find(lines.begin() + indexStartCodedNames, lines.end(), "") - lines.begin();

    std::map<std::string, std::string> codedSpotNames;

    for(int i = indexStartCodedNames; i < indexStartPatternBody - 1; i++)
    {
        std::vector<std::string> codedName = Parser::parseString(lines[i], ":");
        if(codedName.size() == 2)
            codedSpotNames[codedName[0]] = codedName[1];
    }

    ParsedPattern parsedPattern;
    int nbColumns = Parser::parseString(lines[indexStartPatternBody], " ").size();
    std::vector<std::vector<std::tuple<std::string, std::string>>> testSpotInformationsByColumns(nbColumns);

    for(int i = indexStartPatternBody; i < lines.size(); i++)
    {
        std::vector<std::string> splittedPatternLine = Parser::parseString(lines[i], " ");
        for(int indexCol = 0; indexCol < splittedPatternLine.size(); indexCol++)
        {
            std::string currentSpotCode = splittedPatternLine[indexCol];
            if((currentSpotCode.size() == 1) && (currentSpotCode != "-"))
            {
                if(i == indexStartPatternBody)
                    parsedPattern.SpotsCASInformations.push_back(currentSpotCode);
                else
                {
                    std::string spotFullName = codedSpotNames[currentSpotCode];
                    std::string distanceWithCAS = std::to_string(i - indexStartPatternBody);
                    testSpotInformationsByColumns[indexCol].push_back({distanceWithCAS, spotFullName});
                }
            }
        }
    }
    parsedPattern.SpotsTestInformations = testSpotInformationsByColumns;

    return parsedPattern;
}

std::map<std::string, std::string> parseJSONParams(const std::string& JSONParams)
{
    Poco::JSON::Parser jsonParser;
    Poco::Dynamic::Var result = jsonParser.parse(JSONParams);

    const auto &jsonObject = result.extract<Poco::JSON::Object::Ptr>();
    Poco::Dynamic::Struct<std::string> aStruct = Poco::JSON::Object::makeStruct(jsonObject);

    std::map<std::string, std::string> params;
    for (const auto &prop : aStruct)
        params[prop.first] = prop.second.toString();

    return params;
}

std::string Parser::findDeviceName(const std::filesystem::path& JSONFilePath)
{
    std::ifstream fileStream(JSONFilePath);
    const std::string fileContent((std::istreambuf_iterator<char>(fileStream)), (std::istreambuf_iterator<char>()));

    try {
        std::map<std::string, std::string> parsedJSONParams = parseJSONParams(fileContent);
        const std::string deviceJSONString = parsedJSONParams["device"];

        std::map<std::string, std::string> parsedDeviceJSON = parseJSONParams(deviceJSONString);

        return parsedDeviceJSON["name"];
    }
    catch (const Poco::Exception& exception) {
        std::cerr << "JSON exception caugth during device name research" << std::endl;
        return "";
    }
}