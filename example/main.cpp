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

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

namespace i18n = brls::i18n; // for loadTranslations() and getStr()
using namespace i18n::literals; // for _i18n

std::string configPath = "sdmc:/config/icongrabber/config.json";

std::vector<std::string> allowedStyles = {
    "all styles",
    "alternate",
    "blurred",
    "white_logo",
    "material",
    "no_logo"
};

std::vector<std::string> allowedImageResolutions = {
    "460x215",
    "920x430",
    "600x900",
    "342x482",
    "660x930",
    "512x512",
    "1024x1024"
};

nlohmann::json loadConfig()
{
    if (std::filesystem::exists(configPath.c_str()))
    {
        brls::Logger::info("Loading config from file");
        std::ifstream i(configPath.c_str());
        nlohmann::json j;
        i >> j;
        return j;
    }
    else
    {
        brls::Logger::info("using default config");
        nlohmann::json config;
        config["api_token"]     = "";
        config["style_id"]      = 0;
        config["resolution_id"] = 5;
        return config;
    }
}

void saveConfig(nlohmann::json config)
{
    if (!std::filesystem::exists("sdmc:/config/icongrabber/"))
        if (!std::filesystem::create_directories("sdmc:/config/icongrabber/"))
            brls::Logger::info("Could not create directory");

    std::ofstream o(configPath.c_str());
    o << std::setw(4) << config << std::endl;
    brls::Logger::info("saved config");
    brls::Application::notify("Saved");
}

std::string formatApplicationId(u64 ApplicationId)
{
    std::stringstream strm;
    strm << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << ApplicationId;
    return strm.str();
}

void overwriteIcon(std::string tid, std::string imagePath)
{
    int width, height, channels;
    unsigned char* img = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
    if (img == NULL)
    {
        brls::Logger::info("Image could not be loaded");
    }
    else
    {
        brls::Logger::info("Image is loaded");
        unsigned char* data = NULL;
        data                = (unsigned char*)malloc(256 * 256 * channels * sizeof(unsigned char));

        if (stbir_resize_uint8(img, width, height, 0, data, 256, 256, 256 * channels, channels))
        {
            brls::Logger::info("resize good");
        }
        else
        {
            brls::Logger::info("resize bad");
        }

        std::string outPath = "sdmc:/atmosphere/contents/";
        outPath             = outPath.append(tid);
        if (!std::filesystem::create_directories(outPath))
            brls::Logger::info("Could not create directory");
        outPath = outPath.append("/icon.jpg");

        brls::Logger::info(outPath);

        if (stbi_write_jpg(outPath.c_str(), 256, 256, channels, data, 100))
        {
            brls::Application::notify("Icon saved");
        }
        else
        {
            brls::Application::notify("Icon could not be saved");
        };
        stbi_image_free(img);
        free(data);
    }
}

nlohmann::json getInstalledGames()
{
    NsApplicationRecord* records = new NsApplicationRecord[64000]();
    uint64_t tid;
    NsApplicationControlData controlData;
    NacpLanguageEntry* langEntry = NULL;

    Result rc;
    int recordCount    = 0;
    size_t controlSize = 0;

    nlohmann::json games = nlohmann::json::array();

    rc = nsListApplicationRecord(records, 64000, 0, &recordCount);
    for (s32 i = 0; i < recordCount; i++)
    {
        tid = records[i].application_id;
        rc  = nsGetApplicationControlData(NsApplicationControlSource_Storage, tid, &controlData, sizeof(controlData), &controlSize);
        if (R_FAILED(rc))
            break;
        rc = nacpGetLanguageEntry(&controlData.nacp, &langEntry);
        if (R_FAILED(rc))
            break;

        if (!langEntry->name)
            continue;

        std::string appName = langEntry->name;
        std::string titleId = formatApplicationId(tid);
        games.push_back({ { "tid", titleId }, { "name", appName } });
    }
    delete[] records;
    return games;
}

