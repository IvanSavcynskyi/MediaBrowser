#pragma once
#include <filesystem>
#include <string>
#include <cstdint>

#include "MediaKind.h"
#include "../../utils/PathUtf8.h"

class MediaItem
{
public:
    virtual ~MediaItem() = default;

    const std::filesystem::path &path() const noexcept { return path_; }
    const std::string &id() const noexcept { return id_; }
    std::uintmax_t fileSize() const noexcept { return size_; }

    std::string title() const
    {
        return pathToUtf8(path_.stem());
    }

    virtual MediaKind kind() const noexcept = 0;
    virtual std::string mime() const = 0;

protected:
    MediaItem(std::filesystem::path path, std::string id, std::uintmax_t fileSize)
        : path_(std::move(path)), id_(std::move(id)), size_(fileSize) {}

private:
    std::filesystem::path path_;
    std::string id_;
    std::uintmax_t size_;
};
