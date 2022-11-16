//
// Created by KarimSadki on 09/03/2021.
//

#ifndef PICA_CASSETTETYPE_H
#define PICA_CASSETTETYPE_H

#include <map>
#include <string>

#include "common/imageProcessing/ImageAnalyzer.h"

enum CassetteModel {K701WT, K701BT, WithoutCassette, UnknownCassetteModel};
enum StripMatrix {Serum, Blood, Tampon, Nasal, UnknownMatrix};

struct CassetteData {
    CassetteModel model;
    StripMatrix stripMatrix;
};

class CassetteType {
public:
    struct CassetteDataToCrop {
        std::string Name;
        ColorChannelLAB channelToCropUVFromVisible;
        ColorChannelLAB channelToCropVisible;
        double widthWindowMm;
        bool revertImageToThreshold;
    };

    /* name need to have the following pattern: project_cassetteModel_topColor_matrix*/
    static CassetteData decodeCassetteName(const std::string& name);

    static std::map<CassetteModel, CassetteDataToCrop> CassetteDataToCropMap;
    static std::map<StripMatrix, bool> MatrixDataToCropMap;
};

#endif //PICA_CASSETTETYPE_H
