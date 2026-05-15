#pragma once
#include "MediaItem.h"
#include "Traits.h"

class VideoItem final : public MediaItem, public IPlayable
{
public:
    VideoItem(std::filesystem::path path, std::string id, std::uintmax_t size,
              std::string mimeType,
              int durationSec = 0)
        : MediaItem(std::move(path), std::move(id), size),
          mimeType_(std::move(mimeType)),
          durationSec_(durationSec) {}

    MediaKind kind() const noexcept override { return MediaKind::Video; }
    std::string mime() const override { return mimeType_; }

    int durationSec() const override { return static_cast<int>(durationSec_); }
    std::string playerTag() const override { return "video"; }

private:
    std::string mimeType_;
    int durationSec_;
};
