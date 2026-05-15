#include "FrontendController.h"

#include <filesystem>

#include "../../utils/PathUtf8.h"

namespace fs = std::filesystem;

namespace
{
fs::path distRoot()
{
    return fs::current_path() / "frontend" / "dist";
}

drogon::HttpResponsePtr notFoundResponse()
{
    auto resp = drogon::HttpResponse::newNotFoundResponse();
    resp->setBody("Frontend build was not found. Run `npm run build` in frontend/.");
    return resp;
}

drogon::HttpResponsePtr fileResponse(const fs::path &path)
{
    if (!fs::exists(path) || !fs::is_regular_file(path))
    {
        return notFoundResponse();
    }
    return drogon::HttpResponse::newFileResponse(pathToUtf8(path));
}
} // namespace

void FrontendController::index(const drogon::HttpRequestPtr &,
                               std::function<void(const drogon::HttpResponsePtr &)> &&cb)
{
    cb(fileResponse(distRoot() / "index.html"));
}

void FrontendController::asset(const drogon::HttpRequestPtr &,
                               std::function<void(const drogon::HttpResponsePtr &)> &&cb,
                               std::string path)
{
    cb(fileResponse(distRoot() / "assets" / path));
}
