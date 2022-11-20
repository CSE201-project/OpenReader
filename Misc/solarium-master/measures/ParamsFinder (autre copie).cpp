//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/06/09.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include <map>
#include <thread>
#include <chrono>
#include <StripCropperRectangleDetection.h>
#include <repository/StudiesRepository.h>
#include <tools/Converter.h>
#include <IImageProcessor.h>
#include <AlgoProxNoise.h>
#include <AlgoExponantialFit.h>
#include <AlgoPoly2.h>
#include <AlgoTestMinusProxNoise.h>
#include "ParamsFinder.h"
#include "MeasureConfig.h"
#include "configure.h"

#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Format.h>
#include <SolariumApp.h>

#define DEFAULT_ROI_LENGTH 2

cv::Mat drawCTAndTLROI(const cv::Mat & redChannelImage, double oneMmLengthInPixel, int controlLinePos, int testLinePos)
{
    cv::Mat rgbPicture(redChannelImage.rows, redChannelImage.cols, CV_8UC3);
    cv::cvtColor(redChannelImage, rgbPicture, cv::COLOR_GRAY2BGR);

    auto noiseLines = {
            (int)(controlLinePos - oneMmLengthInPixel),
            (int)(controlLinePos - std::ceil(oneMmLengthInPixel/2)),
            (int)(controlLinePos + std::ceil(oneMmLengthInPixel/2)+1),
            (int)(controlLinePos + oneMmLengthInPixel),
            (int)(testLinePos - oneMmLengthInPixel),
            (int)(testLinePos - std::ceil(oneMmLengthInPixel/2)),
            (int)(testLinePos + std::ceil(oneMmLengthInPixel/2)+1),
            (int)(testLinePos + oneMmLengthInPixel),
    };

    auto testLines = {
            (int)(controlLinePos - std::ceil(oneMmLengthInPixel/2)+1),
            (int)(controlLinePos + std::ceil(oneMmLengthInPixel/2)),
            (int)(testLinePos - std::ceil(oneMmLengthInPixel/2)+1),
            (int)(testLinePos + std::ceil(oneMmLengthInPixel/2))
    };

    try {
        for (int i = 0; i < rgbPicture.rows; i++)
        {
            cv::Vec3b noiseROIColor(0, 255, 0);
            for (auto nl : noiseLines)
            {
                auto col = nl < rgbPicture.cols ? nl : rgbPicture.cols - 1;
                rgbPicture.at<cv::Vec3b>(cv::Point(nl, i)) = noiseROIColor;
            }
            cv::Vec3b testROIColor(0, 0, 255);
            for (auto tl : testLines)
            {
                auto col = tl < rgbPicture.cols ? tl : rgbPicture.cols - 1;
                rgbPicture.at<cv::Vec3b>(cv::Point(tl, i)) = testROIColor;
            }
        }
    }
    catch (const std::exception & ex) {
        poco_debug_f1(LOGGER, "drawCTAndTLROI error on %s", std::string(ex.what()));
    }
    return rgbPicture;
}

std::vector<ICamera::TCameraParams> computeAcquisitionParamsCombination(std::map<std::string, ParamsFinder::TLoopValues> inputParams)
{
    std::vector<ICamera::TCameraParams> results;

    std::vector<std::string> mapKeys;
    std::vector<int> indexes, indexesLimit;
    for(const auto & kv : inputParams)
    {
        if (kv.second.size())
        {
            mapKeys.push_back(kv.first);
            indexes.push_back(0);
            indexesLimit.push_back(kv.second.size());
        }
    }

    while (indexes.at(0) < indexesLimit.at(0))
    {
        ICamera::TCameraParams currentParams;

        for (int i = 0; i < indexes.size(); i++)
            currentParams[mapKeys.at(i)] = std::to_string(inputParams[mapKeys.at(i)].at(indexes.at(i)));

        results.push_back(currentParams);

        for (int i = indexes.size() - 1; i >= 0; i--)
        {
            int nextIndex = indexes.at(i) + 1;
            if (nextIndex < indexesLimit.at(i) || i == 0)
            {
                indexes.at(i) = nextIndex;
                break;
            }
            else
                indexes.at(i) = 0;
        }
    }

    return results;
}

