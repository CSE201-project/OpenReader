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

#include "common/cropper/StripCropperRectangleDetection.h"
#include "common/fileProcessing/Parser.h"

int PROTOV2_PRECROPPED_HEIGHT = 504;

enum RunnerFunction { Undefined, Cropper, FindControlLine, DetermineDeviceInformations, GetPatternHeader };

struct ParamsToAnalyze{
    bool isVisible;
    ColorChannel channel;

    double lengthROIMm;
    uint32_t padding;

    double referenceNumberOfPixels;

    uint32_t minCLSearchArea;
    uint32_t maxCLSearchArea;

    std::filesystem::path imagePath;
};

void print_usage(const char * applicationName){
    std::cout << "Usage:" << std::endl;
    std::cout << applicationName << " imagePath cropFolder ratioNumberPixelsMm [--Visible] [--Cassette] [--V2] [--12bits]" << std::endl;
}

void print_usage(const wchar_t * applicationName){
    std::cout << "Usage:" << std::endl;
    std::wcout << applicationName << L" imagePath cropFolder ratioNumberPixelsMm [--Visible] [--Cassette] [--V2] [--12bits]" << std::endl;
}

ParamsToAnalyze initParamsFromMatlab(std::vector<std::string> paramsWithoutImagePath, std::filesystem::path imagePath)
{
    ParamsToAnalyze params;

    params.imagePath = imagePath;

    //params in paramsWithoutImagePath (and on the command line) are in this order:
    // isVisible, channel, padding, rectangleWidth, maskLength, minCLSearchArea, maxCLSearchArea, distanceCLtoTL
    if(paramsWithoutImagePath[0] == "Visible")
        params.isVisible = true;
    else
        params.isVisible = false;

    if(paramsWithoutImagePath[1] == "Red")
        params.channel = Red;
    else if(paramsWithoutImagePath[1] == "Blue")
        params.channel = Blue;
    else
        params.channel = Green;

    params.lengthROIMm = std::stod(paramsWithoutImagePath[2]);
    params.referenceNumberOfPixels = std::stod(paramsWithoutImagePath[3]);

    return params;
}

