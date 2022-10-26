//
// Created by KarimSadki on 09/12/2020.
//
#include <monoplex/useTestAnalyzer/MonoplexUseTestAnalyzer.h>
#include "common/cropper/StripCropperRectangleDetection.h"

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
    if (argc < 18){
        print_usage(argv[0]);
        return 0;
    }

    std::vector<std::string> paramsWithoutImagePath;
    for(int i = 4; i < argc-1; i++)
    {
#ifdef WIN32
        std::wstring wParamStr(argv[i]);
        std::string paramStr(wParamStr.begin(), wParamStr.end());
#else
        std::string paramStr(argv[i]);
#endif
        paramsWithoutImagePath.push_back(paramStr);
    }

    std::filesystem::path croppedFolderPath(argv[1]);
    std::filesystem::path templateExcelFilePath(argv[2]);
    std::filesystem::path excelFilePath(argv[3]);
    std::filesystem::path excelFileCalibrationCurves(argv[argc-1]);

    MonoplexUseTestAnalyzer useTestAnalyzer(paramsWithoutImagePath, croppedFolderPath, templateExcelFilePath, excelFilePath, excelFileCalibrationCurves);
    useTestAnalyzer.processUseTestAnalyze();

    return 0;
}
