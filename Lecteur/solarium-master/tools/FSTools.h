#ifndef FILESYSTEMTOOLS_H
#define FILESYSTEMTOOLS_H

#include <string>
#include <vector>

class FSTools
{
public:

    static std::string getFileContent(const std::string &path);

    static std::vector<char> getBinaryContent(const std::string &path);
    static std::vector<uint8_t> getBinaryContentAsUInt8(const std::string &path);
    static std::string getUsbStorageMountpoint();
};

#endif
