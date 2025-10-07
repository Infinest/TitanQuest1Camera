#include "configManager.h"


ConfigManager::ConfigManager(const std::string& path): configPath(path)
{
}

bool ConfigManager::WriteDefaultConfig() {
    std::ofstream file(configPath);
    if (!file.is_open()) {
        MessageBoxA(nullptr, ("Titan Quest 1 Camera Mod: Could not created config file in " + configPath).c_str(),
            "Write error", MB_ICONERROR);
        return false;
    }

    file << "invertHorizontal=" << (config.invertHorizontal ? 1 : 0) << "\n"
    << "invertVertical=" << (config.invertVertical ? 1 : 0) << "\n"
    << "invertHorizontalFreeFly=" << (config.invertHorizontalFreeFly ? 1 : 0) << "\n"
    << "invertVerticalFreeFly=" << (config.invertVerticalFreeFly ? 1 : 0) << "\n";
    file.close();

    return true;
}

bool ConfigManager::load() {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return WriteDefaultConfig();
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // Trim whitespaces
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);


            if (key == "invertHorizontal") config.invertHorizontal = (value == "true" || value == "1");
            else if (key == "invertVertical") config.invertVertical = (value == "true" || value == "1");
            else if (key == "invertHorizontalFreeFly") config.invertHorizontalFreeFly = (value == "true" || value == "1");
            else if (key == "invertVerticalFreeFly") config.invertVerticalFreeFly = (value == "true" || value == "1");
        }
    }
    file.close();

    return true;
}