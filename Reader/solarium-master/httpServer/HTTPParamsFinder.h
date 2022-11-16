//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/06/09.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPPARAMSFINDER_H
#define SOLARIUM_HTTPPARAMSFINDER_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPParamsFinder : public Poco::Net::HTTPRequestHandler {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};


#endif //SOLARIUM_HTTPPARAMSFINDER_H