double getROIMean(ParamsFinder::ROI roi, const cv::Mat & picture)
{
    int32_t minX = roi.topLeft.x >= 0 ? roi.topLeft.x : 0, maxX = roi.bottomRight.x < picture.cols ? roi.bottomRight.x : (picture.cols-1);
    int32_t minY = roi.topLeft.y >= 0 ? roi.topLeft.y : 0, maxY = roi.bottomRight.y < picture.rows ? roi.bottomRight.y : (picture.rows-1);
    double sum = 0;
    int32_t count = 0;
    for (int32_t x = minX; x <= maxX; x++)
    {
        for (int32_t y = minY; y <= maxY; y++)
        {
            auto redValue = picture.at<cv::Vec3b>(y, x)[2];
            sum = sum + redValue;
            count++;
        }
    }
    return sum/count;
}

std::pair<double, double> getRoiAndProxNoiseMean(const cv::Mat & windowCassette, double oneMmLengthInPixel, uint32_t testLinePos)
{
    ParamsFinder::ROI roi = {0};
    roi.bottomRight.x = windowCassette.cols - 1;

    auto noiseBeforePos = testLinePos - oneMmLengthInPixel;
    roi.topLeft.y = noiseBeforePos;
    roi.bottomRight.y = noiseBeforePos + oneMmLengthInPixel/2;
    auto noiseBeforeValue = getROIMean(roi, windowCassette);

    roi.topLeft.y = testLinePos - oneMmLengthInPixel/2;
    roi.bottomRight.y = testLinePos + oneMmLengthInPixel/2;
    auto tlValue = getROIMean(roi, windowCassette);

    auto noiseAfterPos = testLinePos + oneMmLengthInPixel/2;
    roi.topLeft.y = noiseAfterPos;
    roi.bottomRight.y = noiseAfterPos + oneMmLengthInPixel/2;
    auto noiseAfterValue = getROIMean(roi, windowCassette);

    auto proxNoise = (noiseBeforeValue + noiseAfterValue) / 2;

    return { tlValue, proxNoise };
}

std::tuple<std::vector<double>, uint32_t, uint32_t, std::vector<double>> computeAlgos(const cv::Mat &windowCassette,
                                                                                      const ProcessorConfig & processorDefaultConfig,
                                                                                      const AlgoConfig & algoConfig,
                                                                                      const std::optional<std::pair<uint32_t, uint32_t>> & linesPos = {})
{
    if (windowCassette.empty())
    {
        return {};
    }

    ProcessorConfig processorConfig = processorDefaultConfig;
    processorConfig.croppedImage = windowCassette;

    auto oneMmLengthInPixel = processorConfig.referenceNumberOfPixels;


    std::vector<std::shared_ptr<IImageProcessor>> algos = {
            std::make_shared<AlgoProxNoise>(),
            std::make_shared<AlgoExponantialFit>(),
            std::make_shared<AlgoPoly2>(),
            std::make_shared<AlgoTestMinusProxNoise>()
    };
    std::vector<double> resultsAlgos;
    ParamsFinder::ROI roi = {0};
    roi.bottomRight.x = windowCassette.cols - 1;

    uint32_t testLinePos, controlLinePos;
    std::vector<double> profile;

    std::optional<uint32_t> ctPos = linesPos.has_value() ? std::make_optional(linesPos->first) : std::nullopt;
    std::optional<uint32_t> tlPos = linesPos.has_value() ? std::make_optional(linesPos->second) : std::nullopt;

    for(auto & algo : algos)
    {
        try
        {
            auto res = algo->compute(processorConfig, algoConfig, ctPos, tlPos);
            resultsAlgos.push_back(res.result);
            testLinePos = res.testLinePos;
            controlLinePos = res.controlLinePos;
            if (profile.empty())
                profile = res.RawProfile;
        }
        catch (const std::exception & ex)
        {
        }
    }

    auto noiseBetweenPos = controlLinePos + (testLinePos - controlLinePos)/2;
    roi.topLeft.y = noiseBetweenPos - oneMmLengthInPixel/2;
    roi.bottomRight.y = noiseBetweenPos + oneMmLengthInPixel/2;
    auto noiseBetweenValue = getROIMean(roi, windowCassette);

    auto [tlValue, proxNoiseTl] = getRoiAndProxNoiseMean(windowCassette, oneMmLengthInPixel, testLinePos);
    auto [clValue, proxNoiseCl] = getRoiAndProxNoiseMean(windowCassette, oneMmLengthInPixel, controlLinePos);

    resultsAlgos.push_back(noiseBetweenValue);

    resultsAlgos.push_back(proxNoiseCl);
    resultsAlgos.push_back(clValue);
    resultsAlgos.push_back(clValue - proxNoiseCl);
    resultsAlgos.push_back(proxNoiseTl);
    resultsAlgos.push_back(tlValue);
    resultsAlgos.push_back(tlValue - proxNoiseTl);

    return {resultsAlgos, controlLinePos, testLinePos, profile};
}

