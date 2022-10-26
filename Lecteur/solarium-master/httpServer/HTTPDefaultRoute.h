//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef HTTPDEFAULTROUTE_H
#define HTTPDEFAULTROUTE_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPDefaultRoute : public Poco::Net::HTTPRequestHandler {
public:
    HTTPDefaultRoute();

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};

#endif // HTTPDEFAULTROUTE_H
