cmake_minimum_required(VERSION 3.16)
project(MediaBrowser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(MEDIA_BROWSER_SERVICE_SOURCES
    src/services/FileScanner.cpp
    src/services/MediaLibrary.cpp
    src/services/MediaRepository.cpp
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
target_link_libraries(MediaBrowser PRIVATE Drogon::Drogon unofficial::sqlite3::sqlite3)

enable_testing()

add_executable(MediaBrowserTests
    tests/MediaLibraryTests.cpp
    ${MEDIA_BROWSER_SERVICE_SOURCES}
)

target_include_directories(MediaBrowserTests PRIVATE src)
target_link_libraries(MediaBrowserTests PRIVATE unofficial::sqlite3::sqlite3)

add_test(NAME MediaBrowserTests COMMAND MediaBrowserTests)