cv::Mat getRedChannelAndRotateLeft(const cv::Mat & verticalPicture)
{
    std::vector<cv::Mat> frameBGRChannels(3);
    cv::split(verticalPicture, frameBGRChannels);

    cv::Mat result;
    cv::rotate(frameBGRChannels.at(2), result, cv::ROTATE_90_COUNTERCLOCKWISE);

    return result;
}

ParamsFinder::ParamsFinder(const std::string &name, const std::string & cassetteName)
    : _name(name)
    , _cassetteName(cassetteName)
{

}

ParamsFinder::~ParamsFinder()
{

}

std::pair<cv::Mat,std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult>> ParamsFinder::takeVerticalSnapshot(std::map<std::string, std::string> cameraParams, LEDParams uvLeds, LEDParams visibleLeds)
{
    auto futurePictureAndSensors = SolariumDevice::instance().takePictureAndLightSensorsForMeasure({
                                                                                  cameraParams,
                                                                                  uvLeds,
                                                                                  visibleLeds
                                                                          });
    futurePictureAndSensors.wait();
    auto [ pictureMat, sensors ] = futurePictureAndSensors.data();

    if (!pictureMat.empty())
    {
#ifdef USE_UEYE_CAMERA
        //
        cv::rotate(pictureMat, pictureMat, cv::ROTATE_90_COUNTERCLOCKWISE);
#elif defined(USE_V4L2_CAMERA)
        cv::rotate(pictureMat, pictureMat, cv::ROTATE_90_CLOCKWISE);
#endif
    }
    else
        throw std::runtime_error("Camera Capture failed !");
    return { pictureMat, sensors };
}

void ParamsFinder::initCropper()
{
    MeasureConfig config = MeasureConfig::LoadDefaults("default_crop_params");
    auto cameraParams = config.getCameraParams();
    auto uvLeds = config.getLEDParams(LEDParams::UV);
    auto visibleLeds = config.getLEDParams(LEDParams::Visible);
    bool isImageInAmbientLight = visibleLeds.enabled.has_value() && visibleLeds.enabled.value();

    auto [ snapshot, sensors ] = takeVerticalSnapshot(cameraParams, uvLeds, visibleLeds);
    _croppingImage = snapshot;

    //Following string will be sent by Workbench.
    CassetteData cassetteData = CassetteType::decodeCassetteName(_cassetteName);

    CropParams cropperParams = {
#ifdef  USE_V4L2_CAMERA
            .deviceType = Proto_v3,
#else
            .deviceType = Proto_v2_PreCropped,
#endif
            .imageDepth = config.getColorDepth() == 12 ? Bits12 : Bits8,
            .lightningCondition = isImageInAmbientLight ? Visible : UV,
            .cassetteModel = cassetteData.model,
            .referenceNumberOfPixels = config.getPixelsToMm(),
            .stripMatrix = cassetteData.stripMatrix,
            .deviceFocal = (DeviceFocal)config.getLensFocal().type()
    };

    _cropper = std::make_unique<StripCropperRectangleDetection>(cropperParams);

    StudiesRepository repository;
    repository.saveParamsFinder(_name, "cropper.png", Converter::cvMatToPng(_croppingImage));
}

