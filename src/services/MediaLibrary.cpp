#include "MediaLibrary.h"
#include "FileScanner.h"
#include "MediaRepository.h"

#include "../domain/VideoItem.h"
#include "../domain/AudioItem.h"
#include "../domain/ImageItem.h"

#include <array>
#include <cctype>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

namespace fs = std::filesystem;

static std::string extLowerNoDot(const fs::path &p)
{
    std::string e = p.extension().string();
    if (!e.empty() && e[0] == '.')
        e.erase(e.begin());
    for (auto &ch : e)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return e;
}

std::string MediaLibrary::ItemFactory::makeId(const fs::path &p)
{
    fs::path canon = fs::weakly_canonical(p);
    std::string s = canon.string();

    std::size_t h = std::hash<std::string>{}(s);
    return std::to_string(h);
}

std::string MediaLibrary::ItemFactory::mimeFromExt(const fs::path &p)
{
    const auto ext = extLowerNoDot(p);
    if (ext == "mp4")
        return "video/mp4";
    if (ext == "mp3")
        return "audio/mpeg";
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "png")
        return "image/png";
    return "application/octet-stream";
}

std::unique_ptr<MediaItem> MediaLibrary::ItemFactory::create(MediaKind kind,
                                                             const fs::path &path,
                                                             std::string id,
                                                             std::uintmax_t size,
                                                             std::string mimeType)
{
    switch (kind)
    {
    case MediaKind::Video:
        return std::make_unique<VideoItem>(path, std::move(id), size, std::move(mimeType));
    case MediaKind::Audio:
        return std::make_unique<AudioItem>(path, std::move(id), size, std::move(mimeType));
    case MediaKind::Image:
        return std::make_unique<ImageItem>(path, std::move(id), size, std::move(mimeType));
    }
    throw std::logic_error("Unsupported media kind");
}

fs::path MediaLibrary::databasePathForRoot(const fs::path &root)
{
    return root / "media_browser.sqlite3";
}

std::vector<MediaRecord> MediaLibrary::scanRecords(const fs::path &root) const
{
    std::vector<MediaRecord> records;
    FileScanner scanner;
    constexpr std::array<MediaKind, 3> kinds{
        MediaKind::Video,
        MediaKind::Audio,
        MediaKind::Image,
    };

    for (const auto kind : kinds)
    {
        for (const auto &p : scanner.scan(root, kind))
        {
            records.push_back(MediaRecord{
                ItemFactory::makeId(p),
                p,
                kind,
                fs::file_size(p),
                ItemFactory::mimeFromExt(p),
            });
        }
    }

    return records;
}

void MediaLibrary::appendRecord(const MediaRecord &record)
{
    auto item = ItemFactory::create(record.kind,
                                    record.path,
                                    record.id,
                                    record.size,
                                    record.mimeType);
    byId_[record.id] = item.get();
    items_.push_back(std::move(item));
}

void MediaLibrary::loadFromRoot(const fs::path &root)
{
    items_.clear();
    byId_.clear();

    MediaRepository repository(databasePathForRoot(root));
    repository.initialize();
    repository.synchronize(root, scanRecords(root));

    for (const auto &record : repository.listByRoot(root))
    {
        appendRecord(record);
    }

    const struct
    {
        bool operator()(const std::unique_ptr<MediaItem> &lhs,
                        const std::unique_ptr<MediaItem> &rhs) const
        {
            if (lhs->kind() != rhs->kind())
                return static_cast<int>(lhs->kind()) < static_cast<int>(rhs->kind());
            return lhs->title() < rhs->title();
        }
    } sortByKindAndTitle;

    std::sort(items_.begin(), items_.end(), sortByKindAndTitle);
}

std::vector<const MediaItem *> MediaLibrary::items(MediaKind kind) const
{
    std::vector<const MediaItem *> out;
    out.reserve(items_.size());
    for (const auto &it : items_)
    {
        if (it->kind() == kind)
            out.push_back(it.get());
    }
    return out;
}

const MediaItem *MediaLibrary::findById(const std::string &id) const
{
    auto it = byId_.find(id);
    if (it == byId_.end())
        return nullptr;
    return it->second;
}
