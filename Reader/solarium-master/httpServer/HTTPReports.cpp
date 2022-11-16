//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/02.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "HTTPReports.h"
#include "reports/Reports.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <repository/AnalysisRepository.h>

using namespace Poco::Net;

void HTTPReports::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    std::string analysisId = "123";
    AnalysisRepository repository;

    auto analysis = repository.getAnalysis(analysisId);
    if (analysis.empty())
    {
        response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        response.send();
    }
    else
    {
        auto report = Reports::createReport(&analysis.at(0));

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("application/pdf");
        response.sendBuffer(report.data(), report.size());
    }
}