std::pair<cv::Mat,std::pair<SolariumDevice::TLightResult, SolariumDevice::TLightResult>> ParamsFinder::captureWindowCassette(std::map<std::string, std::string> cameraParams, LEDParams uvLeds, LEDParams visibleLeds)
{
    auto [ snapshot, sensors ] = takeVerticalSnapshot(cameraParams, uvLeds, visibleLeds);
    cv::Mat windowCassette;
    try {
        windowCassette = _cropper->crop(_croppingImage, snapshot);
    }
    catch (const std::exception & ex){
        windowCassette = snapshot;
    }

    return { windowCassette, sensors };
}

std::tuple<int32_t, int32_t, std::vector<double>> ParamsFinder::takeControlSnapshot(const Poco::Timestamp & currentTime)
{
    MeasureConfig config = MeasureConfig::LoadDefaults("default_params");
    auto cameraParams = config.getCameraParams();
    auto uvLeds = config.getLEDParams(LEDParams::UV);
    auto visibleLeds = config.getLEDParams(LEDParams::Visible);

    auto [ croppedImage, sensors ] = captureWindowCassette(cameraParams, uvLeds, visibleLeds);
#ifdef USE_UEYE_CAMERA
    Poco::Timestamp timestampCapture;
#endif

    auto filename = Poco::DateTimeFormatter::format(currentTime, "control-%H%M%S.%i.png");

    StudiesRepository repository;
    repository.saveParamsFinder(_name, filename, Converter::cvMatToPng(getRedChannelAndRotateLeft(croppedImage)));

    ProcessorConfig processorDefaultConfig = {
            .isVisible = config.isImageInAmbientLight(),
            .channel = config.getColorChannel(),
            .theoricDistanceCLtoTLMm = config.getDistanceCL2TL(),
            .TLLengthMm = config.getTLLength(),
            .referenceNumberOfPixels = config.getPixelsToMm(),
            .maskLengthMm = config.getMaskLength()
    };

    AlgoConfig algoConfig = {
            .lengthROIMm = config.getPadding(),
            .proxNoiseROIMm = {0.5, 1, 0.5},
            .useConvolPreProcessing = false
    };
    auto [ algosResults, controlLinePos, testLinePos, profile ] = computeAlgos(croppedImage, processorDefaultConfig, algoConfig);

    if (!_linesPos.has_value())
        _linesPos = std::make_pair(controlLinePos, testLinePos);

#ifdef USE_UEYE_CAMERA
    {
        Poco::Timespan timespan(0, 500 * 1000);
        auto coolingTime = timespan.totalMicroseconds() - timestampCapture.elapsed();
        std::this_thread::sleep_for(std::chrono::microseconds(coolingTime));
    }
#endif

    std::vector<double> results;
    results.insert(results.end(), algosResults.begin(), algosResults.end());

    return {sensors.first->uv, sensors.second->uv, results};
}



