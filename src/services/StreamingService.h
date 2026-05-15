#pragma once
#include <drogon/HttpResponse.h>
#include <memory>
#include <string>

#include "IStreamingService.h"

class IMediaCatalog;

class StreamingService : public IStreamingService
{
public:
    explicit StreamingService(std::shared_ptr<const IMediaCatalog> library);

    drogon::HttpResponsePtr serveById(const std::string &id) const override;

private:
    std::shared_ptr<const IMediaCatalog> library_;
};
