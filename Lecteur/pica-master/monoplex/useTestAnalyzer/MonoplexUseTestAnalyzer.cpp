
#include "MonoplexUseTestAnalyzer.h"
#include "ConcentrationPredictor.h"

#include "common/imageProcessing/ImagePreProcessing.h"
#include "ProxNoiseGrids.h"
#include "RegulatoryDataManager.h"


MonoplexUseTestAnalyzer::MonoplexUseTestAnalyzer(const std::vector<std::string>& paramsWithoutImagePath,
                                                 const std::filesystem::path& croppedFolderPath,
                                                 const std::filesystem::path& templateExcelFilePath,
                                                 const std::filesystem::path& excelFilePath,
                                                 const std::filesystem::path& excelFileCalibrationCurves)
{
    _params.croppedFolderPath = croppedFolderPath;
    _params.templateExcelFilePath = templateExcelFilePath;
    _params.excelFilePath = excelFilePath;
    _params.excelFileCalibrationCurves = excelFileCalibrationCurves;

    _params.appVersion = paramsWithoutImagePath[0];
    _params.currentDate = paramsWithoutImagePath[1];
    _params.measure = paramsWithoutImagePath[2];
    _params.proto = paramsWithoutImagePath[3];
    _params.cassette = paramsWithoutImagePath[4];
    _params.gamme = paramsWithoutImagePath[5];
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

    _params.lengthROIMm = std::stod(paramsWithoutImagePath[10]);
    _params.rectangleWidthMm = std::stod(paramsWithoutImagePath[11]);
    _params.rectangleWidth = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.rectangleWidthMm);
    _params.maskLengthMm = std::stod(paramsWithoutImagePath[12]);
    _params.maskLength = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.maskLengthMm);

    _params.theoricDistanceCLtoTLMm = std::stod(paramsWithoutImagePath[13]);
    _params.TLLengthMm = std::stod(paramsWithoutImagePath[14]);
    _params.proxNoiseTLSize = std::stod(paramsWithoutImagePath[15]);
    _params.proxNoiseNoiseSize = std::stod(paramsWithoutImagePath[16]);
}

std::vector<std::string> MonoplexUseTestAnalyzer::searchAllCroppedImagesNames(std::filesystem::path croppedFolderPath)
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

void MonoplexUseTestAnalyzer::processUseTestAnalyze()
{
    OpenXLSX::XLDocument file;
    file.open(_params.templateExcelFilePath.string());

    OpenXLSX::XLWorkbook workbook = file.workbook();
    workbook.setFullCalculationOnLoad();
    writeSummaryHeader(workbook, _params.croppedFolderPath, _params.appVersion, _params.currentDate);

    std::vector<std::string> croppedImagesNames = searchAllCroppedImagesNames(_params.croppedFolderPath);
    std::map<double, std::vector<std::string>> sortedNamesMap = initConcentrationList(croppedImagesNames);
    int biggestConcentration = (sortedNamesMap.rbegin())-> first;

    writeConcentrationList(workbook, sortedNamesMap);

    AllProfiles allProfiles = computeAllProfiles(sortedNamesMap);
    writeCalibrationParams(workbook, _params);

    AllBestParamsData bestParamsData = ProxNoiseGrids::computeAndWriteAllGrids(sortedNamesMap,
                                                                               _params,
                                                                               allProfiles,
                                                                               workbook,
                                                                               biggestConcentration,
                                                                               false);
    writeAllBestParams(workbook, bestParamsData, biggestConcentration, false);

    uint32_t numImage = 1;
    int theoricTestLine = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.theoricDistanceCLtoTLMm);

    std::map<double, std::vector<std::vector<Results>>> allImagesResults;
    uint32_t numberOfTL = 3;

    for(auto& element : sortedNamesMap)
    {
        double concentration = element.first;
        allImagesResults[concentration] = {{}, {}, {}};
        for(auto& name : element.second)
        {
            cv::Mat cropped = allProfiles.croppedImages[numImage - 1];

            for(int numTL = 1; numTL <= numberOfTL; numTL++)
                analyzeImage(cropped, concentration, name, numImage, numTL, workbook, bestParamsData, allProfiles, allImagesResults);

            int findedTL = allProfiles.distancesCLtoTL[numImage-1][1];
            int differenceInPx = std::abs(theoricTestLine - findedTL);
            double differenceInMm = std::round(ImageAnalyzer::convertPixelsToMillimeters(_params.referenceNumberOfPixels, differenceInPx) * 100)/100.;

            writeFindTLInformations(workbook, name, numImage, differenceInMm, differenceInPx);
            numImage++;
        }
    }

    ConcentrationPredictor::processConcentrationPrediction(workbook, allImagesResults, _params.excelFileCalibrationCurves);
    ConcentrationPredictor::generateLinearRegressionCoefficients(workbook, allImagesResults);

    std::vector<ResultsByAlgo> allTLResultsByAlgo;
    for(int numTL = 1; numTL <= numberOfTL; numTL++)
        allTLResultsByAlgo.push_back(sortResultsByAlgo(allImagesResults, numTL));

    ROCCurveManager::processROCCurves(workbook, allTLResultsByAlgo[1]);
    RegulatoryDataManager::processRegulatoryData(workbook, allTLResultsByAlgo);

    file.saveAs(_params.excelFilePath.string());
    file.close();
}