void ParamsFinder::run(const std::map<std::string, TLoopValues> &inputParams, ParamsFinder::runningLogCb cb)
{
    MeasureConfig config("default_params");
    initCropper();

    auto allParams = computeAcquisitionParamsCombination(inputParams);

    auto cameraParams = config.getCameraParams();
    auto uvLeds = config.getLEDParams(LEDParams::UV);
    auto visibleLeds = config.getLEDParams(LEDParams::Visible);

    ProcessorConfig processorDefaultConfig = {
            .isVisible = config.isImageInAmbientLight(),
            .channel = config.getColorChannel(),
            .theoricDistanceCLtoTLMm = config.getDistanceCL2TL(),
            .TLLengthMm = config.getTLLength(),
            .referenceNumberOfPixels = config.getPixelsToMm(),
            .maskLengthMm = config.getMaskLength()
    };

    AlgoConfig algoConfig = {
            .lengthROIMm = config.getPadding(),
            .proxNoiseROIMm = {0.5, 1, 0.5},
            .useConvolPreProcessing = false
    };

    StudiesRepository repository;
    typedef std::tuple<std::vector<int>, int32_t, int32_t, std::vector<double>,std::vector<double>> SResult;
    typedef std::tuple<uint32_t, int32_t, int32_t, std::vector<double>> SControlResult;
    std::vector<SResult> results;
    std::vector<SControlResult> controlResults;

    Poco::Timestamp lastControlSnapshotTime, startTime;
    Poco::Timestamp::TimeDiff timeBetweenControlSnapshot = 30 * 1000000;
    auto [s1, s2, controlRes] = takeControlSnapshot(lastControlSnapshotTime);
    controlResults.push_back({
        startTime.elapsed() / 1000000,
        s1,
        s2,
        controlRes
    });

    int count = 0, nbIterations = allParams.size();

    for (const auto & paramsToTry : allParams)
    {
        std::vector<int> paramsAsInt = {count};
        std::ostringstream filename;
        filename << "redChannel";
        filename << Poco::format("-%04d", count);

        for (const auto & kv : paramsToTry)
        {
            cameraParams[kv.first] = kv.second;
            filename << "_" << kv.first.substr(0, 1) << kv.second;
            paramsAsInt.push_back(std::stoi(kv.second));
        }

        filename << ".png";

        auto [ windowCassette, sensors ] = captureWindowCassette(cameraParams, uvLeds, visibleLeds);
        Poco::Timestamp timestampCapture;

        auto [ algosResults, controlLinePos, testLinePos, profile ] = computeAlgos(windowCassette, processorDefaultConfig, algoConfig, _linesPos);
        results.push_back({
            paramsAsInt,
            sensors.first->uv,
            sensors.second->uv,
            algosResults,
            profile
        });

        try
        {
            auto redChannelPicture = getRedChannelAndRotateLeft(windowCassette);
            auto redChannelWithROI = drawCTAndTLROI(redChannelPicture,
                                                    processorDefaultConfig.referenceNumberOfPixels,
                                                    _linesPos->first, _linesPos->second);
            repository.saveParamsFinder(_name, filename.str(), Converter::cvMatToPng(redChannelWithROI));
        }
        catch (const std::exception & ex)
        {
            poco_debug_f1(LOGGER, "Error when saving picture : %s", std::string(ex.what()));
        }
        std::cout << "Finding Params : " << filename.str() << " of " << nbIterations << " pictures" << std::endl;

        count++;
#ifdef USE_UEYE_CAMERA
        {
Poco::Timespan timespan(0, 500 * 1000);
auto coolingTime = timespan.totalMicroseconds() - timestampCapture.elapsed();
std::this_thread::sleep_for(std::chrono::microseconds(coolingTime));
}
#endif
        if (lastControlSnapshotTime.isElapsed(timeBetweenControlSnapshot))
        {
            lastControlSnapshotTime.update();

            auto [s1, s2, controlRes] = takeControlSnapshot(lastControlSnapshotTime);
            controlResults.push_back({
                startTime.elapsed() / 1000000,
                s1,
                s2,
                controlRes
            });
        }
    }

    std::vector<std::string> headers = {"PictureNumber"};
    for (const auto & kv : allParams.at(0))
        headers.push_back(kv.first);

    repository.saveParamsFinder(_name, std::string("results_").append(_name).append(".csv"), headers, results);
    repository.saveParamsFinder(_name, std::string("resultsControl_").append(_name).append(".csv"), controlResults);

    std::cout << "Finding Params done" << std::endl;
    if (cb) cb("Finding Params done");
}

