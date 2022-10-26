//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/08/20.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "SpaceInfo.h"

SpaceInfo::SpaceInfo()
{

}

SpaceInfo::SpaceInfo(const std::filesystem::space_info &info)
    : std::filesystem::space_info(info)
{

}
