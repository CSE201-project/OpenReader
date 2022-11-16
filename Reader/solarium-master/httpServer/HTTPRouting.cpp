//
// Created by Mario Valdivia on 2020/06/04.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouting.h"

#include <functional>
#include <tuple>
#include <string>
#include <regex>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "../SolariumApp.h"

#include "HTTPDefaultRoute.h"
#include "HTTPRouteInfo.h"
#include "HTTPNotFound.h"
#include "HTTPRouteSnapshot.h"
#include "HTTPRouteFiles.h"
#include "HTTPRouteLeds.h"
#include "HTTPRouteTakeMeasure.h"
#include "HTTPRouteBrowsePatient.h"
#include "HTTPRouteBrowseAnalysis.h"
#include "HTTPRouteStudyComments.h"
#include "HTTPMeasureCalibration.h"
#include "HTTPRouteDiagnostics.h"
#include "HTTPRouteStudies.h"
#include "HTTPRouteStudySequence.h"
#include "HTTPParamsFinder.h"
#include "HTTPReports.h"

HTTPRouting::HTTPRouting()
{
}

const std::tuple<std::regex, std::function<HTTPRequestHandler *()>> HttpRoutes[]{
    { std::regex("\\/"), []() { return new HTTPDefaultRoute(); }},
    { std::regex("\\/info(\\?.*)?$"), []() { return new HTTPRouteInfo(); }},
    { std::regex("\\/snapshot"), []() { return new HTTPRouteSnapshot(); }},
    { std::regex("\\/leds/.*"), []() { return new HTTPRouteLeds(); }},
    { std::regex("\\/files(\\/.*)?$"), []() { return new HTTPRouteFiles(); }},
    { std::regex("\\/measure"), []() { return new HTTPRouteTakeMeasure(); }},
    { std::regex("\\/calibration-measure"), [](){ return new HTTPMeasureCalibration(); } },
    { std::regex("\\/diagnostics"), [](){ return new HTTPRouteDiagnostics(); } },
    { std::regex("\\/studies(\\?.*)?$"), [](){ return new HTTPRouteStudies(); } },
    { std::regex("\\/browsePatient(\\?.*)?$"), []() { return new HTTPRouteBrowsePatient(); }},
    { std::regex("\\/browseAnalysis(\\?.*)?$"), []() { return new HTTPRouteBrowseAnalysis(); }},
    { std::regex("\\/studyComment"), []() { return new HTTPRouteStudyComment(); }},
    { std::regex("\\/sequence$"), [](){ return new HTTPRouteStudySequence(); }},
    { std::regex("\\/finder$"), [](){ return new HTTPParamsFinder(); }},
    { std::regex("\\/reports$"), [](){ return new HTTPReports(); }},
};

HTTPRequestHandler *getRouteHandler(const HTTPServerRequest &request)
{
    const auto &requestedURI = request.getURI();

    poco_information_f3(LOGGER, "Requesting [%s %s] from [%s]", request.getMethod(), requestedURI, request.clientAddress().toString());

    for (const auto &[regex, handler] : HttpRoutes)
    {
        if (std::regex_match(requestedURI, regex))
            return handler();
    }

    return new HTTPNotFound();
}

HTTPRequestHandler *HTTPRouting::createRequestHandler(const HTTPServerRequest &request)
{
    auto routeHandler = getRouteHandler(request);

    class _corsHandler : public HTTPRequestHandler
    {
        std::unique_ptr<HTTPRequestHandler> _routeHandler;

    public:
        _corsHandler(HTTPRequestHandler *handler) : _routeHandler(handler) {}

        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
        {
            response.add("Access-Control-Allow-Origin", "*");
            response.add("Access-Control-Allow-Headers", "*");
            response.add("Access-Control-Allow-Methods", "GET,POST,DELETE,OPTIONS");
            _routeHandler->handleRequest(request, response);
        }
    };

    return new _corsHandler(routeHandler);
}
