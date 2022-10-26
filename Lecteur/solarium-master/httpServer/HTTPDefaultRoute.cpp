//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPDefaultRoute.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

using namespace Poco::Net;

HTTPDefaultRoute::HTTPDefaultRoute()
{

}

void HTTPDefaultRoute::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {

    response.setContentType("text/html");
    auto & ostream = response.send();
    ostream << "<html>"
               "<head><title>SolariumApp</title></head>"
               "<body>Served by SolariumApp</body>"
               "</html>";
}
