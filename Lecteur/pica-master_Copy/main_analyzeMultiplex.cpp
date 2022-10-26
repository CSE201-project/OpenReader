//
// Created by KarimSadki on 05/08/2021.
//
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <multiplex/useTestAnalyzer/MultiplexUseTestAnalyzer.h>

void print_usage(const char * applicationName){
    std::cout << "Usage:" << std::endl;
}

void print_usage(const wchar_t * applicationName){
    std::cout << "Usage:" << std::endl;
}

#ifdef WIN32
int wmain(int argc, wchar_t ** argv){
#else
int main(int argc, char ** argv){
#endif
    if (argc != 17){
        print_usage(argv[0]);
        return 0;
    }

    std::filesystem::path croppedFolderPath(argv[1]);
    std::filesystem::path templateExcelFilePath(argv[2]);
    std::filesystem::path excelFilePath(argv[3]);
    std::filesystem::path patternFilePath(argv[14]);
    std::filesystem::path gridsLocationsFilePath(argv[15]);

    std::vector<std::string> paramsWithoutImagePath;
    for(int i = 4; i < 17; i++)
    {
#ifdef WIN32
        std::wstring wParamStr(argv[i]);
        std::string paramStr(wParamStr.begin(), wParamStr.end());
#else
        std::string paramStr(argv[i]);
#endif
        paramsWithoutImagePath.push_back(paramStr);
    }

    MultiplexUseTestAnalyzer useTestAnalyzer(paramsWithoutImagePath, croppedFolderPath, templateExcelFilePath, excelFilePath, patternFilePath, gridsLocationsFilePath);
    useTestAnalyzer.processUseTestAnalyze();

    return 0;
}
