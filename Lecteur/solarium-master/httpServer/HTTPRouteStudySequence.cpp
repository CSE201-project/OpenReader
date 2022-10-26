//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/03/31.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "HTTPRouteStudySequence.h"

#include <vector>
#include <map>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

using namespace Poco::Net;

#include "SolariumApp.h"
#include "tools/Converter.h"
#include "measures/MeasuresManager.h"

/*
 * {
 *     "sequence_name" : "sequence-yyyymmdd-hhmmss",
 *     "sequence_interval" : 123,
 *     "sequence_count" : 12,
 *     "awb" : "off",
 *     "exposure" : 60,
 *     ...
 *     "LEDs_params" : {
 *         "visible" : {
 *         },
 *         "uv" : {
 *         }
 *     }
 *  },
 *
 */

#define ARG_SEQUENCE_INTERVAL "sequence_interval"
#define ARG_SEQUENCE_COUNT "sequence_count"

void HTTPRouteStudySequence::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    uint32_t sequenceInterval = 10;
    uint32_t sequenceCount = 3;
    std::map<std::string, std::string> studyParams;

    try
    {
        studyParams = Converter::jsonParamsStream2StringsMap(request.stream());

        auto extractParam = [&studyParams](const std::string paramName) {
            if (auto arg = studyParams.find(paramName); arg == studyParams.end())
                throw std::runtime_error(std::string("Parameter not found : ").append(paramName));
            else
                return arg->second;
        };

        sequenceInterval = std::stoul(extractParam(ARG_SEQUENCE_INTERVAL));
        sequenceCount = std::stoul(extractParam(ARG_SEQUENCE_COUNT));
    }
    catch (std::exception & ex) {
        poco_debug_f1(LOGGER, "[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        MeasuresManager measuresManager;
        measuresManager.runSequence(studyParams, sequenceInterval, sequenceCount);
        poco_information_f1(LOGGER, "Took %?u Pictures", sequenceCount);
        response.send();

    }
    catch(std::exception &ex)
    {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}
