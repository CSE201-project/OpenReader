//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/03.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "StudiesRepository.h"
#include "configure.h"

#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include <sstream>
#include <regex>

#include <Poco/DateTimeFormatter.h>
#include <Poco/Format.h>

#include "tools/FSTools.h"

fs::path getStudyParamPath(const fs::path &basePath,
                           const std::string &studyName,
                           const std::string &studyDate,
                           uint8_t paramIndex);

StudiesRepository::StudiesRepository(const std::string &studiesPath)
#ifdef USE_DUMMY_PERIPHERALS
    : IRepository("./data/studies")
#else
    : IRepository(studiesPath)
#endif
{
    if (auto usbkeyMountpoint = FSTools::getUsbStorageMountpoint(); !usbkeyMountpoint.empty())
        _repositoryPath = usbkeyMountpoint;

    fs::create_directories(_repositoryPath);
}

fs::path getStudyParamPath(const fs::path &basePath,
                           const std::string &studyName,
                           const std::string &studyDate,
                           uint8_t paramIndex,
                           bool isCroppingParam = false)
{
    auto destinationPath = basePath;

    if (studyDate.empty())
    {
        Poco::Timestamp now;
        destinationPath /= Poco::DateTimeFormatter::format(now, "%Y%m%d");
    }
    else
        destinationPath /= studyDate;

    std::ostringstream oss;
    oss << studyName << "-p";
    if (isCroppingParam)
        oss << "Crop";
    else
        oss << unsigned(paramIndex);

    destinationPath /= oss.str();
    return destinationPath;
}

fs::path buildDestinationPath(const fs::path &basePath,
                              const std::string &studyName,
                              const std::string &studyDate,
                              uint8_t paramIndex,
                              bool isCroppingParam = false)
{
    fs::path destinationPath = getStudyParamPath(basePath, studyName, studyDate, paramIndex, isCroppingParam);

    fs::create_directories(destinationPath);

    return destinationPath;
}

fs::path buildDestinationPath(const fs::path &basePath,
                              const std::string &sequenceName)
{
    fs::path destinationPath(basePath);
    destinationPath /= sequenceName;

    fs::create_directories(destinationPath);

    return destinationPath;
}

void saveFile(const fs::path & basePath, const std::string & fileName, const std::string & fileContent)
{
    fs::path filePath(basePath);
    filePath /= fileName;

    std::ofstream destFile(filePath);
    destFile << fileContent;
    destFile.close();
}

void saveFile(const fs::path & basePath, const std::string & fileName, const std::vector<uint8_t> & fileContent)
{
    fs::path filePath(basePath);
    filePath /= fileName;

    std::ofstream destFile(filePath);
    destFile.write((const char*)fileContent.data(), fileContent.size());
    destFile.close();
}

void StudiesRepository::save(const std::string &studyName,
                             const std::string &studyDate,
                             uint8_t paramIndex,
                             const StudySnapshotResult &result)
{
    auto destFolder = buildDestinationPath(_repositoryPath, studyName, studyDate, paramIndex, result.IsCroppingSnapshot);

    std::ostringstream oss;
    oss << "{\"acquisition_params\":" << result.Params << ",\"device\":" << result.SensorsInfo << "}";
    saveFile(destFolder, result.FileName + ".json", oss.str());
    saveFile(destFolder, result.FileName + ".png", result.Snapshot);
}

void removeFile(const fs::path & directory, const std::string & baseFileName, const std::string & extension)
{
    fs::path filepath(directory);
    filepath /= baseFileName + extension;
    if (fs::exists(filepath))
        fs::remove(filepath);
};

bool StudiesRepository::isStudyNameDerived(const std::string & studyName, const std::string & name)
{
    if (studyName.length() > name.length() || name.length() > (studyName.length() + 4))
        return false;

    if (name.compare(0, studyName.length(), studyName))
        return false;

    std::regex rgPartName("-p\\d{1,2}");
    auto partName = name.substr(studyName.length());
    return std::regex_match(partName, rgPartName);
}

void StudiesRepository::remove(const std::string &studyName, const std::string &studyDate, const std::string &fileName)
{
    fs::path destFolder(_repositoryPath);
    destFolder /= studyDate;
    for (auto & entry : fs::directory_iterator(destFolder))
    {
        if (entry.is_directory() && isStudyNameDerived(studyName, entry.path().filename().string()))
        {
            removeFile(entry.path(), fileName, ".json");
            removeFile(entry.path(), fileName, ".png");
        }
    }
}

void StudiesRepository::saveSequenceSnap(const std::string sequenceName, uint32_t index, const std::vector<uint8_t> snapshot)
{
    auto destFolder = buildDestinationPath(_repositoryPath, sequenceName);

    std::string snapName = Poco::format("%s-%03u.png", sequenceName, index);
    saveFile(destFolder, snapName, snapshot);
}

uint32_t StudiesRepository::getFreeSpace() const
{
    fs::space_info spaceInfo = fs::space(_repositoryPath);
    return spaceInfo.available;
}

