//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/08/19.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "IRepository.h"
#include <unistd.h>
namespace fs = std::filesystem;

IRepository::IRepository(const std::string &repositoryPath)
        : _repositoryPath(repositoryPath)
{
}

SpaceInfo IRepository::getSpaceInfo() const
{
    auto path = fs::path(_repositoryPath);
    if (path.is_relative())
        path = fs::canonical(fs::current_path() / path);
    SpaceInfo info = std::filesystem::space(path);
    info.path = path.string();
    return info;
}

void IRepository::syncWrites() const
{
#ifndef WIN32
    ::sync();
#endif
}
