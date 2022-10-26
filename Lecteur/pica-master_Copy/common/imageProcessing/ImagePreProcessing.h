//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/30.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef IMAGEPREPROCESSING_H
#define IMAGEPREPROCESSING_H

#include <opencv2/opencv.hpp>
#include "ImageAnalyzer.h"

class ImagePreProcessing {
public:
    constexpr static double DEFAULT_WIDTH_GAUSSIAN_CONVOL_TL_MM = 3.;

    static cv::Mat convolImage(const cv::Mat& image, uint32_t widthGaussianToConvol);
    static TImageProfile convolImageAndComputeProfile(const cv::Mat& image, uint32_t widthGaussianToConvol);

private:
    /* Code from opencv, discrete_fourier_transform example */
    static cv::Mat applyDFTandIFFTSHIFT(const cv::Mat& grayImage);
    static cv::Mat ifftshift(const cv::Mat& image, bool doFFTSHIFT);
};

#endif //IMAGEPREPROCESSING_H
