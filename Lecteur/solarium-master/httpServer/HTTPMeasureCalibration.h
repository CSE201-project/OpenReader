//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/01/18.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPMEASURECALIBRATION_H
#define SOLARIUM_HTTPMEASURECALIBRATION_H

#include <Poco/Net/HTTPRequestHandler.h>

class HTTPMeasureCalibration : public Poco::Net::HTTPRequestHandler {
public:

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
};


#endif //SOLARIUM_HTTPMEASURECALIBRATION_H