size_t write_to_string(void* ptr, size_t size, size_t nmemb, std::string stream)
{
    size_t realsize = size * nmemb;
    std::string temp(static_cast<const char*>(ptr), realsize);
    stream.append(temp);
    return realsize;
}
size_t write_to_file(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
std::string base_name(std::string const& path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}
std::string downloadFile(nlohmann::json game, bool thumbnail)
{
    std::string url = game["thumb"].get<std::string>();

    std::string outpath = "sdmc:/gameIcons/";
    if (thumbnail)
    {
        outpath = outpath.append("thumbnails/");
    }
    else
    {
        outpath = outpath.append("full/");
    }

    if (!std::filesystem::exists(outpath))
        if (!std::filesystem::create_directories(outpath))
            brls::Logger::info("Could not create directory");

    std::string filename = "";
    filename.append(game["width"].dump());
    filename.append("x");
    filename.append(game["height"].dump());
    filename.append("_");
    filename.append(base_name(url));
    outpath = outpath.append(filename);

    if (!std::filesystem::exists(outpath))
    {
        CURL* curl;
        FILE* fp;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            fp = fopen(outpath.c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
        }
        else
        {
            outpath = "";
        }
    }

    if (!thumbnail)
    {
        if (thumbnail == false && outpath != "")
        {
            brls::Application::notify("Image Downloaded.\nApply to a title in main menu.");
        }
        else
        {
            brls::Application::notify("Image Download Failed.");
        }
    }
    return outpath;
}
nlohmann::json requestGames(std::string gameName)
{
    nlohmann::json config = loadConfig();
    std::string api_token = config["api_token"];

    std::string authString = "Authorization: Bearer ";
    authString.append(api_token);

    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {

        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, authString.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        char* output        = curl_easy_escape(curl, gameName.c_str(), gameName.length());
        std::string url     = "https://www.steamgriddb.com/api/v2/search/autocomplete/";
        std::string fullurl = url.append(output);
        curl_easy_setopt(curl, CURLOPT_URL, fullurl.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libnx curl example/1.0");

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            nlohmann::json j;
            j["success"] = false;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return j;
        }
        else
        {
            nlohmann::json j = nlohmann::json::parse(response);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return j;
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    nlohmann::json j;
    j["success"] = false;
    return j;
}

nlohmann::json requestIcons(std::string gameId)
{
    nlohmann::json config = loadConfig();
    std::string api_token = config["api_token"];

    std::string authString = "Authorization: Bearer ";
    authString.append(api_token);

    std::string postFields = "";
    postFields.append("?styles=");
    if (config["style_id"] == 0)
    {
        postFields.append("alternate,blurred,white_logo,material,no_logo");
    }
    else
    {
        postFields.append(allowedStyles[config["style_id"]]);
    }

    postFields.append("&dimensions=");
    postFields.append(allowedImageResolutions[config["resolution_id"]]);
    postFields.append("&mimes=image/png,image/jpeg");

    brls::Logger::info(postFields);

    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {

        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, authString.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string url     = "https://www.steamgriddb.com/api/v2/grids/game/";
        std::string fullurl = url.append(gameId);
        fullurl.append(postFields);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libnx curl example/1.0");
        curl_easy_setopt(curl, CURLOPT_URL, fullurl.c_str());

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            nlohmann::json j;
            j["success"] = false;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return j;
        }
        else
        {
            nlohmann::json j = nlohmann::json::parse(response);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return j;
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    nlohmann::json j;
    j["success"] = false;
    return j;
}

void frame_showOnlineTitleIcons(std::string gameId)
{
    brls::Logger::info("frame_showOnlineTitleIcons");
    brls::ThumbnailFrame* rootFrame = new brls::ThumbnailFrame();
    rootFrame->setTitle("List of fetched icons from SteamGridDB");
    rootFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));
    rootFrame->getSidebar()->setThumbnail(BOREALIS_ASSET("icon/borealis.jpg"));
    rootFrame->getSidebar()->getButton()->setLabel("Download");
    nlohmann::json iconList = requestIcons(gameId);
    brls::List* list        = new brls::List();

    brls::Logger::info(iconList.dump());
    brls::Logger::info(iconList["success"].dump());
    brls::Logger::info(std::to_string(iconList["data"].size()));

    if (iconList["success"].get<bool>() && iconList["data"].size() != 0)
    {
        brls::Logger::info("iconlist success");
        for (auto it : iconList["data"])
        {
            if (it["lock"].get<bool>())
            {
                brls::Logger::info("skipped DMCA hidden image");
                continue;
            }
            brls::Logger::info(it["id"].dump());
            brls::ListItem* litem = new brls::ListItem(it["id"].dump());
            litem->getFocusEvent()->subscribe([=](brls::View* view)
                {
            rootFrame->getSidebar()->getButton()->getClickEvent()->unsubscribeAll();
            std::string path = downloadFile(it, true);
            if (path != ""){
                rootFrame->getSidebar()->setThumbnail(path);
                rootFrame->getSidebar()->setTitle(it["id"].dump());
                rootFrame->getSidebar()->getButton()->getClickEvent()->subscribe([=](brls::View* view) {
                    std::string path = downloadFile(it, false);
                });
            } else {
                rootFrame->getSidebar()->setThumbnail(BOREALIS_ASSET("icon/borealis.jpg"));
                rootFrame->getSidebar()->setTitle("Could not fetch thumbnail");
                rootFrame->getSidebar()->setSubtitle("");
            } });
            list->addView(litem);
        }
    }
    else
    {
        brls::Logger::info("error");
        brls::ListItem* litem = new brls::ListItem("No icon found", "Maybe select a different icon style or resolution.");
        list->addView(litem);
    }
    rootFrame->setContentView(list);
    brls::Application::pushView(rootFrame);
}

void frame_showOnlineTitles(std::string searchTerm)
{
    brls::AppletFrame* onlinegamelistFrame = new brls::AppletFrame(true, true);
    onlinegamelistFrame->setTitle("Found games on steamgriddb.com");
    onlinegamelistFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));
    brls::List* onlinetitleList = new brls::List();
    nlohmann::json foundGames   = requestGames(searchTerm);
    brls::Logger::info(foundGames.dump());
    brls::Logger::info(foundGames["success"].dump());
    brls::Logger::info(foundGames["data"].dump());
    if (foundGames["success"].get<bool>() && foundGames["data"].size() != 0)
    {
        for (auto it : foundGames["data"])
        {
            brls::ListItem* litem = new brls::ListItem(it["name"]);
            litem->getClickEvent()->subscribe([=](brls::View* view)
                { frame_showOnlineTitleIcons(it["id"].dump()); });
            onlinetitleList->addView(litem);
        }
    }
    else
    {
        brls::Logger::info("error");
        brls::ListItem* litem = new brls::ListItem("No game found");
        onlinetitleList->addView(litem);
    }

    onlinegamelistFrame->setContentView(onlinetitleList);
    brls::Application::pushView(onlinegamelistFrame);
}

