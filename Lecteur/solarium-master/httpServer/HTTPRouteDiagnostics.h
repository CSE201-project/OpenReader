//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/21.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTEDIAGNOSTICS_H
#define SOLARIUM_HTTPROUTEDIAGNOSTICS_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteDiagnostics : public Poco::Net::HTTPRequestHandler {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTEDIAGNOSTICS_H
