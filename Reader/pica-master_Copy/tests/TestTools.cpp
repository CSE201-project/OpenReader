//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/12/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch2/catch.hpp>

#include "TestTools.h"

#include <filesystem>
namespace fs = std::filesystem;

std::string getResourcePath(const std::string &relativePath)
{
    fs::path currentBasePath = fs::path(__FILE__).remove_filename();
    currentBasePath /= relativePath;
    return currentBasePath.string();
}
