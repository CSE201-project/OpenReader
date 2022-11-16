#include "FSTools.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>

std::string FSTools::getFileContent(const std::string &path)
{
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content;
}

std::vector<char> FSTools::getBinaryContent(const std::string &path)
{
    std::ifstream file(path);
    std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content;
}

std::vector<uint8_t> FSTools::getBinaryContentAsUInt8(const std::string &path)
{
    std::ifstream file(path);
    std::vector<uint8_t> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content;
}


std::string FSTools::getUsbStorageMountpoint()
{
    std::string usbkeyMountpoint = "/run/media/sda1";

    if (std::filesystem::exists(usbkeyMountpoint))
        return usbkeyMountpoint;
    else
        return std::string();
}
