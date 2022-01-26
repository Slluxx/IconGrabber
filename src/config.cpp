#ifndef UTILS_CPP
#define UTILS_CPP

#include "config.hpp"
#include "utils.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

// Date constructor
Config::Config()
{
    initConfig();
}

void Config::initConfig()
{
    if (utils::file_exists(c_configPath))
    {
        loadConfigFromFile();
    }
    else
    {
        setConfigDefault();
        saveConfigToFile();
    }
}
void Config::loadConfigFromFile()
{
    std::ifstream ifs(c_configPath.c_str());
    ifs >> c_config;
}
void Config::saveConfigToFile()
{
    std::ofstream o(c_configPath.c_str());
    o << std::setw(4) << c_config << std::endl;
}
void Config::setConfigDefault()
{
    c_config["api_token"] = "abc";
    c_config["customSources"] = nlohmann::json::array();
}
nlohmann::json Config::getConfig()
{
    return c_config;
}
void Config::setConfig(nlohmann::json config)
{
    c_config = config;
}
#endif