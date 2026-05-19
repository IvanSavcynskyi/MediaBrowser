#pragma once
#include <drogon/HttpResponse.h>
#include <memory>
#include <string>

#include "IStreamingService.h"

class IMediaCatalog;
class SupabaseStorageScanner;

class StreamingService : public IStreamingService
{
public:
    StreamingService(std::shared_ptr<const IMediaCatalog> library,
                     std::shared_ptr<const SupabaseStorageScanner> storage);

    drogon::HttpResponsePtr serveById(const std::string &id) const override;

private:
    std::shared_ptr<const IMediaCatalog> library_;
    std::shared_ptr<const SupabaseStorageScanner> storage_;
};
