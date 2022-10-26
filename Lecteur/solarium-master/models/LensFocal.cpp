//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/27.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "LensFocal.h"
#include <common/cropper/CropperStructsAndData.h>

LensFocal::LensFocal()
{
    _type = DeviceFocal::UnknownFocal;
}

LensFocal::LensFocal(const std::string &focal, DeviceSeries series)
{
    if (auto it = AvailableFocals().find(focal); it != AvailableFocals().end())
    {
        if (series == Series2)
        {
            _type = (*it == "F060") ? DeviceFocal::ProtoV2_F060 : DeviceFocal::ProtoV2_DefaultFocal;
        }
        else
        {
            if (*it == "F080")
                _type = DeviceFocal::ProtoV3_F080;
            else if (*it == "F060")
                _type = DeviceFocal::ProtoV3_F060;
            else if (*it == "F060_L")
                _type = DeviceFocal::ProtoV3_F060_L;
            else
                _type = DeviceFocal::ProtoV3_F041;
        }
    }
    else
        _type = DeviceFocal::UnknownFocal;
}

int LensFocal::type() const
{
    return _type;
}

