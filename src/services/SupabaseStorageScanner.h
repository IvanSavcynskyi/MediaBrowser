#pragma once

#include <string>
#include <vector>

#include "MediaRepository.h"

class SupabaseStorageScanner
{
public:
    SupabaseStorageScanner(std::string endpoint,
                           std::string region,
                           std::string accessId,
                           std::string accessKey,
                           std::string bucket);

    const std::string &bucket() const noexcept { return bucket_; }
    std::vector<MediaRecord> scan() const;
    std::string presignedGetUrl(const std::string &objectPath,
                                int expiresSeconds = 3600) const;

private:
    std::string endpoint_;
    std::string region_;
    std::string accessId_;
    std::string accessKey_;
    std::string bucket_;

    std::vector<MediaRecord> scanPrefix(const std::string &prefix) const;
};
