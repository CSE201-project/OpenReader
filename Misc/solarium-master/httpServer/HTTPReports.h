//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/02.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPREPORTS_H
#define SOLARIUM_HTTPREPORTS_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPReports : public Poco::Net::HTTPRequestHandler  {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPREPORTS_H
