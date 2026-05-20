#include <drogon/drogon.h>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include "services/MediaLibrary.h"
#include "services/StreamingService.h"
#include "services/SupabaseStorageScanner.h"
#include "controllers/ApiMediaController.h"
#include "controllers/FrontendController.h"
#include "controllers/MediaController.h"

namespace fs = std::filesystem;

static void setEnvIfMissing(const std::string &name, const std::string &value)
{
    if (name.empty() || std::getenv(name.c_str()))
        return;
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 0);
#endif
}

static std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

static void loadDotEnvFile(const fs::path &path)
{
    std::ifstream in(path);
    if (!in)
        return;

    std::string line;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line.front() == '#')
            continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;

        auto name = trim(line.substr(0, eq));
        auto value = trim(line.substr(eq + 1));
        if (value.size() >= 2 &&
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\'')))
        {
            value = value.substr(1, value.size() - 2);
        }
        setEnvIfMissing(name, value);
    }
}

static void loadDotEnv()
{
    loadDotEnvFile(fs::current_path() / ".env");
    loadDotEnvFile(fs::current_path().parent_path() / ".env");
}

static std::string readEnv(const char *name, std::string fallback = {})
{
    if (const char *value = std::getenv(name))
    {
        if (*value != '\0')
            return value;
    }
    return fallback;
}

static std::string requireEnv(const char *name)
{
    auto value = readEnv(name);
    if (value.empty())
        throw std::runtime_error(std::string(name) + " is not set");
    return value;
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
    try
    {
        std::cout << "START\n";
        loadDotEnv();
        drogon::app().loadConfigFile("config.json");

        auto storage = std::make_shared<SupabaseStorageScanner>(
            readEnv("SUPABASE_S3_ENDPOINT", "https://cdumjkcrnuqaacgfudqd.storage.supabase.co/storage/v1/s3"),
            readEnv("SUPABASE_REGION", "eu-west-1"),
            requireEnv("SUPABASE_ACCESS_ID"),
            requireEnv("SUPABASE_ACCESS_KEY"),
            readEnv("SUPABASE_BUCKET", "MediaBrowser"));

        auto lib = std::make_shared<MediaLibrary>();
        std::cout << "Scanning Supabase Storage bucket..." << std::endl;
        lib->loadFromSupabase(requireEnv("DATABASE_URL"), *storage);

        std::shared_ptr<const IMediaCatalog> catalog = lib;
        auto streaming = std::make_shared<StreamingService>(catalog, storage);

        ApiMediaController::init(catalog);
        MediaController::init(catalog, streaming);

        drogon::app().addListener("0.0.0.0", readPort()).run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Startup failed: " << ex.what() << std::endl;
        return 1;
    }
}
