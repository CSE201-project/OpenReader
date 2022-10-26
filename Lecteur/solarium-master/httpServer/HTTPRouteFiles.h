//
// Created by Mario Valdivia on 2020/06/18.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTEFILES_H
#define SOLARIUM_HTTPROUTEFILES_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteFiles : public Poco::Net::HTTPRequestHandler  {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTEFILES_H
