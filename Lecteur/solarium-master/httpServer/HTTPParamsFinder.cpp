//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/06/09.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "HTTPParamsFinder.h"

#include <map>
#include <exception>
#include <sstream>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

#include <tools/Converter.h>
#include <measures/ParamsFinder.h>
#include <SolariumApp.h>

/*
 * {
 *   "name" : "20210621-112233",
 *   "params" : {
 *       "awb" : "off",
 *       "exposure" : 60,
 *       ...
 *       "LEDs_params" : {
 *           "visible" : {
 *           },
 *           "uv" : {
 *           }
 *       }
 *    },
 *    "crop_param" : {
 *      ...
 *    }
 *    "cassette_name" : "AZE"
 * }
 *
 *
 */

#define ARG_CASSETTE_NAME "cassette_name"
#define ARG_CROP_PARAM "crop_param"
#define ARG_NAME "name"
#define ARG_ORDER "order"
#define ARG_PARAMS "params"

void HTTPParamsFinder::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    std::map<std::string, std::vector<int>> finderParams;
    std::string nameParam, cassetteName;
    bool decreasingValuesDirection = false;

    try
    {
        auto fullParams = Converter::jsonParamsStream2StringsMap(request.stream());
        nameParam  = fullParams[ARG_NAME];
        finderParams = Converter::jsonObjectToParamsValues(fullParams[ARG_PARAMS]);
        cassetteName = fullParams[ARG_CASSETTE_NAME];
        if (auto it = fullParams.find(ARG_ORDER); it != fullParams.end())
        {
            decreasingValuesDirection = it->second == "desc";
        }
    }
    catch (const std::exception & ex)
    {
        poco_debug_f1(LOGGER, "[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        response.setContentType("text/plain");
        std::ostringstream oss;

        ParamsFinder::runningLogCb cb = [&oss](const std::string & msg)
        {
            oss << msg;
        };

        ParamsFinder finder(nameParam, cassetteName);
        finder.run(finderParams, cb);

        response.send() << oss.str();

    }
    catch(std::exception &ex)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}
