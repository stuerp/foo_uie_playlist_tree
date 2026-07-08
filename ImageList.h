
/** $VER: ImageList.h (2026.07.07) P. Stuer **/

#pragma once

#include <SDKDDKVer.h>

#define NOMINMAX

#include <Windows.h>

class image_list_factory_t
{
private:
    image_list_factory_t();

public:
    static HIMAGELIST Create(const std::string & fileName, uint32_t iconSize, uint32_t maxIcons = ~0) noexcept;
    static uint32_t GetIconCount(HMODULE hModule) noexcept;
};
