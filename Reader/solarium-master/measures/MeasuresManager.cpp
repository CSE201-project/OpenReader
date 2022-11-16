//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/05.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "MeasuresManager.h"

#include <common/cropper/StripCropperRectangleDetection.h>
#include <common/imageProcessing/ImageAnalyzer.h>

#include "configure.h"
#include "AnalysisResult.h"
#include "MeasureConfig.h"
#include "StripAnalysis.h"

#include "SolariumApp.h"
#include "repository/AnalysisRepository.h"
#include "repository/StudiesRepository.h"
#include "tools/Converter.h"
#include "tools/ImgTools.h"
#include "device/DeviceInfoFormatter.h"
#include "reports/Reports.h"

#include <thread>
#include <chrono>
#include <cmath>
#include <Poco/Logger.h>
#include <monoplex/algos/AlgoProxNoise.h>
#include <monoplex/algos/AlgoPoly2.h>
#include <monoplex/algos/AlgoExponantialFit.h>
#include <monoplex/algos/AlgoTestMinusProxNoise.h>

#define LEDS_TIMEOUT 200
#define CAMERA_TIMEOUT 2000


cv::Mat rotateVerticalPicToCLOnLeft(const cv::Mat &inputImage)
{
    cv::Mat result;
    cv::rotate(inputImage, result, cv::ROTATE_90_COUNTERCLOCKWISE);
    return result;
}


cv::Mat rotateToVerticalPicture(const cv::Mat &inputImage)
{
    cv::Mat result;
#ifdef USE_V4L2_CAMERA
    cv::rotate(inputImage, result, cv::ROTATE_90_CLOCKWISE);
#else
    cv::rotate(inputImage, result, cv::ROTATE_90_COUNTERCLOCKWISE);
#endif
    return result;
}

cv::Mat takeRawPicture(const MeasureConfig &config)
{
    auto cameraParams = config.getCameraParams();
    auto uvLeds = config.getLEDParams(LEDParams::UV);
    auto visibleLeds = config.getLEDParams(LEDParams::Visible);

    auto futurePicture = SolariumDevice::instance().takePictureForMeasure({
                                                                                  cameraParams,
                                                                                  uvLeds,
                                                                                  visibleLeds
                                                                          });
    futurePicture.wait();
    auto pic = futurePicture.data();
    if (pic.empty())
        throw std::runtime_error("Error taking picture");
    return pic;
}

cv::Mat takeVerticalPicture(const MeasureConfig &config)
{
    return rotateToVerticalPicture(takeRawPicture(config));
}

cv::Mat takePictureWithCLOnTheLeft(const MeasureConfig &config)
{
    auto picture = takeRawPicture(config);
#ifdef USE_UEYE_CAMERA
    cv::rotate(picture, picture, cv::ROTATE_180);
#endif
    return picture;
}

MeasuresManager::MeasuresManager()
{

}

AnalysisResult MeasuresManager::runMeasure(const std::string &patientId,
                                           const std::string &stripType,
                                           const std::string &stripBatchId,
                                           const std::string &dateString)
{
    MeasureConfig config(stripType);
    AnalysisResult analysisResult = doMeasure(config);
    analysisResult.setDate(dateString);
    analysisResult.PatientId = patientId;
    analysisResult.StripBatchId = stripBatchId;
    LOGGER.information("MeasuresManager::runMeasure done, creating report");

    auto stripImage = analysisResult.StripImage;

    analysisResult.StripImage = ImgTools::extractROIForReport(stripImage, analysisResult.ControlLinePos,
                                                              analysisResult.TestLinePos, config.getPixelsToMm(),
                                                              config.getPadding());
    try {
        analysisResult.PdfReport = Reports::createReport(&analysisResult);
    }
    catch (const std::exception & ex) {
        LOGGER.error("Measure Error Creating PDF Report : %s", ex.what());
    }
    analysisResult.StripImage = stripImage;

    AnalysisRepository repository;
    repository.save(analysisResult);

    if (analysisResult.AllResults.size() > 0)
        analysisResult.StripProfile = analysisResult.AllResults.at(0).Profile;

    for (auto & algoRes : analysisResult.AllResults)
    {
        algoRes.Profile = {};
    }

    return analysisResult;
}

