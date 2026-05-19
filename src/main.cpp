#include <drogon/drogon.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include "services/MediaLibrary.h"
#include "services/StreamingService.h"
#include "controllers/ApiMediaController.h"
#include "controllers/FrontendController.h"
#include "controllers/MediaController.h"

namespace fs = std::filesystem;

static fs::path readMediaRootFromConfig()
{
    if (const char *mediaRoot = std::getenv("MEDIA_ROOT"))
    {
        fs::path p(mediaRoot);
        p = fs::weakly_canonical(p);
        std::cout << "Media root: " << p << std::endl;
        return p;
    }

    const Json::Value &cfg = drogon::app().getCustomConfig();
    fs::path p(cfg["media"]["root"].asString());
    p = fs::weakly_canonical(p);
    std::cout << "Media root: " << p << std::endl;
    return p;
}

static uint16_t readPort()
{
    if (const char *port = std::getenv("PORT"))
    {
        return static_cast<uint16_t>(std::stoi(port));
    }
    return 8080;
}

int main()
{
    std::cout << "START\n";
    drogon::app().loadConfigFile("config.json");

    auto mediaRoot = readMediaRootFromConfig();
    std::cout << "Scanning media library..." << std::endl;

    auto lib = std::make_shared<MediaLibrary>();
    lib->loadFromRoot(mediaRoot);

    std::shared_ptr<const IMediaCatalog> catalog = lib;
    auto streaming = std::make_shared<StreamingService>(catalog);

    ApiMediaController::init(catalog);
    MediaController::init(catalog, streaming);

    drogon::app().addListener("0.0.0.0", readPort()).run();
}
