//
// Created by KarimSadki on 09/12/2020.
//
#include <iostream>
#include <string>
#include <filesystem>
#include <numeric>

#include "common/cropper/StripCropperRectangleDetection.h"
#include "GitVersion.h"
#include "monoplex/algos/AlgoExponantialFit.h"

int PROTOV1_HEIGHT = 1350;
int PROTOV2_PRECROPPED_HEIGHT = 504;
int PROTOV3_HEIGHT = 1944;

enum ToolFunction { Undefined, Cropper };

struct CroppingResults{
    int croppedWidth;
    bool success;
    double elapsed;
    std::string deviceType;
};

typedef std::vector<CroppingResults> AllCroppingResults;

struct ImageInformations{
    cv::Mat image;
    std::filesystem::path filePath;
    std::filesystem::path fileName;
    std::filesystem::path stepsFolderPath;
};

void print_usage(const char * applicationName){
    std::cout << "Usage:" << std::endl;
    std::cout << applicationName << " imagesToCropFolderPath outputImagesFolderPath csvStatsFilePath [--crop] [--analyze] [--intermediateStep] [--12bits]" << std::endl;
}

std::string version()
{
    if (!GitVersion::Tag.empty())
        return GitVersion::Tag;

    std::ostringstream oss;
    if (GitVersion::Branch.empty())
        oss <<  "HEAD detached at ";
    else
        oss << GitVersion::Branch << ":";

    oss << GitVersion::Revision;
    return oss.str();
}

void fillEndCsvSectionCrop(std::ofstream& csvFile, const AllCroppingResults & results)
{
    double sumTimeSpan = std::accumulate(results.begin(), results.end(), 0.0, [](double sum, const CroppingResults & currentResult) {
        return sum + currentResult.elapsed;
    });

    double mean = std::accumulate(results.begin(), results.end(), 0.0, [](double sum, const CroppingResults & currentResult) {
        return sum + currentResult.croppedWidth;
    }) / results.size();

    double accumulator = 0;
    for(auto& result : results)
        accumulator += std::pow(result.croppedWidth - mean, 2);

    double standardDeviation = std::sqrt(accumulator / results.size());
    int globalSuccessPercentage = (double) std::count_if(results.begin(), results.end(), [](const CroppingResults & result) { return result.success; }) / results.size() * 100;

    csvFile << ";Results;" << mean << ";" << standardDeviation << ";" << standardDeviation / mean << ";" << globalSuccessPercentage << ";" << sumTimeSpan;
}

CroppingResults processOneImage(bool saveSteps,
                     StripCropperRectangleDetection& cropperProtoV1,
                     StripCropperRectangleDetection& cropperProtoV2PreCropped,
                     StripCropperRectangleDetection& cropperProtoV3PreCropped,
                     const ImageInformations& imageInformations)
{
    cv::Mat imageSource, cropped;
    std::chrono::high_resolution_clock::time_point t1;
    std::vector<cv::Mat> intermediateSteps;
    CroppingResults results;
    results.deviceType = "ProtoV1";
    StripCropperRectangleDetection * cropper = nullptr;

    if(imageInformations.image.rows == PROTOV1_HEIGHT)
    {
        imageSource = imageInformations.image;
        cropper = &cropperProtoV1;
    }
    else if (imageInformations.image.rows == PROTOV2_PRECROPPED_HEIGHT)
    {
        results.deviceType = "ProtoV2_PreCropped";
        cv::Mat rotated;
        cv::rotate(imageInformations.image, imageSource, cv::ROTATE_90_COUNTERCLOCKWISE);
        cropper = &cropperProtoV2PreCropped;
    }
    else if (imageInformations.image.rows == PROTOV3_HEIGHT)
    {
        results.deviceType = "ProtoV3_PreCropped";
        cv::Mat rotated;
        cv::rotate(imageInformations.image, imageSource, cv::ROTATE_90_CLOCKWISE);
        cropper = &cropperProtoV3PreCropped;
    }

    t1 = std::chrono::high_resolution_clock::now();
    if(saveSteps)
        cropped = cropper->crop(imageSource, imageSource, &intermediateSteps);
    else
        cropped = cropper->crop(imageSource, imageSource);

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1);

    double time_span_milli = time_span.count() / 1000;
    results.elapsed = time_span_milli;

    uint32_t minimumAcceptableWidthPx = ImageAnalyzer::convertMillimetersToPixels(cropper->cropParams.referenceNumberOfPixels, cropper->minimumAcceptableWidth);
    uint32_t maximumAcceptableWidthPx = ImageAnalyzer::convertMillimetersToPixels(cropper->cropParams.referenceNumberOfPixels, cropper->maximumAcceptableWidth);

    bool success = ( minimumAcceptableWidthPx < cropped.cols) &&
                   (cropped.cols < maximumAcceptableWidthPx);
    results.success = success;
    results.croppedWidth = cropped.cols;

    if(cropped.cols > 0)
    {
        std::string elementNameWithoutExtension = imageInformations.fileName.stem().string();

        if(saveSteps)
        {
            for(int i = 0; i < intermediateSteps.size(); i++)
                if(!intermediateSteps[i].empty())
                    cv::imwrite(imageInformations.stepsFolderPath.string()+elementNameWithoutExtension+"_step"+std::to_string(i)+".png",intermediateSteps[i]);
        }
        cv::imwrite(imageInformations.stepsFolderPath.string()+elementNameWithoutExtension+"_step5_Cropped.png",cropped);
    }

    return results;
}

