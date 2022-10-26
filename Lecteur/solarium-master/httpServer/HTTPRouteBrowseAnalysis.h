//
// Created by Mario Valdivia on 2020/06/18.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPROUTEBROWSEANALYSIS_H
#define SOLARIUM_HTTPROUTEBROWSEANALYSIS_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPRouteBrowseAnalysis : public Poco::Net::HTTPRequestHandler  {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

};


#endif //SOLARIUM_HTTPROUTEBROWSEANALYSIS_H
