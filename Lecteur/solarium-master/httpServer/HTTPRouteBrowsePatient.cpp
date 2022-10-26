#include "HTTPRouteBrowsePatient.h"

#include <map>
#include <iostream>
#include <exception>
#include <filesystem>
#include <Poco/URI.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "../SolariumApp.h"
#include "../repository/ReaderDB.h"
#include "../repository/AnalysisRepository.h"
#include "HTTPTools.h"

using namespace Poco::Net;
using namespace Poco::JSON;
using namespace std;

#define ARG_START "s"
#define ARG_END "e"
#define ARG_QUERY "q"
#define ARG_PATIENT_ID "id"

string createJSON(const vector<PatientInfo> &patientList)
{
    ostringstream oss;

    oss << "[";
    for (int i = 0; i < patientList.size(); i++)
    {
        oss << patientList[i].toJSON();
        i == patientList.size() - 1 ? oss << "" : oss << ",";
    }
    oss << "]";
    return oss.str();
}

void handleDeletePatient(HTTPServerRequest &request, HTTPServerResponse &response)
{
    AnalysisRepository repository;
    try
    {
        std::string patientId = getRequiredQueryParam(request, ARG_PATIENT_ID);
        repository.deletePatient(patientId);

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/plain");
        response.send() << "Patient '" << patientId << "' has been deleted";
    }
    catch (const Poco::NotFoundException &ex)
    {
        poco_debug(LOGGER, "Missing Param");
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << ex.what();
    }
    catch (exception &ex)
    {
        LOGGER.debug("[HandleRequest] Delete Patient request failed miserably : %s", string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}

void handleSearchPatient(HTTPServerRequest &request, HTTPServerResponse &response)
{
    string start, end, query;
    int limit, offset;

    try
    {
        Poco::URI uri(request.getURI());
        for (const auto&kv : uri.getQueryParameters())
        {
            if (kv.first == ARG_START)
                start = kv.second;
            else if (kv.first == ARG_END)
                end = kv.second;
            else if (kv.first == ARG_QUERY)
                query = kv.second;
        }

        limit = std::atoi(end.c_str());
        offset = std::atoi(start.c_str());
    }
    catch (const exception &ex)
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << "Bad Request : " << std::string(ex.what());
        return;
    }

    AnalysisRepository repository;
    try
    {
        vector<PatientInfo> patientList = repository.searchPatient(query, limit, offset);

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        response.send() << createJSON(patientList);

        LOGGER.debug("Sending Done");
    }
    catch (exception &ex)
    {
        LOGGER.debug("[HandleRequest] Browse Patient request failed miserably : %s", string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}

void HTTPRouteBrowsePatient::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    if (request.getMethod() == HTTPRequest::HTTP_OPTIONS)
    {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/plain");
        response.send() << "Options";
    }
    else if (request.getMethod() == HTTPRequest::HTTP_DELETE)
    {
        handleDeletePatient(request, response);
    }
    else if (request.getMethod() == HTTPRequest::HTTP_GET)
    {
        handleSearchPatient(request, response);
    }
}
