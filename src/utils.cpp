#include "utils.hpp"

#include <switch.h>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace utils {

    // does file exist
    bool file_exists(std::string path)
    {
        std::filesystem::path p(path.c_str());
        return std::filesystem::exists(path);
    }

    //does directory exist
    bool directory_exists(std::string path)
    {
        std::filesystem::path p(path.c_str());
        return std::filesystem::exists(p);
    }

    std::string formatApplicationId(u64 ApplicationId)
    {
        std::stringstream strm;
        strm << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << ApplicationId;
        return strm.str();
    }

    size_t write_to_string(void *ptr, size_t size, size_t nmemb, std::string stream)
    {
        size_t realsize = size * nmemb;
        std::string temp(static_cast<const char*>(ptr), realsize);
        stream.append(temp);
        return realsize;
    }
    size_t write_to_file(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
    }
}