//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/08/19.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_IREPOSITORY_H
#define SOLARIUM_IREPOSITORY_H

#include <string>
#include <filesystem>
#include "models/SpaceInfo.h"

class IRepository {
protected:
    std::string _repositoryPath;
    IRepository(const std::string & repositoryPath);

public:

    SpaceInfo getSpaceInfo() const;
    void syncWrites() const;
};


#endif //SOLARIUM_IREPOSITORY_H