void StudiesRepository::saveParamsFinder(const std::string &sequenceName, const std::string &filename, const std::vector<uint8_t> snapshot)
{
    auto destFolder = buildDestinationPath(_repositoryPath, std::string("paramsfinder-").append(sequenceName));

    saveFile(destFolder, filename, snapshot);
}

void StudiesRepository::saveParamsFinder(const std::string &sequenceName, const std::string &filename, std::vector<std::tuple<int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, std::vector<double>, std::vector<double>>> values)
{
    auto destFolder = buildDestinationPath(_repositoryPath, std::string("paramsfinder-").append(sequenceName));
    fs::path filePath(destFolder);
    filePath /= filename;

    std::ofstream destFile(filePath);
#ifdef USE_UEYE_CAMERA
    destFile << "Master Gain;Red Gain;Green Gain;Blue Gain;UV Sensor 1;UV Sensor 2;Algo ProxNoise;Algo ExpoFit;Algo Poly2Fit;Algo Test-ProxNoise;BetweenNoise;ProxNoise;TL;Amplitude(TL-ProxNoise);Profile" << std::endl;
#elif defined(USE_V4L2_CAMERA)
    destFile << "Brightness;Contrast;Saturation;Hue;UV Sensor 1;UV Sensor 2;Algo ProxNoise;Algo ExpoFit;Algo Poly2Fit;Algo Test-ProxNoise;BetweenNoise;ProxNoiseCL;CL;Amplitude(CL-ProxNoiseCL);ProxNoiseTL;TL;Amplitude(TL-ProxNoiseTL);Profile" << std::endl;
#else
    destFile << "Param 1;Param 2;Param 2;Param 4;Algo ProxNoise;Algo ExpoFit;Algo Poly2Fit;Algo Test-ProxNoise;BetweenNoise;ProxNoise;TL;Amplitude(TL-ProxNoise);Profile" << std::endl;
#endif
    for (const auto & line : values)
    {
        destFile << std::to_string(std::get<0>(line)) << ";";
        destFile << std::to_string(std::get<1>(line)) << ";";
        destFile << std::to_string(std::get<2>(line)) << ";";
        destFile << std::to_string(std::get<3>(line)) << ";";
        destFile << std::to_string(std::get<4>(line)) << ";";
        destFile << std::to_string(std::get<5>(line)) << ";";
        for (const auto & res : std::get<6>(line))
        {
            destFile << std::to_string(res) << ";";
        }
        for (const auto & res : std::get<7>(line))
        {
            destFile << std::to_string(res) << ";";
        }
        destFile << std::endl;
    }
    destFile.close();
}

void StudiesRepository::saveParamsFinder(const std::string &sequenceName,
                                         const std::string &filename,
                                         const std::vector<std::string> & headers,
                                         std::vector<std::tuple<std::vector<int>, int32_t, int32_t, std::vector<double>, std::vector<double>>> values)
{
    auto destFolder = buildDestinationPath(_repositoryPath, std::string("paramsfinder-").append(sequenceName));
    fs::path filePath(destFolder);
    filePath /= filename;

    std::ofstream destFile(filePath);
    for(const auto & s : headers)
        destFile << s << ";";

    destFile << "UV Sensor 1;UV Sensor 2;Algo ProxNoise;Algo ExpoFit;Algo Poly2Fit;Algo Test-ProxNoise;BetweenNoise;ProxNoiseCL;CL;Amplitude(CL-ProxNoiseCL);ProxNoiseTL;TL;Amplitude(TL-ProxNoiseTL);Profile" << std::endl;

    for (const auto & line : values)
    {
        for (const auto & p : std::get<0>(line))
        {
            destFile << std::to_string(p) << ";";
        }
        destFile << std::to_string(std::get<1>(line)) << ";";
        destFile << std::to_string(std::get<2>(line)) << ";";
        for (const auto & res : std::get<3>(line))
        {
            destFile << std::to_string(res) << ";";
        }
        for (const auto & res : std::get<4>(line))
        {
            destFile << std::to_string(res) << ";";
        }
        destFile << std::endl;
    }
    destFile.close();
}

void StudiesRepository::saveParamsFinder(const std::string &sequenceName, const std::string &filename, std::vector<std::tuple<uint32_t, int32_t, int32_t, std::vector<double>>> values)
{
    auto destFolder = buildDestinationPath(_repositoryPath, std::string("paramsfinder-").append(sequenceName));
    fs::path filePath(destFolder);
    filePath /= filename;

    std::ofstream destFile(filePath);
    destFile << "Time (s);UV Sensor 1;UV Sensor 2;Algo ProxNoise;Algo ExpoFit;Algo Poly2Fit;Algo Test-ProxNoise;BetweenNoise;ProxNoiseCL;CL;Amplitude(CL-ProxNoiseCL);ProxNoiseTL;TL;Amplitude(TL-ProxNoiseTL)" << std::endl;
    for (const auto & line : values)
    {
        destFile << std::to_string(std::get<0>(line)) << ";";
        destFile << std::to_string(std::get<1>(line)) << ";";
        destFile << std::to_string(std::get<2>(line)) << ";";
        for (const auto & res : std::get<3>(line))
        {
            destFile << std::to_string(res) << ";";
        }
        destFile << std::endl;
    }
    destFile.close();
}
