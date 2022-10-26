//
// Created by KarimSadki on 17/12/2020.
//

#ifndef PICA_CROPPERSTRUCTSANDDATA_H
#define PICA_CROPPERSTRUCTSANDDATA_H

#include "CassetteType.h"

const int BG = 0;
const int R = 1;
const int R_Visible = 2;

const int NUMBER_OF_STRIP_PIXEL_IN_PROTOV1[3] = {120000, 100000, 130000};
const int NUMBER_OF_STRIP_PIXEL_IN_PROTOV2_PRECROPPED[3] = {80000, 95000, 80000};

enum DeviceType { Proto_v1, Proto_v2_PreCropped, Proto_v3 };
enum ImageDepth { Bits8, Bits12 };
enum LightningCondition {UV, Visible};
enum CroppingValidationMessage {TooThin, TooLarge, IsValid};
enum DeviceFocal {ProtoV3_F041, ProtoV3_F060, ProtoV3_F060_L, ProtoV3_F080, ProtoV1_Focal, ProtoV2_DefaultFocal, ProtoV2_F060, UnknownFocal};

struct CropROIMm {
    double upLeftCornerMarginX;
    double upLeftCornerMarginY;
    double width;
    double height;
};

struct CropParams {
    DeviceType deviceType;
    ImageDepth imageDepth;
    LightningCondition lightningCondition;
    CassetteModel cassetteModel;
    double referenceNumberOfPixels;
    StripMatrix stripMatrix;
    DeviceFocal deviceFocal;
};

struct ROIsToCrop {
    CropROIMm ROIToThreshold;
    CropROIMm outputROI;
};

#endif //PICA_CROPPERSTRUCTSANDDATA_H
