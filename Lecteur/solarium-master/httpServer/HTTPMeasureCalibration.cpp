//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/01/18.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "HTTPMeasureCalibration.h"

#include <map>
#include <exception>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

using namespace Poco::Net;

#include "../SolariumApp.h"
#include "../tools/Converter.h"
#include "../measures/MeasuresManager.h"

#define ARG_PATIENT_ID "patient_id"
#define ARG_STRIP_TYPE "strip_type"
#define ARG_CROPPING_PARAMS  "croppingParam"
#define ARG_ACQUISITION_PARAMS  "acquisitionParam"

void HTTPMeasureCalibration::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    std::map<std::string, std::string> croppingParams;
    std::map<std::string, std::string> acquisitionParams;
    std::string cassetteName;
    try
    {
        auto fullParams = Converter::jsonParamsStream2StringsMap(request.stream());
        cassetteName = fullParams["cassetteName"];

        if (auto acquisitionParamsIter = fullParams.find(ARG_ACQUISITION_PARAMS); acquisitionParamsIter != fullParams.end())
        {
            acquisitionParams = Converter::jsonParams2StringsMap(acquisitionParamsIter->second);
        }
        else
        {
            acquisitionParams = fullParams;
        }
        if (auto croppingParamsIter = fullParams.find(ARG_CROPPING_PARAMS); croppingParamsIter != fullParams.end())
        {
            croppingParams = Converter::jsonParams2StringsMap(croppingParamsIter->second);
        }
    }
    catch (std::exception & ex) {
        LOGGER.debug("[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    if(request.getMethod() == HTTPRequest::HTTP_POST)
    {
        try
        {
            LOGGER.debug("Calling runMeasureCalibration");
            MeasuresManager measuresManager;
            auto result = measuresManager.takePicturesForCalibration(croppingParams, acquisitionParams, cassetteName);

            LOGGER.information("Sending Calibration Result");

            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.setContentType("application/json");

            auto &ostream = response.send();
            ostream << result.toJSON();

            LOGGER.debug("Sending Done");
        }
        catch (const Poco::TimeoutException &ex)
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_REQUEST_TIMEOUT);
            response.setContentType("text/plain");
            response.send() << "Request Camera Timeout'ed : " << ex.what();
        }
        catch (std::exception &ex)
        {
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.send() << ex.what();
        }
    }
    else
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.send();
    }
}
