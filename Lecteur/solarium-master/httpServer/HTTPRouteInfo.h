//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTEINFO_H
#define SOLARIUM_HTTPROUTEINFO_H

#include <Poco/Net/HTTPRequestHandler.h>


class HTTPRouteInfo : public Poco::Net::HTTPRequestHandler {
public:
    HTTPRouteInfo();

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTEINFO_H
