#include "cwindow.hpp"
#include "utils.hpp"
#include "config.hpp"

#include <switch.h>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <borealis.hpp>

namespace cwindow {

    void frame_mainWindow(){

        Config config;
        nlohmann::json configJson = config.getConfig();

        brls::TabFrame* rootFrame = new brls::TabFrame();
        rootFrame->setTitle("IconGrabber: Main");
        rootFrame->setIcon(BOREALIS_ASSET("icon/icon.jpg"));

        brls::List* settingsTab = new brls::List();

        brls::Header* sysHeader = new brls::Header("sys-tweak");

        int sysTweakStatus = 0;
        std::string sysTweakStatusStr = "Sys-tweak is not installed.";
        std::string sysTweakActionStr = "Set up Sys-tweak";

        if (utils::file_exists("sdmc:/atmosphere/contents/00FF747765616BFF/exefs.nsp")) {
            if (!utils::file_exists("sdmc:/atmosphere/contents/00FF747765616BFF/flags/boot2.flag")) {
                sysTweakStatusStr = "Sys-tweak is installed but not active.";
                sysTweakActionStr = "Activate Sys-tweak";
                sysTweakStatus = 2;
            } else {
                sysTweakStatusStr = "Sys-tweak is installed and active.";
                sysTweakActionStr = "Update Sys-tweak";
                sysTweakStatus = 1;
            }
        }
        brls::ListItem* systweakButton = new brls::ListItem(sysTweakStatusStr);

        systweakButton->registerAction(sysTweakActionStr, brls::Key::X, [sysTweakStatus] {
            if (sysTweakStatus == 0) {
                // set up
                brls::Application::notify("setup sys-tweak");
                cwindow::frame_showLocalTitles();
            } else if (sysTweakStatus == 1) {
                // update
                brls::Application::notify("update sys-tweak");
                cwindow::frame_showLocalTitles();
            } else if (sysTweakStatus == 2) {
                // activate
                brls::Application::notify("activate sys-tweak");
                cwindow::frame_showLocalTitles();
            }
            return true;
        });

        brls::Header* settingsHeader = new brls::Header("Settings");
        brls::InputListItem* settingApiToken = new brls::InputListItem("SteamGridDB.com API token", configJson["api_token"].get<std::string>(), "Enter your steamgriddb.com api key", "", 32);
        settingApiToken->getClickEvent()->subscribe([&settingApiToken, &configJson](brls::View* view) {
            configJson["api_token"] = settingApiToken->getValue();
        });

        brls::ListItem* savebutton = new brls::ListItem("Save settings");
        savebutton->getClickEvent()->subscribe([&savebutton, &config, &configJson](brls::View* view) {
            config.setConfig(configJson);
            config.saveConfigToFile();
        });

        settingsTab->addView(sysHeader);
        settingsTab->addView(systweakButton);
        settingsTab->addView(settingsHeader);
        settingsTab->addView(settingApiToken);
        settingsTab->addView(savebutton);

        brls::List* getImagesTab = new brls::List();

        brls::ListItem* online = new brls::ListItem("SteamGridDB.com", "You need an API key to use this feature");
        brls::ListItem* custom = new brls::ListItem("Custom source(s)", "Set your own sources in the config file");
        brls::ListItem* local = new brls::ListItem("Local folder", "Your own icons.");

        getImagesTab->addView(online);
        getImagesTab->addView(custom);
        getImagesTab->addView(local);

        brls::List* browseImagesTab = new brls::List();
        brls::ListItem* test = new brls::ListItem("test");
        browseImagesTab->addView(test);

        rootFrame->addTab("Settings", settingsTab);
        rootFrame->addTab("Get Images", getImagesTab);

        brls::Application::pushView(rootFrame);
    }



    void frame_showLocalTitles(){

        brls::AppletFrame* gamelistFrame = new brls::AppletFrame(true, true);
        gamelistFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));

        brls::List* titleList = new brls::List();
        brls::ListItem* litem = new brls::ListItem("item");

        titleList->addView(litem);
        gamelistFrame->setContentView(titleList);
        brls::Application::pushView(gamelistFrame);
    }
}