cv::Mat readImage(std::filesystem::path imagePath)
{
    cv::Mat image;
#ifdef WIN32
    std::ifstream sourceFile(imagePath.wstring().c_str(), std::ios::in | std::ios::binary);
    if(sourceFile)
    {
        sourceFile.seekg (0, sourceFile.end);
        int fileSize = sourceFile.tellg();
        sourceFile.seekg (0, sourceFile.beg);

        std::vector<char> buffer(fileSize);
        sourceFile.read(buffer.data(), fileSize);

        image = cv::imdecode(cv::Mat(buffer), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        sourceFile.close();
    }
    else{
        std::cerr << "Error during source image reading" << std::endl;
        exit(1);
    }
#else
    image = cv::imread(imagePathStr);
#endif
    return image;
}

void analyzeImage(ParamsToAnalyze params, RunnerFunction runnerFunction)
{
    cv::Mat cropped = readImage(params.imagePath);

    uint32_t lengthCenterPx = ImageAnalyzer::convertMillimetersToPixels(params.referenceNumberOfPixels, params.lengthROIMm);
    int theoricPadding = cropped.cols - lengthCenterPx;
    if(theoricPadding < 2)
        theoricPadding = 2;

    params.padding = theoricPadding;

    const auto [oneChannelImage, stripCroppedPadding] = ImageAnalyzer::extractChannelImageWithPadding(cropped, params.channel, params.padding);;

    std::vector<double> stripProfile = ImageAnalyzer::computeImageProfile(stripCroppedPadding, params.isVisible, params.channel);

    params.minCLSearchArea = 10;
    params.maxCLSearchArea = cropped.rows / 3;

    uint32_t controlLine = ImageAnalyzer::findControlLine(stripProfile, params.referenceNumberOfPixels, params.minCLSearchArea, params.maxCLSearchArea);
    std::cout << controlLine + 1;
}

std::filesystem::path constructImagePCropPath(const std::filesystem::path& imagePath)
{
    std::filesystem::path nameImage = imagePath.filename();
    std::filesystem::path folderPCropPath = imagePath;
    folderPCropPath.remove_filename();

    std::wstring tmp = folderPCropPath.wstring();
    int size = tmp.size();
    tmp.replace(size-2, 4, L"Crop");

    std::filesystem::path imagePCropPath(tmp);
    imagePCropPath /= nameImage;

    return imagePCropPath;
}

std::filesystem::path findJSONFile(const std::filesystem::path& folderPath)
{
    auto folderIterator = std::filesystem::directory_iterator(folderPath);
    for(auto& element : folderIterator)
    {
        std::string fileName = element.path().filename().string();
        std::regex regexp(".+_[0-9]+[a-z]+_N[0-9]+\\.json");

        if (std::regex_match(element.path().filename().string(), regexp))
            return element.path();
    }
    return std::filesystem::path();
}

std::tuple<std::string, std::string, double>
determineDeviceInformations(const std::string& deviceName, const std::filesystem::path configDevicesExcelPath)
{
    OpenXLSX::XLDocument file;
    file.open(configDevicesExcelPath.string());
    OpenXLSX::XLWorkbook workbook = file.workbook();
    OpenXLSX::XLWorksheet worksheet = workbook.worksheet("Config");

    std::string prototype, focal;
    double numberOfPixels;

    std::string currentDeviceName = worksheet.cell(2, 1).value().asString();
    int i = 2;
    while(currentDeviceName != "")
    {
        if(deviceName == currentDeviceName)
        {
            focal = worksheet.cell(i, 2).value().asString();
            numberOfPixels = worksheet.cell(i, 3).value().get<double>();
            break;
        }

        i++;
        currentDeviceName = worksheet.cell(i, 1).value().asString();
    }

    std::regex regexpV2("proto-lmx[0-9]{2}");
    std::regex regexpV3("lmx-3[0-9]{2}");

    if(std::regex_match(deviceName, regexpV2))
        prototype = "ProtoV2";
    else if(std::regex_match(deviceName, regexpV3))
        prototype = "ProtoV3";

    file.close();
    return {prototype, focal, numberOfPixels};
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

    /*Section where we determine runner's function (crop or analyze)--------------------------------------------------*/
    RunnerFunction runnerFunction = Undefined;
    #ifdef WIN32
        std::wstring wString(argv[1]);
        std::string function(wString.begin(), wString.end());
    #else
        std::string function = argv[1];
    #endif
    if(function == "--crop")
        runnerFunction = Cropper;
    else if(function == "--findCL")
        runnerFunction = FindControlLine;
    else if(function == "--deviceInfos")
        runnerFunction = DetermineDeviceInformations;
    else if(function == "--getPatternHeader")
        runnerFunction = GetPatternHeader;

    if(runnerFunction == Undefined)
    {
        std::cout << "Watch out, you forgot to precise a function for the runner, these are:"
                  << std::endl << "Cropper: --crop" << std::endl
                  << std::endl << "FindControlLine: --findCL" << std::endl;
        return 0;
    }
    /*----------------------------------------------------------------------------------------------------------------*/


    /*Section where we crop the image---------------------------------------------------------------------------------*/
    else if(runnerFunction == Cropper)
    {
#ifdef WIN32
        std::wstring numberOfPixelsStr(argv[4]);
        std::wstring cassetteNameWStr(argv[5]);
        std::wstring focalWStr(argv[6]);
        std::string cassetteName(cassetteNameWStr.begin(), cassetteNameWStr.end());
        std::string focal(focalWStr.begin(), focalWStr.end());
#else
        std::string numberOfPixelsStr(argv[4]);
        std::string cassetteName(argv[5]);
#endif
        double referenceNumberOfPixels = std::stod(numberOfPixelsStr);
        CassetteData cassetteData = CassetteType::decodeCassetteName(cassetteName);

        CropParams cropParams{
                Proto_v1,
                Bits8,
                UV,
                cassetteData.model,
                referenceNumberOfPixels,
                cassetteData.stripMatrix,
                ProtoV1_Focal
        };

        if(argc > 5 )
        {
            for(int i = 6; i < argc; i++)
            {
#ifdef WIN32
                std::wstring wString(argv[i]);
                std::string tmp(wString.begin(), wString.end());
#else
                std::string tmp = argv[i];
#endif
                if(tmp == "--V2")
                {
                    cropParams.deviceType = Proto_v2_PreCropped;
                    DeviceFocal deviceFocal = StripCropperRectangleDetection::determineProtoV2Focal(focal);
                    if(deviceFocal == UnknownFocal)
                    {
                        std::cout << "Unknown focal for ProtoV2, we can't crop correctly." << std::endl
                                  << "Please register the focal " << focal << " or use one of these: " << std::endl
                                  << "F041 F060" << std::endl;
                        return 0;
                    }

                    cropParams.deviceFocal = deviceFocal;
                }
                else if(tmp == "--V3")
                {
                    cropParams.deviceType = Proto_v3;
                    DeviceFocal deviceFocal = StripCropperRectangleDetection::determineProtoV3Focal(focal);
                    if(deviceFocal == UnknownFocal)
                    {
                        std::cout << "Unknown focal for ProtoV3, we can't crop correctly." << std::endl
                                  << "Please register the focal " << focal << " or use one of these: " << std::endl
                                  << "F041 F060 F060_L F080" << std::endl;
                        return 0;
                    }
                    cropParams.deviceFocal = deviceFocal;
                }
                else if(tmp == "--12bits")
                    cropParams.imageDepth = Bits12;
                else if(tmp == "--Visible")
                    cropParams.lightningCondition = Visible;
            }
        }

#ifdef WIN32
        std::wstring imagePathStr(argv[2]);
#else
        std::string imagePathStr(argv[2]);
#endif
        std::filesystem::path imagePath(imagePathStr);
        std::filesystem::path imagePCropPath = constructImagePCropPath(imagePath);

        cv::Mat imageToCrop, imagePCrop;

        if(std::filesystem::exists(imagePCropPath))
        {
            cv::Mat image = readImage(imagePCropPath);
            if(cropParams.deviceType == Proto_v2_PreCropped)
                cv::rotate(image, imagePCrop, cv::ROTATE_90_COUNTERCLOCKWISE);
            else if(cropParams.deviceType == Proto_v3)
                cv::rotate(image, imagePCrop, cv::ROTATE_90_CLOCKWISE);
        }

        cv::Mat image = readImage(imagePath);

        if(cropParams.deviceType == Proto_v2_PreCropped)
            cv::rotate(image, imageToCrop, cv::ROTATE_90_COUNTERCLOCKWISE);
        else if(cropParams.deviceType == Proto_v3)
            cv::rotate(image, imageToCrop, cv::ROTATE_90_CLOCKWISE);
        else
            imageToCrop = image;

        StripCropperRectangleDetection stripCropper = StripCropperRectangleDetection(cropParams);

        cv::Mat cropped;
        if(std::filesystem::exists(imagePCropPath))
            cropped = stripCropper.crop(imagePCrop, imageToCrop);
        else
            cropped = stripCropper.crop(imageToCrop, imageToCrop);

#ifdef WIN32
        std::wstring wStringDestinationDir(argv[3]);
        std::wstring wPrefix(L"RGB_");
        std::wstring wFileName(imagePath.filename().wstring());
        std::filesystem::path destFilePath(wStringDestinationDir);
        destFilePath /= wPrefix + wFileName;

        std::vector<uint8_t> buffer;
        imencode(".png", cropped,buffer);

        std::fstream  outputFile(destFilePath.wstring().c_str(), std::ios::out | std::ios::binary);
        if(outputFile)
        {
            outputFile.write((const char*)&buffer[0], buffer.size());
            outputFile.close();
        }

#else
        std::filesystem::path destFilePath(argv[3]);
        destFilePath /= "RGB_" + std::filesystem::path(imagePath).filename().string();
        cv::imwrite(destFilePath.string(),cropped);
#endif
    }
    /*----------------------------------------------------------------------------------------------------------------*/


    /*Section where we search control line------------------------------------------------------------------------------*/
    else if(runnerFunction == FindControlLine)
    {
        std::vector<std::string> paramsWithoutImagePath;
        for(int i = 2; i < 7; i++)
        {
            if(i == 4)
                continue;
#ifdef WIN32
            std::wstring wParamStr(argv[i]);
            std::string paramStr(wParamStr.begin(), wParamStr.end());
#else
            std::string paramStr(argv[i]);
#endif
            paramsWithoutImagePath.push_back(paramStr);
        }

        std::filesystem::path imagePath(argv[4]);
        ParamsToAnalyze params = initParamsFromMatlab(paramsWithoutImagePath, imagePath);
        analyzeImage(params, runnerFunction);
    }

    /*Section where we parse config file to determine Device name, px/mm and focal--------------------------------------*/
    else if(runnerFunction == DetermineDeviceInformations)
    {
        std::filesystem::path JSONFileFolderPath(argv[2]);
        std::filesystem::path configDevicesExcelPath(argv[3]);

        std::filesystem::path JSONFilePath = findJSONFile(JSONFileFolderPath);
        std::string deviceName = Parser::findDeviceName(JSONFilePath);
        
        //DEBUG
        std::cout << "Debug: " << deviceName;

        auto [prototype, focal, nbPixels] = determineDeviceInformations(deviceName, configDevicesExcelPath);
        std::cout << prototype << " " << focal << " " << nbPixels;
    }
    else if(runnerFunction == GetPatternHeader)
    {
        std::filesystem::path patternFilePath(argv[2]);
        std::map<std::string, std::string> patternHeader = Parser::parsePatternFileHeader(patternFilePath);
        std::cout << patternHeader["number_lines"]<< " " <<patternHeader["horizontal_pitch"] << " " << patternHeader["vertical_pitch"] << " " << patternHeader["theoric_spots_diameter"];
    }


    return 0;
}