void frame_showLocalTitles(std::string imagePath)
{
    nlohmann::json installedGames    = getInstalledGames();
    brls::AppletFrame* gamelistFrame = new brls::AppletFrame(true, true);
    if (imagePath != "")
    {
        gamelistFrame->setTitle("Choose a title to search for icons.");
    }
    else
    {
        gamelistFrame->setTitle("Choose a title to overwrite its icon.");
    }

    gamelistFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));
    brls::List* titleList = new brls::List();
    for (auto it : installedGames)
    {
        std::string name      = it["name"].get<std::string>();
        std::string tid       = it["tid"].get<std::string>();
        brls::ListItem* litem = new brls::ListItem(it["name"]);
        litem->getClickEvent()->subscribe([name, tid, imagePath](brls::View* view)
            {
                if (imagePath == "")
                {
                    frame_showOnlineTitles(name);
                }
                else
                {
                    overwriteIcon(tid, imagePath);
                }
            });
        if (imagePath == "")
        {
            litem->registerAction("Delete current icon", brls::Key::X, [=]
                { 
                std::string outPath = "sdmc:/atmosphere/contents/";
                outPath = outPath.append(tid);
                outPath = outPath.append("/icon.jpg");
                std::remove(outPath.c_str());
                brls::Application::notify("Icon deleted");
                return true; });
        }

        titleList->addView(litem);
    }
    gamelistFrame->setContentView(titleList);
    brls::Application::pushView(gamelistFrame);
}

void frame_showLocalIcons()
{
    brls::ThumbnailFrame* rootFrame = new brls::ThumbnailFrame();
    rootFrame->setTitle("A list of downloaded icons");
    rootFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));
    rootFrame->getSidebar()->setThumbnail(BOREALIS_ASSET("icon/borealis.jpg"));
    rootFrame->getSidebar()->getButton()->setLabel("Set as icon");

    brls::List* list = new brls::List();

    int count = 0;
    for (auto const& dir_entry : std::filesystem::directory_iterator("sdmc:/gameIcons/full/"))
    {
        count++;
        std::string filepath  = dir_entry.path().string();
        std::string filename  = dir_entry.path().filename().string();
        brls::ListItem* litem = new brls::ListItem(filename);
        litem->getFocusEvent()->subscribe([filename, filepath, rootFrame](brls::View* view)
            {
            rootFrame->getSidebar()->getButton()->getClickEvent()->unsubscribeAll();
            rootFrame->getSidebar()->setThumbnail(filepath);
            rootFrame->getSidebar()->getButton()->getClickEvent()->subscribe([filepath](brls::View* view) {
                frame_showLocalTitles(filepath);
            }); });

        list->addView(litem);
    }

    if (count == 0)
    {
        brls::ListItem* litem = new brls::ListItem("No files found.");
        list->addView(litem);
    }

    rootFrame->setContentView(list);
    brls::Application::pushView(rootFrame);
}

