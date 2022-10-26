//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/30.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "ImagePreProcessing.h"
#include "ImageAnalyzer.h"

TImageProfile ImagePreProcessing::convolImageAndComputeProfile(const cv::Mat& image, uint32_t widthGaussianToConvol)
{
    cv::Mat imageLineConvol = ImagePreProcessing::convolImage(image, widthGaussianToConvol);

    TImageProfile convolImageProfile = ImageAnalyzer::computeProfileSum(imageLineConvol);
    double minLine = *std::min_element(convolImageProfile.begin(), convolImageProfile.end());

    for(uint32_t i = 0; i < convolImageProfile.size(); i++)
        convolImageProfile[i] -= minLine;

    double maxProfileLine = *std::max_element(convolImageProfile.begin(), convolImageProfile.end());
    TImageProfile normalizedProfile = ImageAnalyzer::normalizeProfile(convolImageProfile, maxProfileLine);

    return normalizedProfile;
}

cv::Mat ImagePreProcessing::convolImage(const cv::Mat& image, uint32_t widthGaussianToConvol)
{
    cv::Mat imageToConvol;

    if(image.depth() != CV_64F)
        image.convertTo(imageToConvol,CV_64F);
    else
        imageToConvol = image.clone();

    cv::Mat psf = cv::Mat(image.size(), CV_64FC1, cv::Scalar(0));

    for(uint32_t i = 0; i < psf.rows; i++)
    {
        for(uint32_t j = 0; j < psf.cols; j++)
        {
            double powResult1 = std::pow(j - std::round(psf.cols / 2), 2);
            double expParam1 = -powResult1 / 0.00001;
            double expResult1 = std::exp(expParam1);

            double powResult2 = std::pow(i - std::round(psf.rows / 2), 2);
            double expParam2 = -powResult2 / widthGaussianToConvol;
            double expResult2 = std::exp(expParam2);

            psf.at<double>(i, j) = expResult1 * expResult2;
        }
    }

    double sum = cv::sum(psf)[0];

    for(uint32_t i = 0; i < psf.rows; i++)
        for(uint32_t j = 0; j < psf.cols; j++)
            psf.at<double>(i, j) /= sum;

    cv::Mat psfFourier = applyDFTandIFFTSHIFT(psf);
    cv::Mat imageFourier = applyDFTandIFFTSHIFT(image);
    cv::Mat imageFourierConvol(imageFourier.size(), imageFourier.type());

    for(uint32_t i = 0; i < psf.rows; i++)
    {
        for(uint32_t j = 0; j < psf.cols; j++)
        {
            cv::Vec2d psfPixel = psfFourier.at<cv::Vec2d>(i, j);
            cv::Vec2d imagePixel = imageFourier.at<cv::Vec2d>(i, j);
            double realPart = psfPixel[0] * imagePixel[0] - (psfPixel[1] * imagePixel[1]);
            double imaginaryPart = psfPixel[0] * imagePixel[1] + psfPixel[1] * imagePixel[0];
            imageFourierConvol.at<cv::Vec2d>(i, j) = cv::Vec2d(realPart, imaginaryPart);
        }
    }

    cv::Mat idft;

    cv::Mat fftshift1 = ifftshift(imageFourierConvol, true);
    cv::dft(fftshift1, idft, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);
    cv::Mat result = ifftshift(idft, true);

    return result;
}

cv::Mat ImagePreProcessing::ifftshift(const cv::Mat& image, bool doFFTSHIFT)
{
    // rearrange the quadrants of image so that the origin is at the image center
    int cx = image.cols/2; //width
    int cy = image.rows/2; //height

    if(doFFTSHIFT)
    {
        cx = image.cols - cx;
        cy = image.rows - cy;
    }

    cv::Mat q0(image, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    cv::Mat q1(image, cv::Rect(cx, 0, image.cols - cx, cy));  // Top-Right
    cv::Mat q2(image, cv::Rect(0, cy, cx, image.rows - cy));  // Bottom-Left
    cv::Mat q3(image, cv::Rect(cx, cy, image.cols - cx, image.rows - cy)); // Bottom-Right

    cv::Mat result(image.size(), image.type());

    //swap TL with BR and TR with BL
    q3.copyTo(result(cv::Rect(0, 0, image.cols - cx, image.rows - cy))); // new top-left
    q2.copyTo(result(cv::Rect(image.cols - cx, 0, cx, image.rows - cy))); // new top-right
    q1.copyTo(result(cv::Rect(0, image.rows - cy, image.cols - cx, cy))); // new bottom-left
    q0.copyTo(result(cv::Rect(image.cols - cx, image.rows - cy, cx, cy))); // new bottom-right

    return result;
}

cv::Mat ImagePreProcessing::applyDFTandIFFTSHIFT(const cv::Mat& grayImage)
{
    cv::Mat padded;                            //expand input image to optimal size
    int m = grayImage.rows ;
    int n = grayImage.cols ; // on the border add zero values
    copyMakeBorder(grayImage, padded, 0, m - grayImage.rows, 0, n - grayImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<double>(padded), cv::Mat::zeros(padded.size(), CV_64FC1)};
    cv::Mat complexI;
    merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

    dft(complexI, complexI, cv::DFT_COMPLEX_OUTPUT);            // this way the result may fit in the source matrix

    return ifftshift(complexI, false);
}