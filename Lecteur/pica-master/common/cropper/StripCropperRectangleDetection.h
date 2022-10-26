//
// Created by KarimSadki on 17/12/2020.
//

#ifndef PICA_STRIPCROPPERRECTANGLEDETECTION_H
#define PICA_STRIPCROPPERRECTANGLEDETECTION_H

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "CropperStructsAndData.h"

class StripCropperRectangleDetection {
public:
    CropParams cropParams;
    const double minimumAcceptableWidth = 1.5;
    const double maximumAcceptableWidth = 5.;

    StripCropperRectangleDetection(CropParams params);

    cv::Mat crop(const cv::Mat& frameToDetectRectangle, const cv::Mat& frameToCrop, std::vector<cv::Mat>* intermediateSteps = nullptr) const;
    static std::tuple<bool, CroppingValidationMessage> isValidCroppedImage(const cv::Mat& cropped,
                                                             const CropParams& cropParams);
    static DeviceFocal determineProtoV3Focal(const std::string& focal);
    static DeviceFocal determineProtoV2Focal(const std::string& focal);

private:
    CropROIMm _ROIToThreshold;
    CropROIMm _outputROI;

    std::map<DeviceFocal, ROIsToCrop> _ROIsToCropMap =
            {{ProtoV3_F041,
                     {.ROIToThreshold = {1.5, 4.5, 9.4, 13.9},
                         .outputROI = {1.5, 3., 9.4, 17.2}
                        }
             },
             {ProtoV3_F060,
                     {.ROIToThreshold = {2.5, 3., 8., 13.9},
                         .outputROI = {2.5, 2., 8., 15.9}
                        }
             },
             {ProtoV3_F060_L,
                     {.ROIToThreshold = {2.5, 1., 8., 13.9},
                             .outputROI = {2.5, 0., 8., 15.9}
                     }
             },
             {ProtoV3_F080,
                     {.ROIToThreshold = {0, 0, -1, -1},
                         .outputROI = {0, 0, -1, -1}
                        }
             },
             {ProtoV1_Focal,
                     {.ROIToThreshold = {392, 100, 270, 1020},
                         .outputROI = {392, 100, 270, 1020}
                        }
             },
             {ProtoV2_DefaultFocal,
                     {.ROIToThreshold = {1.1, 3.9, 9.4, 13.9},
                         .outputROI = {1.1, 2.5, 9.4, 17.2}
                        }
             },
             {ProtoV2_F060,
                     {.ROIToThreshold = {4., 3.9, 9.7, 13.9},
                             .outputROI = {4, 3.9, 9.4, 17.2}
                     }
             }
            };

    const double _maxPossibleTilt = 45;

    ROIsToCrop checkAndInitCropROIMm(const cv::Mat& frame) const;
    cv::Mat cropToThreshold(const cv::Mat& frame, const CropROIMm ROIToThreshold, std::vector<cv::Mat>* intermediateSteps) const;
    cv::RotatedRect detectRotatedRectangle(const cv::Mat& channelThreshold, std::vector<cv::Mat>* intermediateSteps) const;
    cv::Mat rotateAndCropImage(const cv::Mat& frameCenter,
                               cv::RotatedRect& rotatedRect,
                               const cv::Mat& channelThreshold,
                               double outputHeight,
                               std::vector<cv::Mat>* intermediateSteps) const;
    std::vector<double> determineThresholdValues(const std::vector<uint32_t>& histogram, const ColorChannelLAB& channel) const;
    std::vector<double> determineThresholdValuesCassette(const std::vector<uint32_t>& histogram,
                                                         double heigthToDetermineNumberOfPixels,
                                                         const ColorChannelLAB& channel) const;

    cv::Mat thresholdOneChannel(const cv::Mat& frameCenter,
                                double heightROIThreshold,
                                ColorChannelLAB channel,
                                std::vector<cv::Mat>* intermediateSteps) const;
    cv::Rect reduceROI(const cv::Mat &rotatedChannelThreshold, double outputHeight, int leftBoundary, int rightBoundary) const;

    std::tuple<cv::Point, cv::Point> determineTopAndBottomLeft(const cv::RotatedRect& rotatedRect) const;
};


#endif //PICA_STRIPCROPPERRECTANGLEDETECTION_H
