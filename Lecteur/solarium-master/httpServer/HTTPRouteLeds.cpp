//
// Created by Mario Valdivia on 2020/06/24.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouteLeds.h"
#include <string>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>

using namespace Poco::Net;
using namespace Poco::JSON;

#include "../tools/Converter.h"
#include "../device/SolariumDevice.h"
#include "models/LEDParams.h"
#include "../SolariumApp.h"

const static std::string ROUTE_PREFIX = "/leds/";

void HTTPRouteLeds::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

    std::string ledType = request.getURI().substr(ROUTE_PREFIX.size());

    LOGGER.debug("trying to control led type : [%s]", ledType);

    LEDParams ledParams;
    try
    {
        ledParams = LEDParams::parse(request.stream(), ledType);
    }
    catch (const std::runtime_error &ex)
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << "Unknown led type";
        return;
    }
    catch (const std::exception &ex)
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        auto result = SolariumDevice::instance().setLEDs(ledParams);
        result.wait();

        response.send() << "OK";

    }
    catch (const std::exception &ex)
    {
        LOGGER.debug("[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }
}
