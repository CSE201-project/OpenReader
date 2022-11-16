//
// Created by Mario Valdivia on 2020/06/08.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include <sstream>
#include <iomanip>
#include "DeviceInfoFormatter.h"
#include "../tools/DebugInfo.h"
#include "models/SpaceInfo.h"

/*
 * {
 *   "name" : "solarium",
 *   "version" : "0.1",
 *   "temperature_sensors" : [
 *      {
 *          "temperature" : 34,
 *          "humidity" : 67
 *      },
 *      {
 *          "temperature" : 34.5
 *          "humidity" : 65
 *      }
 *   ],
 *   "light_sensors" : [
 *      {
 *          "visible" : 345
 *          "uv" : 65
 *      },
 *      {
 *          "visible" : 345
 *          "uv" : 65
 *      }
 *   ],
 *   "battery" : {
 *      "status" : "discharging",
 *      "charge" : 45
 *   }
 * }
 */


std::string toJson(const Poco::Timespan * timespan = nullptr)
{
    if (timespan == nullptr)
        return "";

    std::ostringstream oss;
    oss << ",\"reading_time\":" << timespan->milliseconds();
    return oss.str();
}

std::string convertToJSON(const std::optional<ITemperatureSensor::TResult> & sensorValue, const Poco::Timespan * timespan = nullptr)
{
    std::ostringstream oss;
    if (sensorValue.has_value())
        oss << std::fixed << std::setprecision(1) << "{\"temperature\":" << sensorValue.value().temperature << ",\"humidity\":" << sensorValue.value().humidity << toJson(timespan) << "}";
    else
        oss << "{\"temperature\":\"#err\",\"humidity\":\"#err\"" << toJson(timespan) << "}";
    return oss.str();
}

std::string convertToJSON(const std::optional<ILightSensor::TResult> & sensorValue, const Poco::Timespan * timespan = nullptr)
{
    std::ostringstream oss;
    if (sensorValue.has_value())
        oss << "{\"uv\":" << sensorValue.value().uv << ",\"visible\":" << sensorValue.value().visible << toJson(timespan) << "}";
    else
        oss << "{\"uv\":\"#err\",\"visible\":\"#err\"" << toJson(timespan) << "}";
    return oss.str();
}

std::string convertToJSON(const std::optional<IBattery::TReading> & batteryReading, const Poco::Timespan * timespan = nullptr)
{
    std::ostringstream oss;
    if (batteryReading.has_value())
        oss << R"({"status":")" << (batteryReading.value().charging ? "charging" : "discharging") << R"(","charge":)" << +batteryReading.value().charge << toJson(timespan) << "}";
    else
        oss << R"({"status":"#err","charge":"#err")" << toJson(timespan) << "}";
    return oss.str();
}

std::string DeviceInfoFormatter::toJSON(const std::string & deviceName, const SolariumDevice::TSensorsInfos &deviceSensors, const std::string &appVersion)
{
    std::ostringstream oss;

    oss << R"({"name":")" << deviceName << "\"," << "\"version\":\"" << appVersion << "\"";

    oss << ",\"temperature_sensors\":[" << convertToJSON(deviceSensors.temperatures[0])  << "," << convertToJSON(deviceSensors.temperatures[1]) << "]";

    oss << ",\"light_sensors\":[" << convertToJSON(deviceSensors.lights[0])  << "," << convertToJSON(deviceSensors.lights[1]) << "]";

    oss << ",\"battery\":" << convertToJSON(deviceSensors.battery);

    oss << "}";

    return oss.str();
}

std::string DeviceInfoFormatter::toJSON(const std::string & deviceName, const SolariumDevice::TSensorsDiagnostics &deviceSensors, const std::string &appVersion)
{
    std::ostringstream oss;

    oss << R"({"name":")" << deviceName << "\"," << "\"version\":\"" << appVersion << "\"";

    oss << ",\"temperature_sensors\":[" << convertToJSON(deviceSensors.temperatures[0].first, &deviceSensors.temperatures[0].second)  << "," << convertToJSON(deviceSensors.temperatures[1].first, &deviceSensors.temperatures[1].second) << "]";

    oss << ",\"light_sensors\":[" << convertToJSON(deviceSensors.lights[0].first, &deviceSensors.lights[0].second)  << "," << convertToJSON(deviceSensors.lights[1].first, &deviceSensors.lights[1].second) << "]";

    oss << ",\"battery\":" << convertToJSON(deviceSensors.battery.first, &deviceSensors.battery.second);

    oss << "}";

    return oss.str();
}

std::string toJSON(const SpaceInfo & info)
{
    std::ostringstream oss;
    oss << "{\"path\":\"" << info.path << "\"";
    oss << ",\"capacity\":" << (info.capacity);
    oss << ",\"free\":" << (info.free);
    oss << ",\"available\":" << (info.available);
    oss << "}";
    return oss.str();
}

std::string DeviceInfoFormatter::toJSON(const SolariumDevice::TSpaceInfos &info)
{
    std::ostringstream oss;
    oss << "{\"data\":" << ::toJSON(info.dataFileSystemInfo);
    oss << ",\"studies\":" << ::toJSON(info.studyFileSystemInfo);
    oss << "}";
    return oss.str();
}
