//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/07.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "Converter.h"
#include <sstream>
#include <limits>

#include <opencv2/opencv.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::JSON;

typedef std::numeric_limits<double> doubleLimit;

std::string Converter::toJson(std::vector<double> data)
{
    std::ostringstream oss;
    oss.precision(doubleLimit::digits10);
    oss << "[";
    bool firstElem = true;

    for(auto item : data)
    {
        if (firstElem)
        {
            oss << item;
            firstElem = false;
        }
        else
            oss << "," << item;
    }
    oss << "]";

    return oss.str();
}

std::vector<std::map<std::string, std::string>> Converter::jsonArrayParams2StringsMapVector(const std::string & jsonArrayParams)
{
    std::vector<std::map<std::string, std::string>> results;
    try
    {
        Parser jsonParser;
        auto result = jsonParser.parse(jsonArrayParams);
        const auto &jsonArray = result.extract<Array::Ptr>();
        for (auto i = 0; i < jsonArray->size(); i++)
        {
            const auto &jsonObject = jsonArray->getObject(i);
            auto aStruct = Object::makeStruct(jsonObject);

            std::map<std::string, std::string> params;
            for (const auto &prop : aStruct)
            {
                params[prop.first] = prop.second.toString();
            }
            results.push_back(params);
        }
    }
    catch (const std::exception & /*ex*/)
    {

    }
    return results;
}

std::map<std::string, std::string> Converter::jsonParamsStream2StringsMap(std::istream & jsonParams)
{
    std::map<std::string, std::string> params;
    try
    {
        Parser jsonParser;
        auto result = jsonParser.parse(jsonParams);
        const auto &jsonObject = result.extract<Object::Ptr>();
        auto aStruct = Object::makeStruct(jsonObject);

        for (const auto &prop : aStruct)
        {
            params[prop.first] = prop.second.toString();
        }
    }
    catch (const std::exception & /*ex*/)
    {

    }
    return params;
}

std::map<std::string, std::string> Converter::jsonParams2StringsMap(const std::string &jsonParams)
{
    std::istringstream is(jsonParams);
    return jsonParamsStream2StringsMap(is);
}

std::vector<uint8_t> Converter::cvMatToPng(cv::Mat img, int colorConvert)
{
    if (colorConvert != 0)
    {
        cv::cvtColor(img, img, colorConvert);
    }

    std::vector<uint8_t> resultingPicture;
    cv::imencode(".png", img, resultingPicture);
    return resultingPicture;
}

std::string Converter::toJson(const std::map<std::string, std::string> &params)
{
    std::ostringstream oss;

    oss << "{";
    bool addComma = false;
    for (const auto & kv : params)
    {
        if (addComma) oss << ",";
        else addComma = true;

        oss << "\"" << kv.first << "\":\"" << kv.second << "\"";
    }
    oss << "}";
    return oss.str();
}

std::vector<int> Converter::jsonArray2IntVector(const std::string &jsonArrayParams)
{
    std::vector<int> results;
    try
    {
        Parser jsonParser;
        auto result = jsonParser.parse(jsonArrayParams);
        const auto &jsonArray = result.extract<Array::Ptr>();
        for (auto i = 0; i < jsonArray->size(); i++)
        {
            int val = jsonArray->getElement<int>(i);
            results.push_back(val);
        }
    }
    catch (const std::exception & /*ex*/)
    {

    }
    return results;
}

std::map<std::string, std::vector<int>> Converter::jsonObjectToParamsValues(const std::string &jsonParams)
{
    std::map<std::string, std::vector<int>> results;
    try
    {
        Parser jsonParser;
        auto result = jsonParser.parse(jsonParams);
        const auto &jsonObject = result.extract<Object::Ptr>();
        auto aStruct = Object::makeStruct(jsonObject);

        for (const auto &prop : aStruct)
        {
            auto arrayString = !prop.second.isEmpty() ? prop.second.toString() : "";
            results[prop.first] = jsonArray2IntVector(arrayString);
        }
    }
    catch (const std::exception & /*ex*/)
    {

    }
    return results;
}