std::tuple<TImageProfile, TImageProfile, uint32_t, double, double> MonoplexUseTestAnalyzer::cutAndFitProfile(TImageProfile& profile,
                                                                                                             uint32_t controlLine,
                                                                                                             uint32_t testLine,
                                                                                                             ParamsToAnalyze params,
                                                                                                             Algorithms& algorithms)
{
    auto [startCutProfile, endCutProfile] = ImageAnalyzer::computeCutProfileBorders(profile,
                                                                                    params.minCLSearchArea,
                                                                                    params.maxCLSearchArea,
                                                                                    controlLine,
                                                                                    testLine,
                                                                                    params.rectangleWidth,
                                                                                    params.maskLength / 2);
    std::vector<double> cutProfile(profile.begin() + startCutProfile,
                                   profile.begin() + endCutProfile);

    controlLine -= startCutProfile;
    testLine -= startCutProfile;

    auto [expoFitProfileFitted, expoFitCoefficient] = algorithms.algoExponantialFit.transformProfileForProcessing(
            cutProfile, {controlLine,
                         testLine});

    auto [poly2ProfileFitted, poly2Coefficient] = algorithms.algoPoly2.transformProfileForProcessing(cutProfile,
                                                                                                     {controlLine,
                                                                                                      testLine});
    return {expoFitProfileFitted, poly2ProfileFitted, controlLine, expoFitCoefficient, poly2Coefficient};
}

