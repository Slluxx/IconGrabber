#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <curl/curl.h>
#include <stdint.h>

#include <filesystem>
#include <iostream>
#include <map>
#include <string_view>
#include <fstream>
#include <nlohmann/json.hpp>
#include <borealis.hpp>

#include <nanovg/stb_image.h>

#include "config.hpp"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

namespace i18n = brls::i18n;    // for loadTranslations() and getStr()
using namespace i18n::literals; // for _i18n

int main(int argc, char *argv[])
{

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    i18n::loadTranslations();
    if (!brls::Application::init("main/name"_i18n))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    nsInitialize();
    socketInitializeDefault();

    Config config;
    nlohmann::json configJson = config.getConfig();

    // Create a sample view
    brls::TabFrame *rootFrame = new brls::TabFrame();
    rootFrame->setTitle("main/name"_i18n);
    rootFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));


    brls::List* settingsTab = new brls::List();

    brls::InputListItem* settingApiToken = new brls::InputListItem("Set the API token", configJson["api_token"].get<std::string>(), "Enter your steamgriddb.com api key", "Get it on steamgriddb.com", 32);
    settingApiToken->getClickEvent()->subscribe([&settingApiToken, &configJson](brls::View* view) {
        configJson["api_token"] = settingApiToken->getValue();
    });

    brls::Header* saveHeader = new brls::Header("Save settings");
    brls::ListItem* savebutton = new brls::ListItem("Save settings");
    savebutton->getClickEvent()->subscribe([&savebutton, &config, &configJson](brls::View* view) {
        config.setConfig(configJson);
        config.saveConfigToFile();
    });

    settingsTab->addView(settingApiToken);
    settingsTab->addView(saveHeader);
    settingsTab->addView(savebutton);



    brls::List *getImagesTab = new brls::List();

    brls::ListItem* online = new brls::ListItem("SteamGridDB.com", "You need an API key to use this feature");
    brls::ListItem* custom = new brls::ListItem("Custom source(s)", "Set your own sources in the config file");
    brls::ListItem* local = new brls::ListItem("Local folder", "Your own icons.");

    getImagesTab->addView(online);
    getImagesTab->addView(custom);
    getImagesTab->addView(local);


    brls::List *browseImagesTab = new brls::List();
    brls::ListItem* test = new brls::ListItem("test");


    rootFrame->addTab("Settings", settingsTab);
    rootFrame->addTab("Get Images", getImagesTab);

    brls::Application::pushView(rootFrame);

    while (brls::Application::mainLoop())
        ;

    // Exit
    socketExit();
    nsExit();
    return EXIT_SUCCESS;
}
