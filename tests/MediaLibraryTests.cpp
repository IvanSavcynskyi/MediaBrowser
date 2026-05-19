#include "../src/services/FileScanner.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <ctime>

namespace fs = std::filesystem;

namespace
{
class TestFailure : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

void expect(bool condition, const std::string &message)
{
    if (!condition)
        throw TestFailure(message);
}

fs::path makeTestRoot()
{
    auto root = fs::temp_directory_path() /
                ("media_browser_tests_" + std::to_string(std::time(nullptr)));
    fs::remove_all(root);
    fs::create_directories(root / "videos");
    fs::create_directories(root / "music");
    fs::create_directories(root / "images");
    return root;
}

void writeFile(const fs::path &path, const std::string &content = "test")
{
    std::ofstream out(path, std::ios::binary);
    out << content;
}

void fileScannerFiltersByMediaKind()
{
    const auto root = makeTestRoot();
    writeFile(root / "videos" / "movie.MP4");
    writeFile(root / "videos" / "notes.txt");
    writeFile(root / "music" / "track.mp3");
    writeFile(root / "images" / "photo.png");

    FileScanner scanner;
    const auto videos = scanner.scan(root, MediaKind::Video);
    const auto audio = scanner.scan(root, MediaKind::Audio);
    const auto images = scanner.scan(root, MediaKind::Image);

    expect(videos.size() == 1, "Video scanner should return only mp4 files");
    expect(audio.size() == 1, "Audio scanner should return only mp3 files");
    expect(images.size() == 1, "Image scanner should return jpg/jpeg/png files");

    fs::remove_all(root);
}

} // namespace

int main()
{
    try
    {
        fileScannerFiltersByMediaKind();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    std::cout << "MediaBrowser tests passed\n";
    return 0;
}
