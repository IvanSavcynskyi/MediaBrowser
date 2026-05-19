#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "IMediaCatalog.h"
#include "MediaRepository.h"

class SupabaseStorageScanner;

class MediaLibrary : public IMediaCatalog
{
public:
    MediaLibrary() = default;
    void loadFromRoot(const std::filesystem::path &root);
    void loadFromSupabase(const std::string &databaseUrl,
                          const SupabaseStorageScanner &storage);

    std::vector<const MediaItem *> items(MediaKind kind) const override;
    const MediaItem *findById(const std::string &id) const override;

    std::size_t size() const noexcept override { return items_.size(); }

private:
    class ItemFactory
    {
    public:
        static std::string makeId(const std::filesystem::path &p);
        static std::string mimeFromExt(const std::filesystem::path &p);
        static std::unique_ptr<MediaItem> create(MediaKind kind,
                                                 const std::filesystem::path &path,
                                                 std::string id,
                                                 std::uintmax_t size,
                                                 std::string mimeType);
    };

    std::vector<std::unique_ptr<MediaItem>> items_;
    std::unordered_map<std::string, const MediaItem *> byId_;

    std::vector<MediaRecord> scanRecords(const std::filesystem::path &root) const;
    void appendRecord(const MediaRecord &record);
};
