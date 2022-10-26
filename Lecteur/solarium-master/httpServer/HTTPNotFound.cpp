//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPNotFound.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>

using namespace Poco::Net;

void HTTPNotFound::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

    response.setContentType("text/html");
    response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    auto & ostream = response.send();
    ostream << "<html>"
               "<head><title>SolariumApp</title></head>"
               "<body>NOT FOUND !</body>"
               "</html>";
}