AllProfiles MonoplexUseTestAnalyzer::computeAllProfiles(std::map<double, std::vector<std::string>>& sortedNamesMap)
{
    AllProfiles allProfiles;

    for(auto& element : sortedNamesMap)
    {
        for(auto& name : element.second)
        {
            std::filesystem::path imagePath = _params.croppedFolderPath;
            imagePath /= name;
            cv::Mat cropped = ImageAnalyzer::readImage(imagePath);

            _params.minCLSearchArea = 10;
            _params.maxCLSearchArea = cropped.rows / 3;

            uint32_t lengthCenterPx = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.lengthROIMm);
            int theoricPadding = cropped.cols - lengthCenterPx;
            if(theoricPadding < 2)
                theoricPadding = 2;

            _params.padding = theoricPadding;

            const auto [oneChannelImage, stripCroppedPadding] = ImageAnalyzer::extractChannelImageWithPadding(cropped, _params.channel, _params.padding);

            std::vector<double> stripProfile = ImageAnalyzer::computeImageProfile(stripCroppedPadding, _params.isVisible, _params.channel);
            uint32_t theoricTLLengthPx = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.TLLengthMm);
            uint32_t controlLine = ImageAnalyzer::findControlLine(stripProfile, theoricTLLengthPx, _params.minCLSearchArea, _params.maxCLSearchArea);

            allProfiles.controlLinesNotCuttedPos.push_back(controlLine);

            uint32_t distanceCLtoTL = ImageAnalyzer::initDistanceCLtoTL(_params.theoricDistanceCLtoTLMm, _params.TLLengthMm, _params.referenceNumberOfPixels, controlLine, stripCroppedPadding, _params.channel);
            uint32_t marginBetweenTLs = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, 0.15);

            allProfiles.distancesCLtoTL.push_back({distanceCLtoTL - marginBetweenTLs, distanceCLtoTL, distanceCLtoTL + marginBetweenTLs});

            uint32_t testLine = controlLine + distanceCLtoTL;

            ProcessorConfig processorConfig(cropped,
                                            _params.isVisible,
                                            _params.channel,
                                            _params.lengthROIMm,
                                            _params.theoricDistanceCLtoTLMm,
                                            _params.TLLengthMm,
                                            _params.referenceNumberOfPixels,
                                            _params.rectangleWidthMm,
                                            _params.maskLengthMm);

            Algorithms algorithms;
            algorithms.setConfig(processorConfig);

            uint32_t widthGaussianPx = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, ImagePreProcessing::DEFAULT_WIDTH_GAUSSIAN_CONVOL_TL_MM);
            TImageProfile convolProfile = ImagePreProcessing::convolImageAndComputeProfile(oneChannelImage, widthGaussianPx);

            auto [expoFitProfile, poly2Profile, controlLineCutted, expoFitCoeff, poly2Coeff] = cutAndFitProfile(stripProfile,
                                                                                                                controlLine,
                                                                                                                testLine,
                                                                                                                _params,
                                                                                                                algorithms);
            auto [convolExpoFitProfile, convolPoly2Profile, convolControlLineCutted, convolExpoFitCoeff, convolPoly2Coeff] = cutAndFitProfile(convolProfile,
                                                                                                                                              controlLine,
                                                                                                                                              testLine,
                                                                                                                                              _params,
                                                                                                                                              algorithms);
            allProfiles.croppedImages.push_back(cropped.clone());

            allProfiles.rawProfiles.push_back(stripProfile);
            allProfiles.expoFitProfilesFitted.push_back(expoFitProfile);
            allProfiles.poly2ProfilesFitted.push_back(poly2Profile);
            allProfiles.controlLinesCuttedPos.push_back(controlLineCutted);
            allProfiles.expoFitCoefficients.push_back(expoFitCoeff);
            allProfiles.poly2Coefficients.push_back(poly2Coeff);

            allProfiles.convolProfiles.push_back(convolProfile);
            allProfiles.convolProfilesExpoFit.push_back(convolExpoFitProfile);
            allProfiles.convolProfilesPoly2.push_back(convolPoly2Profile);
            allProfiles.convolControlLinesCuttedPos.push_back(convolControlLineCutted);
            allProfiles.convolExpoFitCoefficients.push_back(convolExpoFitCoeff);
            allProfiles.convolPoly2Coefficients.push_back(convolPoly2Coeff);
        }
    }

    return allProfiles;
}

