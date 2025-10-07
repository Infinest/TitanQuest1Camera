#pragma once
#include "includes.h"

struct Config {
    bool invertHorizontal = false;
    bool invertVertical = false;
    bool invertHorizontalFreeFly = false;
    bool invertVerticalFreeFly = false;
};

class ConfigManager {
public:
    ConfigManager(const std::string& path);

    bool load();

    const Config& getConfig() const { return config; }

    bool WriteDefaultConfig();

private:
    std::string configPath;
    Config config;
};