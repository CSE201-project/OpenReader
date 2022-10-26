#include "HTTPRouteBrowseAnalysis.h"

#include <map>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <exception>
#include <Poco/URI.h>
#include <Poco/Path.h>
#include <Poco/DateTime.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "../SolariumApp.h"
#include "../tools/Converter.h"
#include "../repository/ReaderDB.h"
#include "../repository/AnalysisRepository.h"
#include "HTTPTools.h"

using namespace Poco::Net;
using namespace std;

#define ARG_ID  "id"
#define ARG_PATIENT_ID  "patientId"

string createJSON(PatientInfo patient, vector<AnalysisResult> analysisList)
{
    ostringstream oss;

    oss << "{\"id\":\"" << patient.Id << "\",";
    oss << "\"date\":\"" << Poco::DateTimeFormatter::format(patient.CreationDate, Poco::DateTimeFormat::ISO8601_FORMAT) << "\",";
    oss << "\"measures\":[";
    for (int i = 0; i < analysisList.size(); i++)
    {
        oss << analysisList[i].toJSON();
        i == analysisList.size() - 1 ? oss << "" : oss << ",";
    }
    oss << "]}";
    return oss.str();
}

void handleDeleteAnalysis(HTTPServerRequest &request, HTTPServerResponse &response)
{
    try
    {
        std::string analysisId = getRequiredQueryParam(request, ARG_ID);

        AnalysisRepository repository;
        repository.deleteAnalysis(analysisId);

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/plain");
        response.send() << "Analysis '" << analysisId << "' deleted";
    }
    catch (const Poco::NotFoundException &ex)
    {
        poco_debug(LOGGER, "Missing Param");
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << ex.what();
    }
    catch (const exception &ex)
    {
        poco_debug_f1(LOGGER, "Delete Analysis failed : %s", string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}

void handleGetPatientAnalysis(HTTPServerRequest &request, HTTPServerResponse &response)
{
    try
    {
        std::string patientId = getRequiredQueryParam(request, ARG_PATIENT_ID);

        AnalysisRepository repository;
        PatientInfo patient = repository.getPatient(patientId);
        vector<AnalysisResult> analysisList = repository.getAnalysis(patient.Id);

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("application/json");
        response.send() << createJSON(patient, analysisList);

        LOGGER.debug("Sending Done");
    }
    catch (const Poco::NotFoundException &ex)
    {
        poco_debug(LOGGER, "Missing Param");
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << ex.what();
    }
    catch (exception &ex)
    {
        poco_debug_f1(LOGGER, "Browse Analysis failed : %s", string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}

void HTTPRouteBrowseAnalysis::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    if (request.getMethod() == HTTPRequest::HTTP_GET)
    {
        handleGetPatientAnalysis(request, response);
    }
    else if (request.getMethod() == HTTPRequest::HTTP_DELETE)
    {
        handleDeleteAnalysis(request, response);
    }
    else if (request.getMethod() == HTTPRequest::HTTP_OPTIONS)
    {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.send();
    }
}
