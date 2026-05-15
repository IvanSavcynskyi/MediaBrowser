#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "../domain/MediaKind.h"

struct MediaRecord
{
    std::string id;
    std::filesystem::path path;
    MediaKind kind;
    std::uintmax_t size;
    std::string mimeType;
};

class MediaRepository
{
public:
    explicit MediaRepository(std::filesystem::path databasePath);
    ~MediaRepository();

    MediaRepository(const MediaRepository &) = delete;
    MediaRepository &operator=(const MediaRepository &) = delete;

    void initialize();
    void synchronize(const std::filesystem::path &root,
                     const std::vector<MediaRecord> &records);
    std::vector<MediaRecord> listByRoot(const std::filesystem::path &root) const;

private:
    struct sqlite3 *db_{};
    std::filesystem::path databasePath_;

    void execute(const char *sql) const;
};
