#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../domain/MediaKind.h"

struct MediaRecord
{
    std::string id;
    std::string objectPath;
    std::string storageUrl;
    MediaKind kind;
    std::uintmax_t size;
    std::string mimeType;
};

class MediaRepository
{
public:
    explicit MediaRepository(std::string databaseUrl);
    ~MediaRepository() = default;

    MediaRepository(const MediaRepository &) = delete;
    MediaRepository &operator=(const MediaRepository &) = delete;

    void initialize();
    void synchronize(const std::string &bucket,
                     const std::vector<MediaRecord> &records);
    std::vector<MediaRecord> listByBucket(const std::string &bucket) const;

private:
    std::string databaseUrl_;
};
