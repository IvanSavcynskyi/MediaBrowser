#include "ApiMediaController.h"

#include "../domain/MediaItem.h"
#include "../services/IMediaCatalog.h"

#include <algorithm>
#include <cctype>
#include <string>

using namespace std;

shared_ptr<const IMediaCatalog> ApiMediaController::lib_{};

namespace
{
string kindToApiValue(MediaKind kind)
{
    switch (kind)
    {
    case MediaKind::Video:
        return "video";
    case MediaKind::Audio:
        return "audio";
    case MediaKind::Image:
        return "image";
    }
    return "unknown";
}

string toLowerCopy(string value)
{
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch)
              { return static_cast<char>(tolower(ch)); });
    return value;
}
} // namespace

void ApiMediaController::init(shared_ptr<const IMediaCatalog> lib)
{
    lib_ = move(lib);
}

optional<MediaKind> ApiMediaController::parseKind(const string &value)
{
    if (value == "video")
        return MediaKind::Video;
    if (value == "audio")
        return MediaKind::Audio;
    if (value == "image")
        return MediaKind::Image;
    return nullopt;
}

Json::Value ApiMediaController::serializeItem(const MediaItem &item)
{
    Json::Value json(Json::objectValue);
    json["id"] = item.id();
    json["title"] = item.title();
    json["kind"] = kindToApiValue(item.kind());
    json["mime"] = item.mime();
    json["size"] = Json::UInt64(item.fileSize());
    json["url"] = "/media/" + item.id();
    return json;
}

drogon::HttpResponsePtr ApiMediaController::jsonResponse(const Json::Value &payload,
                                                         drogon::HttpStatusCode status)
{
    auto resp = drogon::HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(status);
    return resp;
}

void ApiMediaController::health(const drogon::HttpRequestPtr &,
                                function<void(const drogon::HttpResponsePtr &)> &&cb)
{
    Json::Value json(Json::objectValue);
    json["status"] = "ok";
    json["service"] = "MediaBrowser API";
    cb(jsonResponse(json));
}

void ApiMediaController::listMedia(const drogon::HttpRequestPtr &req,
                                   function<void(const drogon::HttpResponsePtr &)> &&cb)
{
    if (!lib_)
    {
        Json::Value error(Json::objectValue);
        error["error"] = "Media catalog is not initialized";
        cb(jsonResponse(error, drogon::k500InternalServerError));
        return;
    }

    vector<const MediaItem *> items;
    const auto kindValue = req->getParameter("kind");
    if (!kindValue.empty())
    {
        const auto parsedKind = parseKind(kindValue);
        if (!parsedKind.has_value())
        {
            Json::Value error(Json::objectValue);
            error["error"] = "Unsupported media kind";
            cb(jsonResponse(error, drogon::k400BadRequest));
            return;
        }
        items = lib_->items(parsedKind.value());
    }
    else
    {
        constexpr MediaKind allKinds[] = {
            MediaKind::Video,
            MediaKind::Audio,
            MediaKind::Image,
        };

        for (const auto kind : allKinds)
        {
            const auto typedItems = lib_->items(kind);
            items.insert(items.end(), typedItems.begin(), typedItems.end());
        }
    }

    const auto query = toLowerCopy(req->getParameter("q"));
    vector<const MediaItem *> filteredItems;
    filteredItems.reserve(items.size());
    for (const auto *item : items)
    {
        if (query.empty() || toLowerCopy(item->title()).find(query) != string::npos)
        {
            filteredItems.push_back(item);
        }
    }

    Json::Value payload(Json::objectValue);
    payload["count"] = static_cast<Json::UInt64>(filteredItems.size());
    payload["items"] = Json::Value(Json::arrayValue);
    for (const auto *item : filteredItems)
    {
        payload["items"].append(serializeItem(*item));
    }
    cb(jsonResponse(payload));
}

void ApiMediaController::mediaById(const drogon::HttpRequestPtr &,
                                   function<void(const drogon::HttpResponsePtr &)> &&cb,
                                   string id)
{
    if (!lib_)
    {
        Json::Value error(Json::objectValue);
        error["error"] = "Media catalog is not initialized";
        cb(jsonResponse(error, drogon::k500InternalServerError));
        return;
    }

    const auto *item = lib_->findById(id);
    if (!item)
    {
        Json::Value error(Json::objectValue);
        error["error"] = "Media item not found";
        cb(jsonResponse(error, drogon::k404NotFound));
        return;
    }

    cb(jsonResponse(serializeItem(*item)));
}
