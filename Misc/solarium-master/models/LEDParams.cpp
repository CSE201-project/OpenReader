//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/20.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "LEDParams.h"
#include "tools/Converter.h"
#include <sstream>

LEDParams LEDParams::parse(const std::string &paramString, LEDType type)
{
    std::istringstream iss(paramString);
    return parse(iss, type);
}

LEDParams LEDParams::parse(std::istream &istream, LEDParams::LEDType type)
{
    LEDParams ledParams;
    ledParams.ledType = type;

    auto params = Converter::jsonParamsStream2StringsMap(istream);
    if (params.empty())
        return ledParams;

    try {
        ledParams.intensity = std::stoul(params["intensity"]);
        ledParams.enabled = (params["status"] == std::string("enable")) || (params["status"] == std::string("enabled"));
    } catch (...) {
        ledParams.intensity = 0;
        ledParams.enabled = false;
    }
    return ledParams;

}

LEDParams LEDParams::parse(std::istream &istream, const std::string & type)
{
    if (type == "uv")
        return parse(istream, UV);
    else if (type == "visible")
        return parse(istream, Visible);
    else
        throw std::runtime_error("invalid LED type");
}


