#include "StreamingService.h"
#include "IMediaCatalog.h"
#include "SupabaseStorageScanner.h"

#include <drogon/HttpResponse.h>
#include <utility>

#include "../../utils/PathUtf8.h"

StreamingService::StreamingService(std::shared_ptr<const IMediaCatalog> library,
                                   std::shared_ptr<const SupabaseStorageScanner> storage)
    : library_(std::move(library)), storage_(std::move(storage)) {}

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

    if (!storage_)
    {
        return drogon::HttpResponse::newNotFoundResponse();
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k302Found);
    resp->addHeader("Location", storage_->presignedGetUrl(pathToUtf8(item->path())));

    return resp;
}
