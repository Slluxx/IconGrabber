#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <switch.h>
#include <nlohmann/json.hpp>

namespace utils {

    //definition inside utils.cpp
    //extern std::unordered_map<std::string, short> rgb2short;

    bool file_exists(std::string path);
    bool directory_exists(std::string path);
    std::string formatApplicationId(u64 ApplicationId);
    size_t write_to_string(void *ptr, size_t size, size_t nmemb, std::string stream);
    size_t write_to_file(void *ptr, size_t size, size_t nmemb, FILE *stream);
    nlohmann::json getInstalledGames();

}

#endif