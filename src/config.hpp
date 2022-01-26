#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <nlohmann/json.hpp>


class Config
{
private:
    std::string c_configPath = "sdmc:/config/icongrabber/config.json";

public:
    nlohmann::json c_config;

    Config();
    void initConfig();
    void loadConfigFromFile();
    void saveConfigToFile();
    void setConfigDefault();
    nlohmann::json getConfig();
    void setConfig(nlohmann::json config);

};
#endif