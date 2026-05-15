#include "StreamingService.h"
#include "IMediaCatalog.h"

#include <drogon/HttpResponse.h>
#include <filesystem>

#include "../../utils/PathUtf8.h"

namespace fs = std::filesystem;

StreamingService::StreamingService(std::shared_ptr<const IMediaCatalog> library)
    : library_(library) {}

drogon::HttpResponsePtr StreamingService::serveById(const std::string &id) const
{
    if (!library_)
    {
        return drogon::HttpResponse::newNotFoundResponse();
    }

    const auto *item = library_->findById(id);
    if (!item)
    {
        return drogon::HttpResponse::newNotFoundResponse();
    }

    const fs::path p = item->path();
    if (!fs::exists(p) || !fs::is_regular_file(p))
    {
        return drogon::HttpResponse::newNotFoundResponse();
    }

    auto resp = drogon::HttpResponse::newFileResponse(pathToUtf8(p));
    resp->setContentTypeString(item->mime());

    return resp;
}
