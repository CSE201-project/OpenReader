//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/03/11.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "HTTPTools.h"
#include <Poco/URI.h>

using namespace Poco::Net;

std::string getRequiredQueryParam(HTTPServerRequest &request, const std::string & paramName)
{
    std::string paramValue;
    Poco::URI uri(request.getURI());
    for (const auto&kv : uri.getQueryParameters())
    {
        if (kv.first == paramName)
        {
            paramValue = kv.second;
            break;
        }
    }

    if (paramValue.empty())
        throw Poco::NotFoundException("Missing Params");

    return paramValue;
}

std::string getQueryParam(HTTPServerRequest &request, const std::string & paramName, const std::string & defaultValue)
{
    std::string paramValue;
    bool paramFound = false;
    Poco::URI uri(request.getURI());
    for (const auto&kv : uri.getQueryParameters())
    {
        if (kv.first == paramName)
        {
            paramValue = kv.second;
            paramFound = true;
            break;
        }
    }

    return paramFound ? paramValue : defaultValue;
}