Results MonoplexUseTestAnalyzer::computeResults(ProcessorConfig processorConfig,
                                                const AllBestParamsData& bestParamsData,
                                                AllProfiles& allProfiles,
                                                uint32_t numTL,
                                                uint32_t indexOfProfile)
{
    uint32_t controlLine = allProfiles.controlLinesNotCuttedPos[indexOfProfile];
    uint32_t controlLineCutted = allProfiles.controlLinesCuttedPos[indexOfProfile];
    uint32_t convolControlLineCutted = allProfiles.convolControlLinesCuttedPos[indexOfProfile];
    uint32_t testLineDistance = allProfiles.distancesCLtoTL[indexOfProfile][numTL-1];

    TImageProfile stripProfile = allProfiles.rawProfiles[indexOfProfile];
    TImageProfile convolStripProfile = allProfiles.convolProfiles[indexOfProfile];
    TImageProfile convolExpoFitProfile = allProfiles.convolProfilesExpoFit[indexOfProfile];
    TImageProfile convolPoly2Profile = allProfiles.convolProfilesPoly2[indexOfProfile];

    Algorithms algorithms;
    algorithms.setConfig(processorConfig);

    uint32_t defaultNewTestLineLength = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.proxNoiseTLSize);
    uint32_t defaultNewNoiseLength = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.proxNoiseNoiseSize);

    algorithms.algoProxNoise.setROILengths(defaultNewTestLineLength, defaultNewNoiseLength, defaultNewNoiseLength);
    algorithms.algoExponantialFit.setROILengths(defaultNewTestLineLength, defaultNewNoiseLength, defaultNewNoiseLength);
    algorithms.algoPoly2.setROILengths(defaultNewTestLineLength, defaultNewNoiseLength, defaultNewNoiseLength);
    algorithms.algoTestMinusProxNoise.setROILengths(defaultNewTestLineLength, defaultNewNoiseLength, defaultNewNoiseLength);

    Results results;
    ProxNoiseParams proxNoiseBestParams = bestParamsData.allProxNoiseBestParams[numTL - 1];
    ProxNoiseParams expoFitProxNoiseBestParams  = bestParamsData.allExpoFitBestParams[numTL - 1];
    ProxNoiseParams poly2ProxNoiseBestParams  = bestParamsData.allPoly2BestParams[numTL - 1];
    ProxNoiseParams testMinusProxNoiseBestParams  = bestParamsData.allTestMinusNoiseBestParams[numTL - 1];
    ProxNoiseParams convolProxNoiseBestParams = bestParamsData.allConvolProxNoiseBestParams[numTL - 1];
    ProxNoiseParams convolExpoFitProxNoiseBestParams  = bestParamsData.allConvolExpoFitBestParams[numTL - 1];
    ProxNoiseParams convolPoly2ProxNoiseBestParams  = bestParamsData.allConvolPoly2BestParams[numTL - 1];
    ProxNoiseParams convolTestMinusNoiseBestParams = bestParamsData.allConvolTestMinusNoiseBestParams[numTL - 1];

    //-------------------
    results.proxNoise = algorithms.algoProxNoise.processImageProfile(stripProfile, { controlLine,
                                                                                     controlLine + testLineDistance});
    results.convolProxNoise = algorithms.algoProxNoise.processImageProfile(convolStripProfile, { controlLine,
                                                                                                 controlLine + testLineDistance});
    //-------------------
    auto expoFitProfileFitted = allProfiles.expoFitProfilesFitted[indexOfProfile];

    results.expoFitProxNoise = algorithms.algoExponantialFit.processImageProfile(expoFitProfileFitted, { controlLineCutted,
                                                                                                         controlLineCutted + testLineDistance});
    results.convolExpoFitProxNoise = algorithms.algoExponantialFit.processImageProfile(convolExpoFitProfile, { convolControlLineCutted,
                                                                                                               convolControlLineCutted + testLineDistance});
    std::get<1>(results.expoFitProxNoise) = allProfiles.expoFitCoefficients[indexOfProfile];

    //-------------------
    auto poly2ProfileFitted = allProfiles.poly2ProfilesFitted[indexOfProfile];

    results.poly2ProxNoise = algorithms.algoPoly2.processImageProfile(poly2ProfileFitted, { controlLineCutted,
                                                                                            controlLineCutted + testLineDistance});
    results.convolPoly2ProxNoise = algorithms.algoPoly2.processImageProfile(convolPoly2Profile, { convolControlLineCutted,
                                                                                                  convolControlLineCutted + testLineDistance});
    std::get<1>(results.poly2ProxNoise) = allProfiles.poly2Coefficients[indexOfProfile];
    //-------------------
    results.testMinusProxNoise = algorithms.algoTestMinusProxNoise.processImageProfile(stripProfile, { controlLine,
                                                                                                       controlLine + testLineDistance});
    results.convolTestMinusProxNoise = algorithms.algoTestMinusProxNoise.processImageProfile(convolStripProfile,{ controlLine,
                                                                                                                  controlLine + testLineDistance});
    //-------------------
    algorithms.algoProxNoise.setROILengths(std::get<0>(proxNoiseBestParams.proxNoiseLenghts),std::get<1>(proxNoiseBestParams.proxNoiseLenghts),std::get<1>(proxNoiseBestParams.proxNoiseLenghts));
    results.proxNoiseBestParams = algorithms.algoProxNoise.processImageProfile(stripProfile, { controlLine,
                                                                                               controlLine + testLineDistance});

    algorithms.algoProxNoise.setROILengths(std::get<0>(convolProxNoiseBestParams.proxNoiseLenghts),std::get<1>(convolProxNoiseBestParams.proxNoiseLenghts),std::get<1>(convolProxNoiseBestParams.proxNoiseLenghts));
    results.convolProxNoiseBestParams = algorithms.algoProxNoise.processImageProfile(convolStripProfile, { controlLine,
                                                                                                           controlLine + testLineDistance});
    //-------------------
    algorithms.algoExponantialFit.setROILengths(std::get<0>(expoFitProxNoiseBestParams.proxNoiseLenghts),std::get<1>(expoFitProxNoiseBestParams.proxNoiseLenghts),std::get<1>(expoFitProxNoiseBestParams.proxNoiseLenghts));
    results.expoFitProxNoiseBestParams = algorithms.algoExponantialFit.processImageProfile(expoFitProfileFitted, { controlLineCutted,
                                                                                                                   controlLineCutted + testLineDistance});

    algorithms.algoExponantialFit.setROILengths(std::get<0>(convolExpoFitProxNoiseBestParams.proxNoiseLenghts),std::get<1>(convolExpoFitProxNoiseBestParams.proxNoiseLenghts),std::get<1>(convolExpoFitProxNoiseBestParams.proxNoiseLenghts));
    results.convolExpoFitProxNoiseBestParams = algorithms.algoExponantialFit.processImageProfile(convolExpoFitProfile, { convolControlLineCutted,
                                                                                                                         convolControlLineCutted + testLineDistance});

    //-------------------
    algorithms.algoPoly2.setROILengths(std::get<0>(poly2ProxNoiseBestParams.proxNoiseLenghts),std::get<1>(poly2ProxNoiseBestParams.proxNoiseLenghts),std::get<1>(poly2ProxNoiseBestParams.proxNoiseLenghts));
    results.poly2ProxNoiseBestParams = algorithms.algoPoly2.processImageProfile(poly2ProfileFitted, { controlLineCutted,
                                                                                                      controlLineCutted + testLineDistance});

    algorithms.algoPoly2.setROILengths(std::get<0>(convolPoly2ProxNoiseBestParams.proxNoiseLenghts),std::get<1>(convolPoly2ProxNoiseBestParams.proxNoiseLenghts),std::get<1>(convolPoly2ProxNoiseBestParams.proxNoiseLenghts));
    results.convolPoly2ProxNoiseBestParams = algorithms.algoPoly2.processImageProfile(convolPoly2Profile, { convolControlLineCutted,
                                                                                                            convolControlLineCutted + testLineDistance});
    //-------------------
    algorithms.algoTestMinusProxNoise.setROILengths(std::get<0>(testMinusProxNoiseBestParams.proxNoiseLenghts),std::get<1>(testMinusProxNoiseBestParams.proxNoiseLenghts),std::get<1>(testMinusProxNoiseBestParams.proxNoiseLenghts));
    results.testMinusProxNoiseBestParams = algorithms.algoTestMinusProxNoise.processImageProfile(stripProfile, { controlLine,
                                                                                                                 controlLine + testLineDistance});
    //-------------------
    algorithms.algoTestMinusProxNoise.setROILengths(std::get<0>(convolTestMinusNoiseBestParams.proxNoiseLenghts),std::get<1>(convolTestMinusNoiseBestParams.proxNoiseLenghts),std::get<1>(convolTestMinusNoiseBestParams.proxNoiseLenghts));
    results.convolTestMinusProxNoiseBestParams = algorithms.algoTestMinusProxNoise.processImageProfile(convolStripProfile, { controlLine,
                                                                                                                             controlLine + testLineDistance});

    auto [rawControl, rawTest, rawNoiseCL, rawNoiseTL] =
    ImageAnalyzer::computeRawControlTestNoise(stripProfile, {controlLine, controlLine + testLineDistance}, defaultNewTestLineLength, controlLine + testLineDistance / 2, controlLine + testLineDistance / 2);

    results.rawControl = rawControl;
    results.rawTest = rawTest;
    results.rawNoise = rawNoiseCL;

    return results;
}