void browseOneFolderCrop(const std::filesystem::path& currentFolder,
                         const std::filesystem::path& outputImagesFolder,
                         std::ofstream& csvFile,
                         bool saveSteps,
                         StripCropperRectangleDetection& cropperProtoV1,
                         StripCropperRectangleDetection& cropperProtoV2PreCropped,
                         StripCropperRectangleDetection& cropperProtoV3PreCropped)
{
    std::filesystem::path stepsFolder(outputImagesFolder.string() + "steps/");

    if(!std::filesystem::exists(stepsFolder))
        std::filesystem::create_directory(stepsFolder);

    auto folderIterator = std::filesystem::directory_iterator(currentFolder);
    std::vector<std::filesystem::path> subFolders;
    AllCroppingResults croppingResults;

    auto csvFilePrecision = csvFile.precision();
    csvFile.precision(2);

    for(auto& element : folderIterator)
    {
        ImageInformations imageInformations;

        imageInformations.filePath = element.path();
        imageInformations.fileName = element.path().filename();
        imageInformations.stepsFolderPath = stepsFolder;

        std::string elementNameString = imageInformations.fileName.string();

        if(elementNameString.find(".png") == -1)
        {
            if(std::filesystem::is_directory(imageInformations.filePath) && elementNameString != "." && elementNameString != "..")
                subFolders.push_back(element);
            continue;
        }

        imageInformations.image = cv::imread(element.path().string());

        if((imageInformations.image.rows != PROTOV1_HEIGHT)
            && (imageInformations.image.rows != PROTOV2_PRECROPPED_HEIGHT)
               && (imageInformations.image.rows != PROTOV3_HEIGHT))
            continue;
        else {
            auto result = processOneImage(saveSteps, cropperProtoV1, cropperProtoV2PreCropped, cropperProtoV3PreCropped, imageInformations);
            csvFile << std::endl << imageInformations.filePath << ";" << result.deviceType << ";" << result.croppedWidth << ";" << result.success << ";" << result.elapsed;
            croppingResults.push_back(result);
        }
    }
    csvFile.precision(csvFilePrecision);

    if(!croppingResults.empty())
        fillEndCsvSectionCrop(csvFile, croppingResults);
    else
        for(auto& subFolder : subFolders)
            browseOneFolderCrop(subFolder, outputImagesFolder, csvFile, saveSteps, cropperProtoV1, cropperProtoV2PreCropped, cropperProtoV3PreCropped);
}

int main(int argc, char ** argv){
    if (argc < 4){
        print_usage(argv[0]);
        return 0;
    }

    bool saveSteps = false;

    CropParams cropParams = {
            Proto_v1,
            Bits8,
            UV,
            K701BT,
            151.57,
            Serum,
            ProtoV3_F060_L
    };

    ToolFunction toolFunction = Undefined;

    std::filesystem::path currentFolder(argv[1]);
    std::filesystem::path outputImagesFolder(argv[2]);

    if(!std::filesystem::is_directory(currentFolder)){
        std::cout << currentFolder.string() << " does not exist" << std::endl;
        return 0;
    }

    if(argc > 4 )
    {
        for(int i = 4; i < argc; i++)
        {
            std::string tmp = argv[i];
            if(tmp == "--intermediateStep")
                saveSteps = true;
            else if(tmp == "--12bits")
                cropParams.imageDepth = Bits12;
            else if(tmp == "--crop")
                toolFunction = Cropper;
        }
    }

    if(toolFunction == Undefined){
        std::cout << "Watch out, you forgot to precise a function for the tool, these are:"
                  << std::endl << "Cropper: --crop" << std::endl
                  << std::endl << "Analyze with avalaible algorithms: --analyze" << std::endl;
        return 0;
    }

    std::ofstream csvFile;
    csvFile.open (argv[3]);
    csvFile << "Pica version: " << version() << std::endl << std::endl;

    StripCropperRectangleDetection stripCropperProtoV1 = StripCropperRectangleDetection(cropParams);

    cropParams.deviceType = Proto_v2_PreCropped;
    StripCropperRectangleDetection stripCropperProtoV2PreCropped = StripCropperRectangleDetection(cropParams);

    cropParams.deviceType = Proto_v3;
    StripCropperRectangleDetection stripCropperProtoV3PreCropped = StripCropperRectangleDetection(cropParams);

    if(toolFunction == Cropper)
    {
        csvFile << "Function: CropperV3" << std::endl << std::endl << "Image path;Device Type;Cropped width;Cropping success;Time in milliseconds;"
                                                                      ";Average;Standard deviation;Coefficient of Variation;Percentage of success in folder;Time cropping folder (milliseconds)";

        browseOneFolderCrop(currentFolder, outputImagesFolder, csvFile, saveSteps, stripCropperProtoV1, stripCropperProtoV2PreCropped, stripCropperProtoV3PreCropped);
    }

    csvFile.close();
    return 0;
}
