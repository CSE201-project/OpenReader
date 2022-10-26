//
// Created by Mario Valdivia on 2020/06/18.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouteFiles.h"
#include "../SolariumApp.h"
#include "configure.h"

#include <regex>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/File.h>

const static std::string ROUTE_PREFIX = "/files";

void HTTPRouteFiles::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

    auto fileNameRequested = request.getURI().substr(ROUTE_PREFIX.size());

    if (fileNameRequested.find("/../") != std::string::npos)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << "Bad request";
        return;
    }

    fileNameRequested = std::regex_replace(fileNameRequested, std::regex("^/*"), "");
#ifdef USE_DUMMY_PERIPHERALS
    Poco::Path filePath("./data/");
#else
    Poco::Path filePath("/data/analysis/");
#endif
    filePath.append(fileNameRequested);
    Poco::File fileRequested(filePath);

    LOGGER.debug("fileNameRequested : [%s]", filePath.toString());

    if (fileRequested.exists())
    {
        if (filePath.getExtension() == "png")
            response.sendFile(fileRequested.path(), "image/png");
        else if (filePath.getExtension() == "json")
            response.sendFile(fileRequested.path(), "application/json");
        else if (filePath.getExtension() == "pdf")
            response.sendFile(fileRequested.path(), "application/pdf");
        else
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            response.send();
        }
    } else {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.send();
    }
}

