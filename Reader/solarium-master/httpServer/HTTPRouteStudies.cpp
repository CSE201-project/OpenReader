//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/04.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "HTTPRouteStudies.h"

#include <map>
#include <exception>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/URI.h>

using namespace Poco::Net;

#include "SolariumApp.h"
#include "tools/Converter.h"
#include "measures/MeasuresManager.h"
#include "repository/StudiesRepository.h"

#define ARG_STUDY_NAME  "study"
#define ARG_STUDY_DATE  "date"
#define ARG_SNAPSHOT_FILENAME   "snapshot"
#define ARG_COMPUTE_CONCENTRATION   "compute_concentration"
#define ARG_SNAPSHOT_PARAMS   "params"
#define ARG_CASSETTE_NAME "cassette_name"

#define HTTP_HEADER_CONCENTRATION   "X-Concentration-Results"

/*
 * {
 *   "study" : "112C",
 *   "snapshot" : "112C_0pgml_N01",
 *   "date" : "20210131",
 *   "compute_concentration" : true,
 *   "params" : [
 *      {
 *          "awb" : "off",
 *          "exposure" : 60,
 *          ...
 *          "LEDs_params" : {
 *              "visible" : {
 *              },
 *              "uv" : {
 *              }
 *          }
 *      },
 *      {
 *          "awb" : "off",
 *          "exposure" : 60,
 *          ...
 *          "LEDs_params" : {
 *              "visible" : {
 *              },
 *              "uv" : {
 *              }
 *          }
 *      }
 *   ]
 * }
 *
 *
 */


void handleSnapshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    std::string studyName, studyDate, snapshotFileName, cassetteName;
    std::vector<std::map<std::string, std::string>> snapshotParams;
    bool computeConcentration = false;
    try
    {
        auto studySnapshotParams = Converter::jsonParamsStream2StringsMap(request.stream());

        auto extractParam = [&studySnapshotParams](const std::string paramName) {
            if (auto arg = studySnapshotParams.find(paramName); arg == studySnapshotParams.end())
                throw std::runtime_error(std::string("Parameter not found : ").append(paramName));
            else
                return arg->second;
        };

        auto extractOptParam = [&studySnapshotParams](const std::string paramName) {
            if (auto arg = studySnapshotParams.find(paramName); arg == studySnapshotParams.end())
                return std::string();
            else
                return arg->second;
        };

        cassetteName = extractParam(ARG_CASSETTE_NAME);
        studyName = extractParam(ARG_STUDY_NAME);
        studyDate = extractParam(ARG_STUDY_DATE);
        snapshotFileName = extractParam(ARG_SNAPSHOT_FILENAME);
        snapshotParams = Converter::jsonArrayParams2StringsMapVector(extractParam(ARG_SNAPSHOT_PARAMS));

        computeConcentration = (extractOptParam(ARG_COMPUTE_CONCENTRATION) == "true");
    }
    catch (std::exception & ex) {
        poco_debug_f1(LOGGER, "[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        MeasuresManager measuresManager;
        auto result = measuresManager.takePicturesForStudy(studyName, studyDate, snapshotFileName, snapshotParams, cassetteName, computeConcentration);

        poco_information_f2(LOGGER, "Took %?u Pictures, Sending Last TakePicture (size: %z)", result.nbPictures, result.lastPicture.size());

        if (computeConcentration)
        {
            response.add("Access-Control-Expose-Headers",HTTP_HEADER_CONCENTRATION);

            for (int i = 0; i < result.concentration.size(); i++)
            {
                response.add(HTTP_HEADER_CONCENTRATION, std::to_string(result.concentration.at(i)));
            }

        }

        response.setContentType("image/png");
        response.sendBuffer(result.lastPicture.data(), result.lastPicture.size());

        poco_information(LOGGER, "Sending Done");
    }
    catch(const std::exception &ex)
    {
        try
        {
            StudiesRepository repository;
            repository.remove(studyName, studyDate, snapshotFileName);
        }
        catch (const std::exception &ex2)
        {
        }


        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}

void handleRemoveSnapshot(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    poco_debug(LOGGER, "Deleting study snapshot");

    std::string studyName, studyDate, snapshotFileName;
    try
    {
        Poco::URI uri(request.getURI());
        for (const auto& kv : uri.getQueryParameters())
        {
            if (kv.first == ARG_STUDY_NAME)
                studyName = kv.second;

            if (kv.first == ARG_STUDY_DATE)
                studyDate = kv.second;

            if (kv.first == ARG_SNAPSHOT_FILENAME)
                snapshotFileName = kv.second;
        }

        if (studyName.empty() || studyDate.empty() || snapshotFileName.empty())
            throw Poco::NotFoundException("Missing params");
    }
    catch (std::exception & ex) {
        poco_debug_f1(LOGGER, "[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        StudiesRepository repository;
        repository.remove(studyName, studyDate, snapshotFileName);
        poco_information_f2(LOGGER, "Deleted all files for %s/%s", studyName, snapshotFileName);
        response.setContentType("text/plain");
        response.send() << "OK";
    }
    catch(std::exception &ex)
    {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}

void HTTPRouteStudies::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    poco_debug_f1(LOGGER, "[HTTPRouteStudies::HandleRequest] Method : %s", request.getMethod());

    if (request.getMethod() == HTTPRequest::HTTP_POST)
    {
        handleSnapshot(request, response);
    }
    else if (request.getMethod() == HTTPRequest::HTTP_DELETE)
    {
        handleRemoveSnapshot(request, response);
    }
    else if (request.getMethod() == HTTPRequest::HTTP_OPTIONS)
    {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.send();
    }
}
