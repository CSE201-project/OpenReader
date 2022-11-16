
#include "MultiplexUseTestAnalyzer.h"

#include "common/imageProcessing/ImagePreProcessing.h"

MultiplexUseTestAnalyzer::MultiplexUseTestAnalyzer(const std::vector<std::string>& paramsWithoutImagePath,
                                                   const std::filesystem::path& croppedFolderPath,
                                                   const std::filesystem::path& templateExcelFilePath,
                                                   const std::filesystem::path& excelFilePath,
                                                   const std::filesystem::path& patternFilePath,
                                                   const std::filesystem::path& gridsLocationsFilePath) : _patternManager(PatternManager(patternFilePath))
{
    _params.croppedFolderPath = croppedFolderPath;
    _params.templateExcelFilePath = templateExcelFilePath;
    _params.excelFilePath = excelFilePath;
    _params.patternFilePath = patternFilePath;
    _params.gridsLocationsFilePath = gridsLocationsFilePath;

    _params.appVersion = paramsWithoutImagePath[0];
    _params.currentDate = paramsWithoutImagePath[1];
    _params.measure = paramsWithoutImagePath[2];
    _params.proto = paramsWithoutImagePath[3];
    _params.cassette = paramsWithoutImagePath[4];
    _params.focal = paramsWithoutImagePath[5];
    _params.nanoParticules = paramsWithoutImagePath[6];

    if(paramsWithoutImagePath[7] == "Visible")
        _params.isVisible = true;
    else
        _params.isVisible = false;

    _params.channelStr = paramsWithoutImagePath[8];
    if(_params.channelStr == "Red")
        _params.channel = Red;
    else if(_params.channelStr == "Blue")
        _params.channel = Blue;
    else
        _params.channel = Green;

    _params.referenceNumberOfPixels = std::stod(paramsWithoutImagePath[9]);
    _params.spotDiameter = std::stod(paramsWithoutImagePath[12]);

    _landmarks = PatternManager::initAllLandmarks(gridsLocationsFilePath, _params.referenceNumberOfPixels);
}

std::vector<std::string> MultiplexUseTestAnalyzer::searchAllCroppedImagesNames(std::filesystem::path croppedFolderPath)
{
    std::vector<std::string> names;
    auto folderIterator = std::filesystem::directory_iterator(croppedFolderPath);

    for(auto& element : folderIterator)
    {
        std::string fileName = element.path().filename().string();
        if(fileName.find("RGB_") != -1)
            names.push_back(fileName);
    }

    return names;
}

void MultiplexUseTestAnalyzer::saveImage(std::filesystem::path folder, const std::string& nameImage, const cv::Mat& image)
{
    folder /= nameImage;

    std::vector<uint8_t> buffer;
    imencode(".png", image,buffer);

    std::fstream  outputFile(folder.c_str(), std::ios::out | std::ios::binary);
    if(outputFile)
    {
        outputFile.write((const char*)&buffer[0], buffer.size());
        outputFile.close();
    }
}

std::map<SpotType, cv::Scalar> MultiplexUseTestAnalyzer::generateColorMap(std::set<SpotType> spotsSpecies)
{
    std::map<SpotType, cv::Scalar> colorMap;
    std::vector<cv::Scalar> colors = {
            cv::Scalar(200, 200, 200),
            cv::Scalar(0, 125, 255),
            cv::Scalar(200, 0, 200),
            cv::Scalar(0, 200, 200),
            cv::Scalar(0, 200, 0),
            cv::Scalar(200, 0, 0)
    };

    uint32_t acc = 0;

    for(auto spotType : spotsSpecies)
    {
        colorMap[spotType] = colors[acc];
        acc++;
    }

    return colorMap;
};

