#pragma once
#include <drogon/HttpController.h>
#include <memory>

#include "../domain/MediaKind.h"

class IMediaCatalog;
class IStreamingService;

class MediaController : public drogon::HttpController<MediaController>
{
public:
    static void init(std::shared_ptr<const IMediaCatalog> lib,
                     std::shared_ptr<IStreamingService> streaming);

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MediaController::videos, "/videos", drogon::Get);
    ADD_METHOD_TO(MediaController::music, "/music", drogon::Get);
    ADD_METHOD_TO(MediaController::images, "/images", drogon::Get);
    ADD_METHOD_TO(MediaController::mediaById, "/media/{1}", drogon::Get);
    METHOD_LIST_END

    void videos(const drogon::HttpRequestPtr &req,
                std::function<void(const drogon::HttpResponsePtr &)> &&cb);

    void music(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&cb);

    void images(const drogon::HttpRequestPtr &req,
                std::function<void(const drogon::HttpResponsePtr &)> &&cb);

    void mediaById(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&cb,
                   std::string id);

private:
    static std::shared_ptr<const IMediaCatalog> lib_;
    static std::shared_ptr<IStreamingService> streaming_;
    static void redirectToFrontend(std::function<void(const drogon::HttpResponsePtr &)> cb,
                                   MediaKind kind);
};