void MonoplexUseTestAnalyzer::analyzeImage(const cv::Mat& cropped,
                                           double concentration,
                                           const std::string& nameImage,
                                           uint32_t numImage,
                                           uint32_t numTL,
                                           OpenXLSX::XLWorkbook& workbook,
                                           const AllBestParamsData& bestParamsData,
                                           AllProfiles& allProfiles,
                                           std::map<double, std::vector<std::vector<Results>>>& allImagesResults)
{
    uint32_t indexOfProfile = numImage - 1;

    ProcessorConfig processorConfig(cropped,
                                    _params.isVisible,
                                    _params.channel,
                                    _params.lengthROIMm,
                                    _params.theoricDistanceCLtoTLMm,
                                    _params.TLLengthMm,
                                    _params.referenceNumberOfPixels,
                                    _params.rectangleWidthMm,
                                    _params.maskLengthMm);

    Results results = computeResults(processorConfig,
                                     bestParamsData,
                                     allProfiles,
                                     numTL,
                                     indexOfProfile);

    std::string nameOriginalImage(nameImage.begin() + 4, nameImage.end());

    if(numTL == 1)
        writeAdjustedProfile(workbook, allProfiles.rawProfiles[indexOfProfile], numImage, nameOriginalImage, allProfiles.controlLinesNotCuttedPos, allProfiles.controlLinesNotCuttedPos[indexOfProfile]);
    if(numTL == 2)
        saveImageWithROI(workbook, cropped, numImage, nameOriginalImage, allProfiles.controlLinesNotCuttedPos[indexOfProfile], allProfiles.distancesCLtoTL[indexOfProfile], _params.channel);
    writeResults(workbook, results, nameOriginalImage, numImage, numTL, concentration);
    writeFitCoefficients(workbook, results, nameOriginalImage, numImage, numTL, concentration);

    results.numImage = numImage;
    results.nameImage = nameOriginalImage;
    results.numTL = numTL;

    allImagesResults[concentration][numTL-1].push_back(results);
}

