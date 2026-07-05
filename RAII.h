
/** $VER: RAII.h (2026.07.05) P. Stuer - RAII wrappers **/

#pragma once

#include <windows.h>

#include <string>
#include <utility> // std::exchange

/// <summary>
/// RAII wrapper for HMODULE
/// </summary>
class module_handle_t
{
public:
    explicit module_handle_t(const std::wstring filePath) : _hModule(::LoadLibraryExW(filePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE)) { }

    explicit module_handle_t(HMODULE hModule) noexcept : _hModule(hModule) { }

    // Move-only type
    module_handle_t(const module_handle_t &) = delete;
    module_handle_t & operator=(const module_handle_t &) = delete;

    module_handle_t(module_handle_t && other) noexcept : _hModule(other.Release()) { }

    module_handle_t & operator=(module_handle_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _hModule = other.Release();
        }

        return *this;
    }

    ~module_handle_t()
    {
        Reset();
    }

    HMODULE Get() const noexcept
    {
        return _hModule;
    }

    explicit operator bool() const noexcept
    {
        return _hModule != nullptr;
    }

    HMODULE Release() noexcept
    {
        return std::exchange(_hModule, nullptr);
    }

    bool Reset() noexcept
    {
        if (_hModule == nullptr)
            return true;

        const BOOL Result = ::FreeLibrary(_hModule);

        _hModule = nullptr;

        return Result != FALSE;
    }

private:
    HMODULE _hModule = nullptr;
};
