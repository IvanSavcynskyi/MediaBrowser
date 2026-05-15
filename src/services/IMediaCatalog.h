#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../domain/MediaItem.h"
#include "../domain/MediaKind.h"

class IMediaCatalog
{
public:
    virtual ~IMediaCatalog() = default;

    virtual std::vector<const MediaItem *> items(MediaKind kind) const = 0;
    virtual const MediaItem *findById(const std::string &id) const = 0;
    virtual std::size_t size() const noexcept = 0;
};
