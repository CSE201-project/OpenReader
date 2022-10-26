//
// Created by KarimSadki on 04/01/2021.
//

#ifndef PICA_PARSER_H
#define PICA_PARSER_H

#include <opencv2/opencv.hpp>
#include <filesystem>

class Parser{
public:
    /*One sub-vector of SpotsTestInformations is a column
     *Each elements of a column is a test spot with two informations (line and type)*/
    struct ParsedPattern{
        std::vector<std::string> SpotsCASInformations;
        std::vector<std::vector<std::tuple<std::string, std::string>>> SpotsTestInformations;
    };

    static std::vector<std::string> parseString(const std::string& content, const std::string& delimiter);
    static std::vector<double> parseStringAsDoubles(const std::string& content, const std::string& delimiter);

    static std::map<std::string, std::string> parsePatternFileHeader(const std::filesystem::path& patternFilePath);
    /*In the output vector, informations are sorted following columns indices in the pattern model*/
    static ParsedPattern parsePatternFileBody(const std::filesystem::path& patternFilePath);
    static std::string findDeviceName(const std::filesystem::path& JSONFilePath);

private:
    static std::vector<std::string> parseByLines(const std::filesystem::path& filePath);
};

#endif //PICA_PARSER_H
