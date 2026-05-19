cmake_minimum_required(VERSION 3.16)
project(MediaBrowser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)

set(MEDIA_BROWSER_SERVICE_SOURCES
    src/services/FileScanner.cpp
    src/services/MediaLibrary.cpp
    src/services/MediaRepository.cpp
    src/services/SupabaseStorageScanner.cpp
)

add_executable(MediaBrowser
    src/main.cpp

    ${MEDIA_BROWSER_SERVICE_SOURCES}
    src/services/StreamingService.cpp

    src/controllers/ApiMediaController.cpp
    src/controllers/FrontendController.cpp
    src/controllers/MediaController.cpp
)

target_include_directories(MediaBrowser PRIVATE src)
target_link_libraries(MediaBrowser PRIVATE
    Drogon::Drogon
    CURL::libcurl
    libpqxx::pqxx
    OpenSSL::Crypto
    tinyxml2::tinyxml2
)

enable_testing()

add_executable(MediaBrowserTests
    tests/MediaLibraryTests.cpp
    src/services/FileScanner.cpp
)

target_include_directories(MediaBrowserTests PRIVATE src)

add_test(NAME MediaBrowserTests COMMAND MediaBrowserTests)
