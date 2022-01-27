#include <curl/curl.h>
#include <nanovg/stb_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include <borealis.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string_view>

#include "config.hpp"
#include "cwindow.hpp"
#include "utils.hpp"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

namespace i18n = brls::i18n;     // for loadTranslations() and getStr()
using namespace i18n::literals;  // for _i18n

int main(int argc, char* argv[]) {
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    i18n::loadTranslations();
    if (!brls::Application::init("main/name"_i18n)) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    nsInitialize();
    socketInitializeDefault();

    nlohmann::json installedGames = utils::getInstalledGames();

    cwindow::frame_mainWindow();

    while (brls::Application::mainLoop())
        ;

    // Exit
    socketExit();
    nsExit();
    return EXIT_SUCCESS;
}
