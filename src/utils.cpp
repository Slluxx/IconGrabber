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

    nlohmann::json getInstalledGames(){
        NsApplicationRecord *records = new NsApplicationRecord[64000]();
        uint64_t tid;
        NsApplicationControlData controlData;
        NacpLanguageEntry* langEntry = NULL;

        Result rc;
        int recordCount     = 0;
        size_t controlSize  = 0;

        nlohmann::json games = nlohmann::json::array();

        rc = nsListApplicationRecord(records, 64000, 0, &recordCount);
        for (s32 i = 0; i < recordCount; i++)
        {
            tid = records[i].application_id;
            rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, tid, &controlData, sizeof(controlData), &controlSize);
            if(R_FAILED(rc)) break;
            rc = nacpGetLanguageEntry(&controlData.nacp, &langEntry);
            if(R_FAILED(rc)) break;
            
            if(!langEntry->name)
                continue;

            std::string appName = langEntry->name;
            std::string titleId = formatApplicationId(tid);
            games.push_back({ {"tid", titleId}, {"name", appName} });
        }
        delete[] records;
        return games;
    }

}