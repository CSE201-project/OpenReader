//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/08/26.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_IMGTOOLS_H
#define SOLARIUM_IMGTOOLS_H

#include <vector>
#include <cstdint>
#include <opencv2/core/mat.hpp>

class ImgTools {
public:

    static std::vector<uint8_t> extractROIForReport(std::vector<uint8_t> image, uint32_t controlLinePos, uint32_t testLinePos, double pixelsToMm, double lengthROIMm);
    static bool isCapture8BitsOK(const cv::Mat & capture);
};


#endif //SOLARIUM_IMGTOOLS_H

