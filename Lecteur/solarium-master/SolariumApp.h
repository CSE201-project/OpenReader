//
// Created by Mario Valdivia on 2020/06/04.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_SOLARIUMAPP_H
#define SOLARIUM_SOLARIUMAPP_H

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <memory>

class SolariumDevice;

class SolariumApp : public Poco::Util::ServerApplication {
public:
    SolariumApp();

    virtual ~SolariumApp();

    static std::string version();
    void saveConfigToDisk();

protected:
    void initialize(Application &self) override;

    void defineOptions(Poco::Util::OptionSet &options) override;

    void handleHelp(const std::string & name, const std::string & value);
    void handleRebuildDB(const std::string &name, const std::string &value);
    void handleReport(const std::string &name, const std::string &value);

    int main(const std::vector<std::string> &args) override;

private:

    enum ApplicationMode {
        Server,
        ShowHelp,
        RebuildDB,
        GenReport
    };
    ApplicationMode _currentApplicationMode;

    std::string _configFilePath;
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _configFile;

    void loadDefaultValues();


    void serverLoop();
    void genReport(const std::string & analysisId);
};

#define LOGGER SolariumApp::instance().logger()

#endif //SOLARIUM_SOLARIUMAPP_H
