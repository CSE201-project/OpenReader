//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/05.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTETAKEMEASURE_H
#define SOLARIUM_HTTPROUTETAKEMEASURE_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteTakeMeasure : public Poco::Net::HTTPRequestHandler {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};


#endif //SOLARIUM_HTTPROUTETAKEMEASURE_H
