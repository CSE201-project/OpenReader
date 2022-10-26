//
// Created by Mario Valdivia on 2020/06/04.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTING_H
#define SOLARIUM_HTTPROUTING_H

#include <Poco/Net/HTTPRequestHandlerFactory.h>

using namespace Poco::Net;

class HTTPRouting : public HTTPRequestHandlerFactory {
public:
    HTTPRouting();

    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override;

};


#endif //SOLARIUM_HTTPROUTING_H
