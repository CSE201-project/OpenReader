//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/04.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTESTUDIES_H
#define SOLARIUM_HTTPROUTESTUDIES_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteStudies : public Poco::Net::HTTPRequestHandler {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};



#endif //SOLARIUM_HTTPROUTESTUDIES_H
