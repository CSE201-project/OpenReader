//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPNOTFOUND_H
#define SOLARIUM_HTTPNOTFOUND_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPNotFound : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPNOTFOUND_H
