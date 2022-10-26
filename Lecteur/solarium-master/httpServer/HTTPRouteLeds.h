//
// Created by Mario Valdivia on 2020/06/24.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTELEDS_H
#define SOLARIUM_HTTPROUTELEDS_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteLeds : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTELEDS_H
