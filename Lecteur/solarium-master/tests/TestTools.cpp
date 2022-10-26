//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/28.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "TestTools.h"

#include <filesystem>
namespace fs = std::filesystem;

std::string getResourcePath(const std::string &relativePath)
{
    fs::path currentBasePath = fs::path(__FILE__).remove_filename();
    currentBasePath /= relativePath;
    return currentBasePath.string();
}
