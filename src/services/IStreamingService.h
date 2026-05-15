#pragma once

#include <drogon/HttpResponse.h>
#include <string>

class IStreamingService
{
public:
    virtual ~IStreamingService() = default;

    virtual drogon::HttpResponsePtr serveById(const std::string &id) const = 0;
};
