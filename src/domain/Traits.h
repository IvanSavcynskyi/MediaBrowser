#pragma once
#include <string>
#include <cstdint>

class IPlayable
{
public:
    virtual ~IPlayable() = default;

    virtual int durationSec() const = 0;

    virtual std::string playerTag() const = 0;
};

class IPreviewable
{
public:
    virtual ~IPreviewable() = default;

    virtual int width() const = 0;
    virtual int height() const = 0;
};