//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/03/11.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_HTTPTOOLS_H
#define SOLARIUM_HTTPTOOLS_H

#include <string>
#include <Poco/Net/HTTPServerRequest.h>

std::string getRequiredQueryParam(Poco::Net::HTTPServerRequest &request, const std::string & paramName);
std::string getQueryParam(Poco::Net::HTTPServerRequest &request, const std::string & paramName, const std::string & defaultValue);

#endif //SOLARIUM_HTTPTOOLS_H
