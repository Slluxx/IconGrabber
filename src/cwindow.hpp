#ifndef CWINDOW_HPP
#define CWINDOW_HPP

#include <string>
#include <switch.h>
#include <nlohmann/json.hpp>
#include "config.hpp"

namespace cwindow {

    extern nlohmann::json configJson;
    extern Config config;

    void frame_mainWindow();
    void frame_showLocalTitles();

}

#endif