void MultiplexUseTestAnalyzer::saveImageWithROI(std::set<SpotType> spotsSpecies,
                                                double widthNoiseROI,
                                                double widthTotalSquare,
                                                ImageInformations imageInformations,
                                                ColorChannel channel,
                                                GridOfSpots gridOfSpots)
{
    std::string nameExcel = _params.excelFilePath.filename().string();
    std::string prefix;
    if(nameExcel.find("Spot diameter") != -1)
        prefix = "Results/Loops/ROI_";
    else
        prefix = "Results/ROI_";

    std::string nameWithoutExtension = _params.excelFilePath.stem().string();
    int underscorePosition = nameWithoutExtension.find('_', 8);
    std::string suffix(nameWithoutExtension.begin() + underscorePosition + 1, nameWithoutExtension.end());

    std::filesystem::path pathROI = _params.croppedFolderPath;
    pathROI.replace_filename(prefix + suffix);

    if(imageInformations.numImage == 1)
        if(!std::filesystem::exists(pathROI))
            std::filesystem::create_directory(pathROI);

    std::map<SpotType, cv::Scalar> colorMap = generateColorMap(spotsSpecies);

    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(imageInformations.cropped, frameBGRChannels);

    cv::Mat oneChannelImage;
    cv::cvtColor(frameBGRChannels[channel], oneChannelImage, cv::COLOR_GRAY2BGR);

    cv::Mat imageROIGrid = oneChannelImage.clone();
    cv::Mat imageROICircles = oneChannelImage.clone();

    double spotRadius = _params.spotDiameter / 2.;
    uint32_t circleRadius = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, spotRadius);
    uint32_t widthSquareNoise = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, widthNoiseROI);
    uint32_t widthSquare = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, widthTotalSquare);

    for(auto element : gridOfSpots.SpotsCASCoordinates)
    {
        cv::Point point = element.second.point;
        cv::Rect squareNoise(point.x - widthSquareNoise/2, point.y - widthSquareNoise/2, widthSquareNoise, widthSquareNoise);
        cv::Rect totalSquare(point.x - widthSquare/2, point.y - widthSquare/2, widthSquare, widthSquare);

        cv::circle(imageROICircles, point, circleRadius, cv::Scalar(0, 0, 255));
        cv::rectangle(imageROICircles, squareNoise, cv::Scalar(0, 0, 255));

        cv::rectangle(imageROIGrid, totalSquare, cv::Scalar(0, 255, 0));
        cv::drawMarker(imageROIGrid, point, cv::Scalar(0, 0, 255));
    }
    for(auto element : gridOfSpots.SpotsTestCoordinates)
    {
        std::vector<SpotTest> column = element.second;
        for(auto [spotLocation, spotType] : column)
        {
            cv::Rect squareNoise(spotLocation.point.x - widthSquareNoise/2, spotLocation.point.y - widthSquareNoise/2, widthSquareNoise, widthSquareNoise);
            cv::Rect totalSquare(spotLocation.point.x - widthSquare/2, spotLocation.point.y - widthSquare/2, widthSquare, widthSquare);

            cv::circle(imageROICircles, spotLocation.point, circleRadius, colorMap[spotType]);
            cv::rectangle(imageROICircles, squareNoise, colorMap[spotType]);

            cv::rectangle(imageROIGrid, totalSquare, cv::Scalar(0, 255, 0));
            cv::drawMarker(imageROIGrid, spotLocation.point, cv::Scalar(0, 0, 255));
        }
    }
    cv::line(imageROIGrid, cv::Point(0, imageInformations.landmark.point.y), cv::Point(imageROIGrid.cols, imageInformations.landmark.point.y), cv::Scalar(0, 0, 255));
    cv::line(imageROIGrid, cv::Point(imageInformations.landmark.point.x, 0), cv::Point(imageInformations.landmark.point.x, imageROIGrid.rows), cv::Scalar(0, 0, 255));

    std::string nameOriginalImage(imageInformations.name.begin() + 4, imageInformations.name.end());
    saveImage(pathROI, "Grid_" + nameOriginalImage, imageROIGrid);
    saveImage(pathROI, "Circles_" + nameOriginalImage, imageROICircles);
}

MultiplexResults MultiplexUseTestAnalyzer::computeResults(GridOfSpots gridOfSpots,
                                                          const cv::Mat& cropped)
{
    uint32_t heightROIAroundSpot = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels,
                                                                             _patternManager.getDefaultHeightNoiseROI());
    uint32_t widthROIAroundSpot =  ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels,
                                                                             _patternManager.getDefaultWidthNoiseROI());
    uint32_t radiusSpot =  ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels,
                                                                     _params.spotDiameter / 2.);
    uint32_t widthGaussianToConvol = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels,
                                                                               _patternManager.getHorizontalPitch() * 2);
    AlgoMultiplexConvolBrut algoMultiplexConvolBrut(heightROIAroundSpot, widthROIAroundSpot, radiusSpot);
    algoMultiplexConvolBrut.setWidthGaussianToConvol(widthGaussianToConvol);

    AlgoMultiplexResult convolBrutResult = algoMultiplexConvolBrut.processMultiplexAnalyze(cropped, gridOfSpots);

    MultiplexResults multiplexResults = {
            .convolBrutResult = convolBrutResult
    };

    return multiplexResults;
}

