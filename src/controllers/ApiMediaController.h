#pragma once

#include <drogon/HttpController.h>
#include <memory>
#include <optional>

#include "../domain/MediaKind.h"

class IMediaCatalog;
class MediaItem;

class ApiMediaController : public drogon::HttpController<ApiMediaController>
{
public:
    static void init(std::shared_ptr<const IMediaCatalog> lib);

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ApiMediaController::health, "/api/health", drogon::Get);
    ADD_METHOD_TO(ApiMediaController::listMedia, "/api/media", drogon::Get);
    ADD_METHOD_TO(ApiMediaController::mediaById, "/api/media/{1}", drogon::Get);
    METHOD_LIST_END

    void health(const drogon::HttpRequestPtr &req,
                std::function<void(const drogon::HttpResponsePtr &)> &&cb);

    void listMedia(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&cb);

    void mediaById(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&cb,
                   std::string id);

private:
    static std::shared_ptr<const IMediaCatalog> lib_;

    static std::optional<MediaKind> parseKind(const std::string &value);
    static Json::Value serializeItem(const class MediaItem &item);
    static drogon::HttpResponsePtr jsonResponse(const Json::Value &payload,
                                                drogon::HttpStatusCode status = drogon::k200OK);
};