AnalysisResult MeasuresManager::doMeasure(const MeasureConfig &config)
{
    MeasureConfig croppingConf("default_crop_params");
    auto croppingPicture = takeVerticalPicture(croppingConf);

    auto pictureMat = takeVerticalPicture(config);

    AnalysisResult result;
    result.StripType = config.getStripType();
    result.RawImage = Converter::cvMatToPng(pictureMat);

    MeasureStatusCode measureStatus = ERR_CALLING_CROPPER;
    try
    {
        //Following string will be sent by Workbench.
        CassetteData cassetteData = CassetteType::decodeCassetteName(config.getCassetteName());

        CropParams cropperParams = {
#ifdef USE_UEYE_CAMERA
                .deviceType = Proto_v2_PreCropped,
#elif defined(USE_V4L2_CAMERA)
                .deviceType = Proto_v3,
#else
                .deviceType = Proto_v3,
#endif
                .imageDepth = config.getColorDepth() == 12 ? Bits12 : Bits8,
                .lightningCondition = config.isImageInAmbientLight() ? Visible : UV,
                .cassetteModel = cassetteData.model,
                .referenceNumberOfPixels = config.getPixelsToMm(),
                .stripMatrix = cassetteData.stripMatrix,
                .deviceFocal = (DeviceFocal)config.getLensFocal().type()
        };

        StripCropperRectangleDetection cropper(cropperParams);
        auto stripImageMat = cropper.crop(croppingPicture, pictureMat);

        measureStatus = ERR_CONVERTING_CROP;
        auto stripImagePng = Converter::cvMatToPng(stripImageMat);
        result.StripImage = stripImagePng;

        //measureStatus = ERR_SAVING_CROP_TO_TMP;
        //repository.save("/tmp/Solarium/measure_cropped.png",stripImagePng);

        ProcessorConfig processorConfig(
                stripImageMat,
                config.isImageInAmbientLight(),
                config.getColorChannel(),
                config.getPadding(),
                config.getDistanceCL2TL(),
                config.getTLLength(),
                config.getPixelsToMm(),
                config.getTLLength(),
                config.getMaskLength()
                );

        AlgoProxNoise imageProfiler;
        auto profileRaw = imageProfiler.computeProfileForAnalyze(processorConfig, false);
        auto profileConvoluted = imageProfiler.computeProfileForAnalyze(processorConfig, true);

        auto proxNoiseROISizes = config.getProxNoiseROISizes();
        ProxNoiseROISizeMm defaultProxNoiseROI ={
                .TLLength = std::get<1>(proxNoiseROISizes),
                .noiseBeforeTL = std::get<0>(proxNoiseROISizes),
                .noiseAfterTL = std::get<2>(proxNoiseROISizes)
        };

        std::vector<std::tuple<std::string, std::shared_ptr<IImageProcessor>, const ProfileData&>> algos = {
                {"prox_noise_convol", std::make_shared<AlgoProxNoise>(), profileConvoluted},
                {"prox_noise", std::make_shared<AlgoProxNoise>(), profileRaw},
                {"expo_fit_convol",std::make_shared<AlgoExponantialFit>(), profileConvoluted},
                {"expo_fit",std::make_shared<AlgoExponantialFit>(), profileRaw},
                {"poly2_convol",std::make_shared<AlgoPoly2>(), profileConvoluted},
                {"poly2",std::make_shared<AlgoPoly2>(), profileRaw}
        };

        measureStatus = ERR_CALLING_PICA_PROCESSOR;
        std::vector<double> resultsAlgos;
        for(auto & algo : algos)
        {
            auto algoName = std::get<0>(algo);
            auto algoProcessor = std::get<1>(algo);
            auto processorResult = algoProcessor->compute(processorConfig, std::get<2>(algo), defaultProxNoiseROI);

            AnalysisResult::TConcentration quantity = {0};
            auto cutOffStr = config.getAlgoInfos( algoName + ".cutoffs");
            if (!cutOffStr.empty()) {
                quantity = {
                    .p1 = algoProcessor->computeFirstPrediction(processorResult, config.getAlgoInfos(algoName + ".first_prediction")),
                    .p2 = 0, //algoProcessor->computeSecondPrediction(processorResult.result, config.getAlgoInfos( algoName + ".second_prediction")),
                    .level = algoProcessor->determinePredictionMessage(processorResult, config.getAlgoInfos(algoName + ".cutoffs"))
                };
            }

            result.ControlLinePos = profileRaw.controlLine;
            result.TestLinePos = profileRaw.testLine;

            AnalysisResult::TAlgoResult algoResult = {
                    .Name = algoName,
                    .Value = processorResult,
                    .CutOff = cutOffStr.empty() ? 0 : std::stod(cutOffStr),
                    //.Profile = processorResult.RawProfile,
                    .Quantity = quantity
            };
            result.AllResults.push_back(algoResult);
        }

        measureStatus = SUCCESS;
    }
    catch (const std::exception & ex)
    {
        LOGGER.debug("[Run Measure] exception %s", std::string(ex.what()));
    }

    result.Status = ToString(measureStatus);

    return result;
}

