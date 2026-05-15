#include "../src/services/FileScanner.h"
#include "../src/services/MediaLibrary.h"

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

void mediaLibraryPersistsAndRefreshesSqliteCatalog()
{
    const auto root = makeTestRoot();
    const auto videoPath = root / "videos" / "movie.mp4";
    const auto audioPath = root / "music" / "track.mp3";
    const auto imagePath = root / "images" / "photo.jpg";

    writeFile(videoPath, "video");
    writeFile(audioPath, "audio");
    writeFile(imagePath, "image");

    MediaLibrary library;
    library.loadFromRoot(root);

    expect(fs::exists(root / "media_browser.sqlite3"), "SQLite database should be created in media root");
    expect(library.size() == 3, "Library should contain all supported media files");
    expect(library.items(MediaKind::Video).size() == 1, "Library should expose one video");
    expect(library.items(MediaKind::Audio).size() == 1, "Library should expose one audio file");
    expect(library.items(MediaKind::Image).size() == 1, "Library should expose one image");

    const auto *video = library.items(MediaKind::Video).front();
    expect(video->mime() == "video/mp4", "Video MIME type should be persisted and restored");
    expect(library.findById(video->id()) == video, "findById should return the loaded item");

    fs::remove(audioPath);
    library.loadFromRoot(root);

    expect(library.size() == 2, "Library should remove missing files after rescan");
    expect(library.items(MediaKind::Audio).empty(), "Removed audio file should disappear from catalog");

    fs::remove_all(root);
}
} // namespace

int main()
{
    try
    {
        fileScannerFiltersByMediaKind();
        mediaLibraryPersistsAndRefreshesSqliteCatalog();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    std::cout << "MediaBrowser tests passed\n";
    return 0;
}
