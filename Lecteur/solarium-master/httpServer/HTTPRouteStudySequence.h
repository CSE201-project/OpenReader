//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/03/31.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTESTUDYSEQUENCE_H
#define SOLARIUM_HTTPROUTESTUDYSEQUENCE_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteStudySequence : public Poco::Net::HTTPRequestHandler {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};


#endif //SOLARIUM_HTTPROUTESTUDYSEQUENCE_H