MeasuresManager::StudyPicturesResult MeasuresManager::takePicturesForStudy(const std::string &studyName,
                                                                           const std::string &studyDate,
                                                                           const std::string &fileName,
                                                                           const std::vector<ICamera::TCameraParams> &paramsList,
                                                                           const std::string &cassetteName,
                                                                           bool computeConcentration)
{
    StudiesRepository repository;
    auto & device = SolariumDevice::instance();
    auto hostname = device.getHostname();
    auto version = SolariumApp::version();
    cv::Mat croppingPicture;

    StudyPicturesResult result;

    for (int i = 0; i < paramsList.size(); i++)
    {
        MeasureConfig config(paramsList.at(i));

        auto futureSensors = device.getSensorsInfos(Poco::Void());

        auto pictureMat = takeRawPicture(config);

        futureSensors.wait();
        auto sensors = futureSensors.data();

        StudySnapshotResult snapshotResult;
        snapshotResult.Snapshot = Converter::cvMatToPng(pictureMat);
        snapshotResult.FileName = fileName;
        snapshotResult.StudyName = studyName;
        snapshotResult.Params = config.ToJSON();
        snapshotResult.CassetteName = cassetteName;
        snapshotResult.SensorsInfo = DeviceInfoFormatter::toJSON(hostname, sensors, version);
        snapshotResult.IsCroppingSnapshot = config.isCroppingParam();
        repository.save(studyName, studyDate, i, snapshotResult);

        if (config.isCroppingParam())
        {
            croppingPicture = rotateToVerticalPicture(pictureMat);
        }
        else
        {
            if (computeConcentration) {
                // Pica want vertical pictures
                pictureMat = rotateToVerticalPicture(pictureMat);

                bool isImageInAmbientLight = config.isImageInAmbientLight();

                CassetteData cassetteData = CassetteType::decodeCassetteName(cassetteName);

                CropParams cropperParams = {
#ifdef USE_UEYE_CAMERA
                        .deviceType = Proto_v2_PreCropped,
#elif defined(USE_V4L2_CAMERA)
                        .deviceType = Proto_v3,
#else
                        .deviceType = Proto_v1,
#endif
                        .imageDepth = config.getColorDepth() == 12 ? Bits12 : Bits8,
                        .lightningCondition = isImageInAmbientLight ? Visible : UV,
                        .cassetteModel = cassetteData.model,
                        .referenceNumberOfPixels = config.getPixelsToMm(),
                        .stripMatrix = cassetteData.stripMatrix,
                        .deviceFocal = (DeviceFocal)config.getLensFocal().type()
                };

                try
                {


                    StripCropperRectangleDetection cropper(cropperParams);
                    auto stripImageMat = cropper.crop(!croppingPicture.empty() ? croppingPicture : pictureMat,
                                                      pictureMat);

                    ProcessorConfig processorConfig(
                            stripImageMat,
                            isImageInAmbientLight,
                            config.getColorChannel(),
                            config.getPadding(),
                            config.getDistanceCL2TL(),
                            config.getTLLength(),
                            config.getPixelsToMm(),
                            config.getTLLength(),
                            config.getMaskLength()
                            );


                    AlgoProxNoise algo;
                    auto profileData = algo.computeProfileForAnalyze(processorConfig, false);
                    auto algoResult = algo.compute(processorConfig, profileData, {1.3, 1.0, 1.0});
                    auto algoPrediction = algo.computeFirstPrediction(algoResult, config.getAlgoInfos(
                            "prox_noise.first_prediction"));
                    result.concentration.push_back(algoPrediction);
                }
                catch (const std::exception &ex)
                {
                    result.concentration.push_back(std::nan(""));
                }
            } else {
                result.concentration.push_back(std::nan(""));
            }
        }

        result.lastPicture = snapshotResult.Snapshot;
        result.nbPictures = i + 1;
    }

    repository.syncWrites();

    return result;
}