void MonoplexUseTestAnalyzer::saveImageWithROI(OpenXLSX::XLWorkbook& workbook,
                                               const cv::Mat cropped,
                                               uint32_t numImage,
                                               const std::string& nameOriginalImage,
                                               uint32_t controlLine,
                                               std::vector<uint32_t>& testLines,
                                               ColorChannel channel)
{
    std::string nameExcel = _params.excelFilePath.filename().string();
    std::string prefix;
    if(nameExcel.find("_Rectangle") != -1)
        prefix = "Results/Loops/ROI_";
    else
        prefix = "Results/ROI_";

    std::string nameWithoutExtension = _params.excelFilePath.stem().string();
    int underscorePosition = nameWithoutExtension.find('_', 8);
    std::string suffix(nameWithoutExtension.begin() + underscorePosition + 1, nameWithoutExtension.end());

    std::filesystem::path pathROI = _params.croppedFolderPath;
    pathROI.replace_filename(prefix + suffix);

    if(numImage == 1)
    {
        if(!std::filesystem::exists(pathROI))
            std::filesystem::create_directory(pathROI);
        writeROIFolderPath(workbook, pathROI);
    }

    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(cropped, frameBGRChannels);

    cv::Mat oneChannelImage;
    cv::cvtColor(frameBGRChannels[channel], oneChannelImage, cv::COLOR_GRAY2BGR);

    cv::Rect roiSearchArea(_params.padding/2, _params.minCLSearchArea, cropped.cols - _params.padding, _params.maxCLSearchArea - _params.minCLSearchArea);
    cv::rectangle(oneChannelImage, roiSearchArea, cv::Scalar(255, 255, 0));

    cv::Rect roiCL(_params.padding/2, controlLine, cropped.cols - _params.padding, 1);
    cv::rectangle(oneChannelImage, roiCL, cv::Scalar(0, 0, 255));

    for(auto& testLine : testLines)
    {
        cv::Rect roiTL(_params.padding/2, testLine + controlLine, cropped.cols - _params.padding, 1);
        cv::rectangle(oneChannelImage, roiTL, cv::Scalar(0, 255, 0));
    }
    uint32_t theoricTestLine = ImageAnalyzer::convertMillimetersToPixels(_params.referenceNumberOfPixels, _params.theoricDistanceCLtoTLMm);
    cv::Rect roiTL(_params.padding/2, theoricTestLine + controlLine, cropped.cols - _params.padding, 1);
    cv::rectangle(oneChannelImage, roiTL, cv::Scalar(255, 0, 0));

    pathROI /= nameOriginalImage;

    std::vector<uint8_t> buffer;
    imencode(".png", oneChannelImage,buffer);

    std::fstream  outputFile(pathROI.c_str(), std::ios::out | std::ios::binary);
    if(outputFile)
    {
        outputFile.write((const char*)&buffer[0], buffer.size());
        outputFile.close();
    }
}

