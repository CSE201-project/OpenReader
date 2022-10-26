//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTESNAPSHOT_H
#define SOLARIUM_HTTPROUTESNAPSHOT_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteSnapshot : public Poco::Net::HTTPRequestHandler  {
public:


    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTESNAPSHOT_H
