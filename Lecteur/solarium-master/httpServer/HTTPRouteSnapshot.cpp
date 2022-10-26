//
// Created by Mario Valdivia on 2020/06/09.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouteSnapshot.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

using namespace Poco::Net;

#include "tools/Converter.h"
#include "device/SolariumDevice.h"
#include "SolariumApp.h"
#include "measures/MeasureConfig.h"

void HTTPRouteSnapshot::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

    ICamera::TCameraParams cameraParameters;
    try
    {
        cameraParameters = Converter::jsonParamsStream2StringsMap(request.stream());
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
        std::vector<uint8_t> picture;

        if (auto calibIter = cameraParameters.find("use_calibration"); calibIter != cameraParameters.end())
        {
            const auto & config = SolariumDevice::instance().getMeasureConfig();

            auto cameraParams = config.getCameraParams();
            auto uvLeds = config.getLEDParams(LEDParams::UV);
            auto visibleLeds = config.getLEDParams(LEDParams::Visible);

            auto futurePicture = SolariumDevice::instance().takePictureForMeasure({
                                                                                cameraParams,
                                                                                uvLeds,
                                                                                visibleLeds
                                                                        });
            futurePicture.wait();
            auto pictureMat = futurePicture.data();
            picture = Converter::cvMatToPng(pictureMat);
        }
        else {
            auto futurePicture = SolariumDevice::instance().takePicture(cameraParameters);
            futurePicture.wait();
            picture = futurePicture.data();
        }

        poco_information_f1(LOGGER, "Sending TakePicture size: %z", picture.size());
        response.setContentType("image/png");
        response.sendBuffer(picture.data(), picture.size());

        poco_information(LOGGER, "Sending Done");
    }
    catch (const Poco::TimeoutException & ex)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_REQUEST_TIMEOUT);
        response.setContentType("text/plain");
        response.send() << "Request Camera Timeout'ed : " << ex.what();
    }
    catch(std::exception &ex)
    {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.setContentType("text/plain");
        response.send() << "Internal Error : " << std::string(ex.what());
    }
}
