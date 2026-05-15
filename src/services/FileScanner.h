#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "../domain/MediaKind.h"

class FileScanner
{
public:
    std::vector<std::filesystem::path> scan(const std::filesystem::path &root,
                                            MediaKind kind) const;

private:
    static std::string fileExtension(const std::filesystem::path &p);
};
