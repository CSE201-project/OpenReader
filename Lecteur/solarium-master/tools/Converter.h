//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/07.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_CONVERTER_H
#define SOLARIUM_CONVERTER_H

#include <vector>
#include <map>
#include <string>
#include <istream>
#include <cstdint>
#include <opencv2/core/mat.hpp>



namespace Converter {
    std::string toJson(std::vector<double> data);
    std::string toJson(const std::map<std::string, std::string> & params);
    std::map<std::string, std::string> jsonParamsStream2StringsMap(std::istream & jsonParams);
    std::map<std::string, std::string> jsonParams2StringsMap(const std::string & jsonParams);
    std::vector<std::map<std::string, std::string>> jsonArrayParams2StringsMapVector(const std::string & jsonArrayParams);

    std::vector<int> jsonArray2IntVector(const std::string & jsonArrayParams);

    std::map<std::string, std::vector<int>> jsonObjectToParamsValues(const std::string & jsonParams);

    std::vector<uint8_t> cvMatToPng(cv::Mat img, int colorConvert = 0);
};


#endif //SOLARIUM_CONVERTER_H
