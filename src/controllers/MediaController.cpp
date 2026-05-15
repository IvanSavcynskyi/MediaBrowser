#include "MediaController.h"

#include "../services/IStreamingService.h"

using namespace std;

shared_ptr<IStreamingService> MediaController::streaming_{};

void MediaController::init(shared_ptr<const IMediaCatalog>,
                           shared_ptr<IStreamingService> streaming)
{
    streaming_ = move(streaming);
}

void MediaController::redirectToFrontend(function<void(const drogon::HttpResponsePtr &)> cb,
                                         MediaKind kind)
{
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k302Found);
    string kindValue = "all";
    switch (kind)
    {
    case MediaKind::Video:
        kindValue = "video";
        break;
    case MediaKind::Audio:
        kindValue = "audio";
        break;
    case MediaKind::Image:
        kindValue = "image";
        break;
    }
    resp->addHeader("Location", "/?kind=" + kindValue);
    cb(resp);
}

void MediaController::videos(const drogon::HttpRequestPtr &req,
                             function<void(const drogon::HttpResponsePtr &)> &&cb)
{
    (void)req;
    redirectToFrontend(move(cb), MediaKind::Video);
}

void MediaController::music(const drogon::HttpRequestPtr &req,
                            function<void(const drogon::HttpResponsePtr &)> &&cb)
{
    (void)req;
    redirectToFrontend(move(cb), MediaKind::Audio);
}

void MediaController::images(const drogon::HttpRequestPtr &req,
                             function<void(const drogon::HttpResponsePtr &)> &&cb)
{
    (void)req;
    redirectToFrontend(move(cb), MediaKind::Image);
}

void MediaController::mediaById(const drogon::HttpRequestPtr &,
                                function<void(const drogon::HttpResponsePtr &)> &&cb,
                                string id)
{
    if (!streaming_)
    {
        cb(drogon::HttpResponse::newNotFoundResponse());
        return;
    }
    cb(streaming_->serveById(id));
}