void MultiplexUseTestAnalyzer::analyzeImage(const ImageInformations& imageInformations,
                                            OpenXLSX::XLWorkbook& workbook)
{
    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(imageInformations.cropped, frameBGRChannels);
    cv::Mat croppedOneChannel = frameBGRChannels[_params.channel];

    GridOfSpots gridOfSpots = _patternManager.buildGridOfSpots(croppedOneChannel.size(),
                                                                  imageInformations.landmark,
                                                                  _params.referenceNumberOfPixels);

    MultiplexResults results = computeResults(gridOfSpots, croppedOneChannel);

    writeResults(workbook, results, imageInformations);
    saveImageWithROI(_patternManager.pattern.SpotsSpecies,
                     _patternManager.getDefaultWidthNoiseROI(),
                     _patternManager.getHorizontalPitch(),
                     imageInformations, _params.channel, gridOfSpots);
    saveImageBrutData(workbook,
                      gridOfSpots,
                      _patternManager.getDefaultWidthNoiseROI(),
                      _patternManager.getHorizontalPitch(),
                      imageInformations);
}

void MultiplexUseTestAnalyzer::saveImageBrutData(OpenXLSX::XLWorkbook& workbook,
                                                 const GridOfSpots& gridOfSpots,
                                                 double widthNoiseROI,
                                                 double widthTotalSquare,
                                                 const ImageInformations& imageInformations)
{
    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(imageInformations.cropped, frameBGRChannels);
    cv::Mat oneChannelImage = frameBGRChannels[_params.channel];

    uint32_t circleRadius = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.spotDiameter / 2);
    uint32_t widthSquareNoise = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, widthNoiseROI);
    uint32_t widthSquare = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, widthTotalSquare);

    std::vector<SpotLocation> spotsLocations;
    std::vector<uint32_t> totalSquareIntensities;
    std::vector<uint32_t> spotsIntensities;
    std::vector<uint32_t> noiseIntensities;

    for(auto element : gridOfSpots.SpotsCASCoordinates)
    {
        SpotLocation spotLocation = element.second;

        auto [totalSquareValue, spotValue, noiseValue]
        = ImageAnalyzer::computeSpotValues(oneChannelImage, spotLocation.point, circleRadius, widthSquareNoise, widthSquare);

        spotsLocations.push_back(spotLocation);
        totalSquareIntensities.push_back(totalSquareValue);
        spotsIntensities.push_back(spotValue);
        noiseIntensities.push_back(noiseValue);
    }

    for(auto element : gridOfSpots.SpotsTestCoordinates)
    {
        std::vector<SpotTest> column = element.second;
        for(auto [spotLocation, spotType] : column)
        {
            auto[totalSquareValue, spotValue, noiseValue]
            = ImageAnalyzer::computeSpotValues(oneChannelImage, spotLocation.point, circleRadius, widthSquareNoise, widthSquare);

            spotsLocations.push_back(spotLocation);
            totalSquareIntensities.push_back(totalSquareValue);
            spotsIntensities.push_back(spotValue);
            noiseIntensities.push_back(noiseValue);
        }
    }

    writeImageBrutData(workbook, spotsLocations, totalSquareIntensities, spotsIntensities, noiseIntensities, imageInformations.name, imageInformations.numImage - 1);
}

void MultiplexUseTestAnalyzer::processUseTestAnalyze()
{
    OpenXLSX::XLDocument file;
    file.open(_params.templateExcelFilePath.string());

    OpenXLSX::XLWorkbook workbook = file.workbook();
    workbook.setFullCalculationOnLoad();

    std::vector<std::string> croppedImagesNames = searchAllCroppedImagesNames(_params.croppedFolderPath);
    std::map<double, std::vector<std::string>> sortedNamesMap = initConcentrationListMultiplex(croppedImagesNames);

    writeSummaryHeaderMultiplex(workbook, _params.croppedFolderPath, _params.appVersion, _params.currentDate, _patternManager.pattern.SpotsSpecies);
    writeConcentrationListMultiplex(workbook, sortedNamesMap);
    writeCalibrationParamsMultiplex(workbook, _params);
    writePatternInformations(workbook,
                             _params.spotDiameter,
                             _patternManager.getVerticalPitch(),
                             _patternManager.getHorizontalPitch(),
                             _patternManager.getDefaultHeightNoiseROI(),
                             _patternManager.getDefaultWidthNoiseROI());
    uint32_t numImage = 0;
    for(auto& element : sortedNamesMap)
    {
        double concentration = element.first;
        for(auto& name : element.second)
        {
            std::filesystem::path imagePath = _params.croppedFolderPath;
            imagePath /= name;

            cv::Mat cropped = ImageAnalyzer::readImage(imagePath);

            ImageInformations imageInformations = {
                    .cropped = cropped,
                    .name = name,
                    .concentration = concentration,
                    .landmark = _landmarks[numImage],
                    .numImage = numImage + 1
            };

            analyzeImage(imageInformations, workbook);

            numImage++;
        }
    }

    file.saveAs(_params.excelFilePath.string());
    file.close();
}