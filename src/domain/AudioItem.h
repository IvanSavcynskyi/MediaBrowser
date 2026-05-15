#pragma once
#include "MediaItem.h"
#include "Traits.h"

class AudioItem final : public MediaItem, public IPlayable
{
public:
    AudioItem(std::filesystem::path path, std::string id, std::uintmax_t size,
              std::string mimeType,
              int durationSec = 0)
        : MediaItem(std::move(path), std::move(id), size),
          mimeType_(std::move(mimeType)),
          durationSec_(durationSec) {}

    MediaKind kind() const noexcept override { return MediaKind::Audio; }
    std::string mime() const override { return mimeType_; }

    int durationSec() const override { return static_cast<int>(durationSec_); }
    std::string playerTag() const override { return "audio"; }

private:
    std::string mimeType_;
    int durationSec_;
};
