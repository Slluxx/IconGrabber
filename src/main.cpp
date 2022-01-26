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
    nsInitialize();
    socketInitializeDefault();

    Config config;

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    i18n::loadTranslations();
    if (!brls::Application::init("main/name"_i18n))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    // Create a sample view
    brls::TabFrame *rootFrame = new brls::TabFrame();
    rootFrame->setTitle("main/name"_i18n);
    rootFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));

    brls::List *testList = new brls::List();

    brls::ListItem *dialogItem = new brls::ListItem("main/pozznx/open"_i18n);
    dialogItem->getClickEvent()->subscribe([](brls::View *view)
        {
            brls::Application::notify("clicked");
        });

    testList->addView(dialogItem);
    rootFrame->addTab("tab1", testList);

    brls::Application::pushView(rootFrame);

    while (brls::Application::mainLoop())
        ;

    // Exit
    socketExit();
    nsExit();
    return EXIT_SUCCESS;
}
