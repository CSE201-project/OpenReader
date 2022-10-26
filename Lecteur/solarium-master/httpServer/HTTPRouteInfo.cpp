//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouteInfo.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>
using namespace Poco::Net;

#include "../SolariumApp.h"
#include "../device/DeviceInfoFormatter.h"

HTTPRouteInfo::HTTPRouteInfo()
{

}

void sendDefaultInfos(HTTPServerResponse &response)
{
    auto & device = SolariumDevice::instance();
    auto version = SolariumApp::version();
    auto futureSensorsInfos = device.getSensorsInfos(Poco::Void());
    futureSensorsInfos.wait(900);

    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentType("application/json");

    response.send() << DeviceInfoFormatter::toJSON(device.getHostname(), futureSensorsInfos.data(), version);
}

void sendSpaceInfo(HTTPServerResponse &response)
{
    auto & device = SolariumDevice::instance();
    auto spaceInfos = device.getSpaceInfos();

    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    response.setContentType("application/json");

    auto tmp = DeviceInfoFormatter::toJSON(spaceInfos);
    response.send() << tmp;
}

void HTTPRouteInfo::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    try {
        Poco::URI uri(request.getURI());

        if (uri.getQueryParameters().size() == 0)
            sendDefaultInfos(response);
        else {
            for (const auto &  kv : uri.getQueryParameters())
            {
                if (kv.first == "space")
                    sendSpaceInfo(response);
            }
        }
    }
    catch (const Poco::TimeoutException & ex)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_REQUEST_TIMEOUT);
        response.setContentType("text/plain");
        response.send() << "Request Timeout'ed : " << ex.what();
    }
    catch (const std::exception & ex)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.setContentType("text/plain");
        response.send() << "Internal Error " << ex.what();
    }
}
