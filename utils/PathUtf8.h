#pragma once
#include <filesystem>
#include <string>

inline std::string pathToUtf8(const std::filesystem::path &p)
{
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    auto u8 = p.u8string();
    return std::string(u8.begin(), u8.end());
#else
    return p.u8string();
#endif
}