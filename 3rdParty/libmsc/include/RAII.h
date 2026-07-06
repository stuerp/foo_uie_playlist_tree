
/** $VER: RAII.h (2026.07.06) P. Stuer - RAII wrappers **/

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <sdkddkver.h>
#include <windows.h>

#include <string>
#include <filesystem>
#include <utility> // std::exchange

namespace fs = std::filesystem;

#include "CriticalSection.h"

#pragma once

namespace msc
{

/// <summary>
/// Implements a RAII wrapper for HANDLE. 
/// </summary>
class handle_t
{
public:
    explicit handle_t(HANDLE handle) noexcept : _Handle(handle) { }

    // Move-only type
    handle_t(const handle_t & other) = delete;
    handle_t & operator=(const handle_t & other) = delete;

    handle_t(handle_t && other) noexcept : _Handle(other._Handle)
    {
        other._Handle = INVALID_HANDLE_VALUE;
    }

    handle_t & operator=(handle_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _Handle = other._Handle;
            other._Handle = INVALID_HANDLE_VALUE;
        }

        return *this;
    }

    ~handle_t() noexcept
    {
        Reset();
    }

    HANDLE Get() const noexcept
    {
        return _Handle;
    }

    operator HANDLE() const noexcept
    {
        return _Handle;
    }

    explicit operator bool() const noexcept
    {
        return _Handle != INVALID_HANDLE_VALUE;
    }

    bool IsValid() const noexcept
    {
        return _Handle != INVALID_HANDLE_VALUE;
    }

    void Reset() noexcept
    {
        if (_Handle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(_Handle);
            _Handle = INVALID_HANDLE_VALUE;
        }
    }

private:
    HANDLE _Handle = INVALID_HANDLE_VALUE;
};

/// <summary>
/// Implements a synchronization lock. 
/// </summary>
class lock_t
{
public:
    lock_t(critical_section_t & cs) : _cs(cs)
    {
        _cs.Enter();
    }

    lock_t(const lock_t &) = delete;
    lock_t & operator=(const lock_t &) = delete;
    lock_t(lock_t &&) = delete;
    lock_t & operator=(lock_t &&) = delete;

    ~lock_t()
    {
        _cs.Leave();
    }
     
private:
    critical_section_t & _cs;
};

/// <summary>
/// Implements a RAII wrapper for HMODULE.
/// </summary>
class module_handle_t
{
public:
    explicit module_handle_t(const std::wstring filePath) noexcept : _hModule(::LoadLibraryExW(filePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE)) { }

    explicit module_handle_t(HMODULE hModule) noexcept : _hModule(hModule) { }

    // Move-only type
    module_handle_t(const module_handle_t & other) = delete;
    module_handle_t & operator=(const module_handle_t & other) = delete;

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

    operator HMODULE() const noexcept
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

}
