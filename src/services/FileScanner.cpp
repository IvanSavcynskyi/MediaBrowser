#include "FileScanner.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

static std::string toLowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c)
                   { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string FileScanner::fileExtension(const fs::path &p)
{
    std::string e = p.extension().string();
    if (e.empty())
        return {};
    if (!e.empty() && e[0] == '.')
        e.erase(e.begin());
    return toLowerCopy(e);
}

namespace
{
bool matchesKindExtension(MediaKind kind, const std::string &ext)
{
    switch (kind)
    {
    case MediaKind::Video:
        if (ext == "mp4")
            return true;
        break;
    case MediaKind::Audio:
        if (ext == "mp3")
            return true;
        break;
    case MediaKind::Image:
        if (ext == "jpg" || ext == "jpeg" || ext == "png")
            return true;
        break;
    }
    return false;
}
} // namespace

std::vector<fs::path> FileScanner::scan(const fs::path &root, MediaKind kind) const
{
    std::vector<fs::path> out;
    const fs::path dir = root / std::string(mediaKindDirectory(kind));
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return out;

    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file())
            continue;
        const auto p = entry.path();

        const auto ext = fileExtension(p);
        if (matchesKindExtension(kind, ext))
            out.push_back(p);
    }
    return out;
}