CalibrationResult getCalibrationPictureAndProfiles(const MeasureConfig &acquisitionConfig, const StripCropperRectangleDetection &cropper, const cv::Mat & croppingPicture)
{
    auto picture = takeVerticalPicture(acquisitionConfig);

    CalibrationResult result;
    result.Result = 0;
    result.StripType = "calibration";
    result.RawImage = Converter::cvMatToPng(picture);

    cv::Mat cassetteWindowPicture;
    try
    {
        cassetteWindowPicture = cropper.crop(!croppingPicture.empty() ? croppingPicture : picture, picture);
    }
    catch (const std::exception &ex)
    {
        cassetteWindowPicture = picture;
        result.Status = ToString(ERR_CALLING_CROPPER);
    }

    result.StripImage = Converter::cvMatToPng(rotateVerticalPicToCLOnLeft(cassetteWindowPicture));

    try
    {
        auto colorDepth = acquisitionConfig.getColorDepth();
        auto normalisationFactor = std::pow(2, colorDepth) - 1;
        result.StripProfileBlue = ImageAnalyzer::normalizeProfile(ImageAnalyzer::computeImageProfile(cassetteWindowPicture, acquisitionConfig.isImageInAmbientLight(), Blue, colorDepth), normalisationFactor);
        result.StripProfileGreen = ImageAnalyzer::normalizeProfile(ImageAnalyzer::computeImageProfile(cassetteWindowPicture, acquisitionConfig.isImageInAmbientLight(), Green, colorDepth), normalisationFactor);
        result.StripProfileRed = ImageAnalyzer::normalizeProfile(ImageAnalyzer::computeImageProfile(cassetteWindowPicture, acquisitionConfig.isImageInAmbientLight(), Red, colorDepth), normalisationFactor);
        result.Status = ToString(SUCCESS);
    }
    catch (const std::exception & ex)
    {
        result.Status = ToString(ERR_COMPUTING_PROFILE);
    }

    AnalysisRepository repository;
    repository.save(result);
    if (!croppingPicture.empty())
    {
#ifdef USE_DUMMY_PERIPHERALS
        repository.save("./data/calibration/croppingPicture.png", Converter::cvMatToPng(croppingPicture));
#else
        repository.save("/data/analysis/calibration/croppingPicture.png", Converter::cvMatToPng(croppingPicture));
#endif
    }


    return result;
}

CalibrationResult MeasuresManager::takePicturesForCalibration(const ICamera::TCameraParams &croppingParams,
                                                              const ICamera::TCameraParams &params,
                                                              const std::string &cassetteName)
{
    MeasureConfig acquisitionConfig(params);

    cv::Mat croppingPicture;

    CassetteData cassetteData = CassetteType::decodeCassetteName(cassetteName);

    CropParams cropperParams = {
#ifdef USE_UEYE_CAMERA
            .deviceType = Proto_v2_PreCropped,
#elif defined(USE_V4L2_CAMERA)
            .deviceType = Proto_v3,
#else
            .deviceType = Proto_v3,
#endif
            .imageDepth = Bits8,
            .lightningCondition = acquisitionConfig.isImageInAmbientLight() ? Visible : UV,
            .cassetteModel = cassetteData.model,
            .referenceNumberOfPixels = acquisitionConfig.getPixelsToMm(),
            .stripMatrix = cassetteData.stripMatrix,
            .deviceFocal = (DeviceFocal)acquisitionConfig.getLensFocal().type()
    };

    if (!croppingParams.empty())
    {
        MeasureConfig croppingConfig(croppingParams);
        cropperParams.lightningCondition = croppingConfig.isImageInAmbientLight() ? Visible : UV;
        croppingPicture = takeVerticalPicture(croppingConfig);
    }

    StripCropperRectangleDetection cropper(cropperParams);

    return getCalibrationPictureAndProfiles(acquisitionConfig, cropper, croppingPicture);
}

std::string getSequenceName(const ICamera::TCameraParams &configInput)
{
    if (auto it = configInput.find("sequence_name"); it != configInput.end())
        return it->second;
    else
        return Poco::DateTimeFormatter::format(Poco::Timestamp(), "sequence-%y%m%d-%H%M");
}

void MeasuresManager::runSequence(const ICamera::TCameraParams &configInput, uint32_t intervalInSeconds, uint32_t nbCaptures)
{
    MeasureConfig config(configInput);
    std::string sequenceName = getSequenceName(configInput);
    StudiesRepository repository;

    for (uint32_t i = 0; i < nbCaptures; i++)
    {
        auto picture = takePictureWithCLOnTheLeft(config);

        repository.saveSequenceSnap(sequenceName, i, Converter::cvMatToPng(picture));

        std::this_thread::sleep_for(std::chrono::milliseconds(intervalInSeconds * 1000));
    }

    repository.syncWrites();
}

// #####################################################################

#define TO_STR(x) #x
#define STR(x) TO_STR(x)
#define CASE(x) case x : return STR(x); break

std::string ToString(MeasureStatusCode code)
{
    switch (code)
    {
        CASE(ERR_CALLING_CROPPER);
        CASE(ERR_CONVERTING_CROP);
        CASE(ERR_SAVING_CROP_TO_TMP);
        CASE(ERR_COMPUTING_PROFILE);
        CASE(ERR_FINDING_CONTROL_LINE);
        CASE(ERR_ZOOM_ON_PROFILE);
        CASE(ERR_CALLING_PICA_PROCESSOR);
        CASE(SUCCESS);
    }
    return "";
}
