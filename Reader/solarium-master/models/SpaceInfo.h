//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/08/20.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_SPACEINFO_H
#define SOLARIUM_SPACEINFO_H

#include <filesystem>

class SpaceInfo : public std::filesystem::space_info {
public:
    std::string path;

    SpaceInfo();
    SpaceInfo(const std::filesystem::space_info &info);
};

#endif //SOLARIUM_SPACEINFO_H
