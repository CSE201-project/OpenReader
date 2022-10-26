//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/05.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_MEASURESMANAGER_H
#define SOLARIUM_MEASURESMANAGER_H

#include <string>
#include <vector>

#include <Poco/UUID.h>

#include "AnalysisResult.h"
#include "CalibrationResult.h"
#include "MeasureConfig.h"

enum MeasureStatusCode {
    ERR_CALLING_CROPPER,
    ERR_CONVERTING_CROP,
    ERR_SAVING_CROP_TO_TMP,
    ERR_COMPUTING_PROFILE,
    ERR_FINDING_CONTROL_LINE,
    ERR_ZOOM_ON_PROFILE,
    ERR_CALLING_PICA_PROCESSOR,
    SUCCESS
};

std::string ToString(MeasureStatusCode code);

class MeasuresManager {
private:
    AnalysisResult doMeasure(const MeasureConfig &config);
    CalibrationResult doMeasureCalibration(const MeasureConfig &config);

public:
    MeasuresManager();

    AnalysisResult runMeasure(const std::string &patientId,
                              const std::string &stripType,
                              const std::string &stripBatchId,
                              const std::string &dateString);

    CalibrationResult takePicturesForCalibration(const ICamera::TCameraParams &croppingParams,  const ICamera::TCameraParams &params, const std::string &cassetteName);

    struct StudyPicturesResult {
        uint8_t nbPictures;
        std::vector<uint8_t> lastPicture;
        std::vector<double> concentration;
    };

    StudyPicturesResult takePicturesForStudy(const std::string &studyName,
                                             const std::string &studyDate,
                                             const std::string &fileName,
                                             const std::vector<ICamera::TCameraParams> &paramsList,
                                             const std::string &cassetteName,
                                             bool computeConcentration = false);

    void runSequence(const ICamera::TCameraParams &config, uint32_t intervalInSeconds, uint32_t nbCaptures);

};


#endif //SOLARIUM_MEASURESMANAGER_H
