#pragma once

#include <string_view>

enum class MediaKind
{
    Video,
    Audio,
    Image
};

inline std::string_view mediaKindSlug(MediaKind kind) noexcept
{
    switch (kind)
    {
    case MediaKind::Video:
        return "videos";
    case MediaKind::Audio:
        return "music";
    case MediaKind::Image:
        return "images";
    }
    return "media";
}

inline std::string_view mediaKindTitle(MediaKind kind) noexcept
{
    switch (kind)
    {
    case MediaKind::Video:
        return "Videos";
    case MediaKind::Audio:
        return "Music";
    case MediaKind::Image:
        return "Images";
    }
    return "Media";
}

inline std::string_view mediaKindEmptyPrompt(MediaKind kind) noexcept
{
    switch (kind)
    {
    case MediaKind::Video:
        return "Select a video from the list.";
    case MediaKind::Audio:
        return "Select an audio track from the list.";
    case MediaKind::Image:
        return "Select an image from the list.";
    }
    return "Select an item from the list.";
}

inline std::string_view mediaKindDirectory(MediaKind kind) noexcept
{
    switch (kind)
    {
    case MediaKind::Video:
        return "videos";
    case MediaKind::Audio:
        return "music";
    case MediaKind::Image:
        return "images";
    }
    return "";
}
