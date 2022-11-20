#include "HTTPRouteStudyComments.h"

#include <map>
#include <fstream>
#include <iostream>
#include <exception>
#include <Poco/URI.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "../SolariumApp.h"
#include "../tools/Converter.h"

using namespace std;
using namespace Poco::Net;

void HTTPRouteStudyComment::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
    LOGGER.debug("[HandleRequest] Study Comment");
    map<string, string> params;

    try
    {
        params = Converter::jsonParamsStream2StringsMap(request.stream());
    }
    catch (exception &ex)
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << "Bad Request";
    }

    try
    {
        cout << "comment:[" << params.at("comment") << "]" << endl;

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("text/plain");
        response.send() << "Comment OK";
    }
    catch (exception &ex)
    {
        LOGGER.debug("[HandleRequest] Browse Patient request failed miserably : %s", string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}