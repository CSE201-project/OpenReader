//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/05.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouteTakeMeasure.h"

#include <map>
#include <exception>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

using namespace Poco::Net;

#include "../SolariumApp.h"
#include "../tools/Converter.h"
#include "../measures/MeasuresManager.h"

#define ARG_PATIENT_ID "patient_id"
#define ARG_STRIP_TYPE "strip_type"
#define ARG_STRIP_BATCH_ID "strip_batch_id"
#define ARG_DATE "date"

void ensureParamsExistsAndIsValid(const std::map<std::string, std::string> & params, const std::string & paramName)
{
    auto param = params.find(paramName);
    if (param == params.end() || param->second.empty())
    {
        throw std::invalid_argument(paramName);
    }
}

void ensureParamsExistsAndIsValid(std::map<std::string, std::string> params, const std::string & paramName, const std::string & defaultValue)
{
    auto param = params.find(paramName);
    if (param == params.end() || param->second.empty())
    {
        params[paramName] = defaultValue;
    }
}

std::string extractDateOrDefaultDate(const std::map<std::string, std::string> & measureParams)
{
    if (auto param = measureParams.find(ARG_DATE); param != measureParams.end())
        return param->second;

    return Poco::DateTimeFormatter::format(Poco::Timestamp(), Poco::DateTimeFormat::ISO8601_FRAC_FORMAT);
}

void HTTPRouteTakeMeasure::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    std::map<std::string, std::string> measureParams;
    LOGGER.debug("[HandleRequest] TakeMeasure");
    try
    {
        measureParams = Converter::jsonParamsStream2StringsMap(request.stream());
        ensureParamsExistsAndIsValid(measureParams, ARG_PATIENT_ID);
        ensureParamsExistsAndIsValid(measureParams, ARG_STRIP_TYPE);
        ensureParamsExistsAndIsValid(measureParams, ARG_STRIP_BATCH_ID);
        measureParams[ARG_DATE] = extractDateOrDefaultDate(measureParams);
    }
    catch (std::exception & ex) {
        LOGGER.debug("[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        MeasuresManager measuresManager;
        auto result = measuresManager.runMeasure(measureParams[ARG_PATIENT_ID],
                                                 measureParams[ARG_STRIP_TYPE],
                                                 measureParams[ARG_STRIP_BATCH_ID],
                                                 measureParams[ARG_DATE]);

        LOGGER.information("Sending Measure Result: %f", result.Result);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/json");

        auto & ostream = response.send();
        ostream << result.toJSON();

        LOGGER.debug("Sending Done");
    }
    catch(const std::exception &ex)
    {
        LOGGER.debug("[HandleRequest] TakeMeasure failed : %s", std::string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}