ResultsByAlgo MonoplexUseTestAnalyzer::sortResultsByAlgo(std::map<double, std::vector<std::vector<Results>>> allImagesResults, int numTL)
{
    ResultsByAlgo resultsByAlgo;
    std::vector<std::vector<double>> brutResults(5);

    for(auto& [concentration, resultsByTL] : allImagesResults)
    {
        std::vector<Results> TLResults = resultsByTL[numTL - 1];
        for(auto& imageResults : TLResults)
        {
            resultsByAlgo.brutResults[concentration].push_back(std::get<0>(imageResults.proxNoise));
            resultsByAlgo.expoFitResults[concentration].push_back(std::get<0>(imageResults.expoFitProxNoise));
            resultsByAlgo.poly2Results[concentration].push_back(std::get<0>(imageResults.poly2ProxNoise));
            resultsByAlgo.testMinusNoiseResults[concentration].push_back(std::get<0>(imageResults.testMinusProxNoise));
            resultsByAlgo.convolBrutResults[concentration].push_back(std::get<0>(imageResults.convolProxNoise));
            resultsByAlgo.convolExpoFitResults[concentration].push_back(std::get<0>(imageResults.convolExpoFitProxNoise));
            resultsByAlgo.convolPoly2Results[concentration].push_back(std::get<0>(imageResults.convolPoly2ProxNoise));
            resultsByAlgo.convolTestMinusNoiseResults[concentration].push_back(std::get<0>(imageResults.convolTestMinusProxNoise));
        }
    }
    return resultsByAlgo;
}