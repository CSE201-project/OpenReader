//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/21.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "HTTPRouteDiagnostics.h"

#include "../SolariumApp.h"
#include "../device/SolariumDevice.h"

#include <exception>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <device/DeviceInfoFormatter.h>

using namespace Poco::Net;

#include "tools/Converter.h"

#define ARG_DIAG_TYPE   "type"
#define ARG_LED_INTENSITY   "led_intensity"
#define ARG_DURATION   "duration"

void sendLightSensorsDiag(std::ostream& output, std::vector<SolariumDevice::TLightSensorsTimestamp> values)
{
    output << "Timestamp (ms);Sensor 1 UV;Sensor 1 Visible;Sensor 2 UV;Sensor 2 Visible;" << std::endl;
    for (const auto & sensorsValue : values)
    {
        output << (sensorsValue.timestamp / 1000) << ";";
        if (sensorsValue.sensor1.has_value())
            output << sensorsValue.sensor1.value().uv << ";" << sensorsValue.sensor1.value().visible << ";";
        else
            output << ";;";
        if (sensorsValue.sensor2.has_value())
            output << sensorsValue.sensor2.value().uv << ";" << sensorsValue.sensor2.value().visible << ";";
        else
            output << ";;";
        output << std::endl;
    }
}

void HTTPRouteDiagnostics::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
    LOGGER.debug("[HandleRequest] Diagnostics");

    std::map<std::string, std::string> params;
    try
    {
        params = Converter::jsonParamsStream2StringsMap(request.stream());
    }
    catch (std::exception & ex) {
        LOGGER.debug("[HandleRequest] exception %s", std::string(ex.what()));

        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.setContentType("text/plain");
        response.send() << ex.what();
        return;
    }

    try
    {
        uint8_t uvLedIntensity = 255;
        if (auto it = params.find(ARG_LED_INTENSITY); it != params.end())
        {
            try {
                uvLedIntensity = (uint8_t)std::atoi(it->second.c_str());
            }
            catch (const std::exception & ex) {
                uvLedIntensity = 255;
            }
        }

        int32_t duration = 10;
        if (auto it = params.find(ARG_DURATION); it != params.end())
        {
            try {
                duration = std::atoi(it->second.c_str());
            }
            catch (const std::exception & ex) {
                duration = 10;
            }
        }

        if (params.find(ARG_DIAG_TYPE) != params.end())
        {
            auto diagType = params[ARG_DIAG_TYPE];
            auto & device = SolariumDevice::instance();

            if (diagType == "light_sensors")
            {
                auto future = device.runLightSensorsDiagnostics(std::make_pair(uvLedIntensity, duration));
                future.wait();

                response.setContentType("text/csv");
                sendLightSensorsDiag(response.send(), future.data());
            }
            else if (diagType == "sensors")
            {
                auto future = device.getSensorsDiagnostics(Poco::Void());
                future.wait();

                response.setContentType("application/json");
                response.send() << DeviceInfoFormatter::toJSON(device.getHostname(), future.data(), SolariumApp::version());
            }
        }

        LOGGER.debug("Diagnostics done");
    }
    catch(std::exception &ex)
    {
        LOGGER.debug("[HandleRequest] Diagnostics failed miserably : %s", std::string(ex.what()));
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << ex.what();
    }
}
