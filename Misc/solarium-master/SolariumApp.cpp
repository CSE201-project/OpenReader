//
// Created by Mario Valdivia on 2020/06/04.
// Copyright (c) 2020 LumediX. All rights reserved.
////

#include "SolariumApp.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <fstream>
#include <filesystem>

using namespace std;

#include <Poco/Util/Option.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/PropertyFileConfiguration.h>

#include <Poco/Net/HTTPServer.h>
#include <measures/MeasureConfig.h>

using namespace Poco::Util;
using namespace Poco::Net;

#define CONFIGURATION_FILENAME "Solarium.conf"

#include "httpServer/HTTPRouting.h"
#include "device/SolariumDevice.h"
#include "repository/AnalysisRepository.h"
#include "reports/Reports.h"
#include "tools/ImgTools.h"
#include "GitVersion.h"

SolariumApp::SolariumApp()
 : _configFile(new Poco::Util::PropertyFileConfiguration())
 , _currentApplicationMode(Server)
{
}

SolariumApp::~SolariumApp()
{
}

void SolariumApp::initialize(Poco::Util::Application &self)
{
    loadConfiguration();
    loadDefaultValues();

    config().addWriteable(_configFile, -200);

    std::filesystem::path configFilePath = config().getString("application.dir");
    configFilePath /= CONFIGURATION_FILENAME;
    _configFilePath = configFilePath.string();

    if (std::filesystem::exists(configFilePath))
        _configFile->load(_configFilePath);

    Application::initialize(self);
    logger().setLevel(Poco::Message::PRIO_DEBUG);
}

void SolariumApp::defineOptions(Poco::Util::OptionSet &options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
        Option("help", "h", "display argument help information")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<SolariumApp>(this, &SolariumApp::handleHelp)));

    options.addOption(
            Option("rebuild-db", "", "Rebuild Database from repository")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<SolariumApp>(this, &SolariumApp::handleRebuildDB)));

    options.addOption(
            Option("report", "", "Build a report for an AnalysisId")
            .required(false)
            .argument("analysisId")
            .callback(OptionCallback<SolariumApp>(this, &SolariumApp::handleReport)));
}

int SolariumApp::main(const std::vector<std::string> &args)
{
    switch (_currentApplicationMode)
    {
        case ShowHelp:
            break;
        case RebuildDB:
            logger().information("Rebuilding DB from Repository");
            AnalysisRepository::initDataBaseFromRepository();
            break;
        case GenReport:
            logger().information("Generating report");
            genReport("");
            break;
        default:
            serverLoop();
            break;
    }
    return Application::EXIT_OK;
}

void SolariumApp::handleHelp(const std::string &name, const std::string &value)
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A web server that commands the Solarium Device.");
    helpFormatter.format(std::cout);
    stopOptionsProcessing();
    _currentApplicationMode = ShowHelp;
}

void SolariumApp::handleRebuildDB(const std::string &name, const std::string &value)
{
    stopOptionsProcessing();
    _currentApplicationMode = RebuildDB;
}

void SolariumApp::handleReport(const string &name, const string &value)
{
    stopOptionsProcessing();
    _currentApplicationMode = GenReport;

    AnalysisRepository repository;
    auto analysis = repository.getAnalysisById(value);
    if (!analysis.PatientId.empty())
    {
        try {
            MeasureConfig config("covid-blood");

            analysis.StripImage = ImgTools::extractROIForReport(analysis.StripImage, 0, 0, config.getPixelsToMm(),
                                                                config.getPadding());
            analysis.PdfReport = Reports::createReport(&analysis);

            auto result = Reports::createReport(&analysis);
            std::ofstream output("./report.pdf");
            output.write((const char*)result.data(), result.size());
            output.close();
        }
        catch (const std::runtime_error & ex)
        {
            std::cerr << "Error Generating Report  : " << std::string(ex.what()) << std::endl;
        }
        catch (const std::exception & ex)
        {
            std::cerr << "Error Generating Report  : " << std::string(ex.what()) << std::endl;
        }
    }
}

std::string SolariumApp::version()
{
    if (!GitVersion::Tag.empty())
        return GitVersion::Tag;

    std::ostringstream oss;
    if (GitVersion::Branch.empty())
        oss << "HEAD detached at ";
    else
        oss << GitVersion::Branch << ":";

    oss << GitVersion::Revision;
    return oss.str();
}

void SolariumApp::loadDefaultValues()
{
    config().setUInt("http.port", 8080);

#ifdef USE_UEYE_CAMERA
    config().setUInt("camera.roi_offset", 0);
    config().setUInt("camera.roi_x", 384);
    config().setUInt("camera.roi_y", 310);
#else
    config().setUInt("camera.roi_offset", 0);
    config().setUInt("camera.roi_x", 0);
    config().setUInt("camera.roi_y", 0);
#endif
}

void SolariumApp::saveConfigToDisk()
{
    _configFile->save(_configFilePath);
}

void SolariumApp::serverLoop()
{
    // Starting Devices
    logger().information("Initializing Device Peripherals");
    SolariumDevice::instance().initialize();

    logger().information("Initializing DB");
    AnalysisRepository::ensureDataBaseExists();

    // Starting HTTP Server
    uint16_t serverPort = config().getUInt("http.port");

    logger().information("Initializing Solarium Server (%s) at %?u", version(), serverPort);

    ServerSocket serverSocket(serverPort);

    HTTPServer httpServer(
            new HTTPRouting(),
            serverSocket,
            new HTTPServerParams);

    httpServer.start();
    waitForTerminationRequest();
    httpServer.stop();
    SolariumDevice::instance().uninitialize();
}

void SolariumApp::genReport(const std::string & analysisId)
{

}