int main(int argc, char* argv[])
{
    nsInitialize();
    socketInitializeDefault();

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    i18n::loadTranslations();
    if (!brls::Application::init("main/name"_i18n))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    nlohmann::json config     = loadConfig();
    brls::TabFrame* rootFrame = new brls::TabFrame();
    rootFrame->setTitle("main/name"_i18n);
    rootFrame->setIcon(BOREALIS_ASSET("icon/borealis.jpg"));

    brls::List* settingsTab                 = new brls::List();
    brls::InputListItem* settingApiToken    = new brls::InputListItem("Set the API token", config["api_token"], "Enter your steamgriddb.com api key", "Get it on steamgriddb.com", 32);
    brls::SelectListItem* settingStyles     = new brls::SelectListItem("Choose icon style", allowedStyles, config["style_id"]);
    brls::SelectListItem* settingResolution = new brls::SelectListItem("Choose icon resolution", allowedImageResolutions, config["resolution_id"], "All icons will be resized. Default Switch icons are 1:1 (512x512 or 1024x1024) while vertical icon themes should use 2:3 (600x900).");
    settingApiToken->getClickEvent()->subscribe([&settingApiToken](brls::View* view)
        {
        nlohmann::json c = loadConfig();
        c["api_token"] = settingApiToken->getValue();
        saveConfig(c); });
    settingStyles->getValueSelectedEvent()->subscribe([](size_t selection)
        {
        nlohmann::json c = loadConfig();
        c["style_id"] = selection;
        saveConfig(c); });
    settingResolution->getValueSelectedEvent()->subscribe([](size_t selection)
        {
        nlohmann::json c = loadConfig();
        c["resolution_id"] = selection;
        saveConfig(c); });

    brls::List* searchTab             = new brls::List();
    brls::InputListItem* browseByName = new brls::InputListItem("Search for a game by name", "", "Enter the name of a game to search icons", "", 32);
    brls::ListItem* browseByTitle     = new brls::ListItem("Search by installed titles");
    browseByName->getClickEvent()->subscribe([&browseByName](brls::View* view)
        {
        std::string text = browseByName->getValue();
        if (text == "")
            text = " ";
        frame_showOnlineTitles(text); });
    browseByTitle->getClickEvent()->subscribe([](brls::View* view)
        { frame_showLocalTitles(""); });

    brls::List* localIcons          = new brls::List();
    brls::ListItem* browseIcons     = new brls::ListItem("Browse downloaded Icons");
    brls::ListItem* deleteIconCache = new brls::ListItem("Delete Imagecache", "This will not delete already applied icons.");

    browseIcons->getClickEvent()->subscribe([=](brls::View* view)
        { frame_showLocalIcons(); });
    deleteIconCache->getClickEvent()->subscribe([=](brls::View* view)
        {
        if (std::filesystem::remove_all("sdmc:/gameIcons/full/"))
            std::filesystem::create_directory("sdmc:/gameIcons/full/");
        if (std::filesystem::remove_all("sdmc:/gameIcons/thumbnails/"))
            std::filesystem::create_directory("sdmc:/gameIcons/thumbnails/");

        brls::Application::notify("Done"); });

    settingsTab->addView(settingApiToken);
    settingsTab->addView(settingStyles);
    settingsTab->addView(settingResolution);

    searchTab->addView(browseByName);
    searchTab->addView(browseByTitle);

    localIcons->addView(browseIcons);
    localIcons->addView(deleteIconCache);

    rootFrame->addTab("Settings", settingsTab);
    rootFrame->addSeparator();
    rootFrame->addTab("Search Icons", searchTab);
    rootFrame->addTab("Downloaded Icons", localIcons);

    brls::Application::pushView(rootFrame);

    while (brls::Application::mainLoop())
        ;

    // Exit
    socketExit();
    nsExit();
    return EXIT_SUCCESS;
}
