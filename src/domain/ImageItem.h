#pragma once
#include "MediaItem.h"
#include "Traits.h"

class ImageItem final : public MediaItem, public IPreviewable
{
public:
    ImageItem(std::filesystem::path path, std::string id, std::uintmax_t size,
              std::string mimeType,
              int width = 0, int height = 0)
        : MediaItem(std::move(path), std::move(id), size),
          mimeType_(std::move(mimeType)),
          width_(width),
          height_(height) {}

    MediaKind kind() const noexcept override { return MediaKind::Image; }
    std::string mime() const override { return mimeType_; }

    int width() const override { return width_; }
    int height() const override { return height_; }

private:
    std::string mimeType_;
    int width_;
    int height_;
};
