
/** $VER: RAII.h (2026.07.22) P. Stuer - RAII wrappers **/

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <sdkddkver.h>
#include <windows.h>
#include <CommCtrl.h>

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
class hmodule_t
{
public:
    explicit hmodule_t(const std::wstring filePath) noexcept : _hModule(::LoadLibraryExW(filePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE)) { }

    explicit hmodule_t(HMODULE hModule) noexcept : _hModule(hModule) { }

    // Move-only type
    hmodule_t(const hmodule_t & other) = delete;
    hmodule_t & operator=(const hmodule_t & other) = delete;

    hmodule_t(hmodule_t && other) noexcept : _hModule(other.Release()) { }

    hmodule_t & operator=(hmodule_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _hModule = other.Release();
        }

        return *this;
    }

    ~hmodule_t()
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

/// <summary>
/// Implements a RAII wrapper for HIMAGELIST.
/// </summary>
class himagelist_t
{
public:
    himagelist_t() noexcept : _hImageList(nullptr) { }
    himagelist_t(HIMAGELIST hImageList) noexcept : _hImageList(hImageList) { }

    // Move-only type
    himagelist_t(const himagelist_t & other) = delete;
    himagelist_t & operator=(const himagelist_t & other) = delete;

    himagelist_t(himagelist_t && other) noexcept : _hImageList(other.Release()) { }

    himagelist_t & operator =(HIMAGELIST hImageList) noexcept
    {
        _hImageList = hImageList;

        return *this;
    }

    himagelist_t & operator =(himagelist_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _hImageList = other.Release();
        }

        return *this;
    }

    ~himagelist_t()
    {
        Reset();
    }

    HIMAGELIST Get() const noexcept
    {
        return _hImageList;
    }

    operator HIMAGELIST() const noexcept
    {
        return _hImageList;
    }

    explicit operator bool() const noexcept
    {
        return _hImageList != nullptr;
    }

    HIMAGELIST Release() noexcept
    {
        return std::exchange(_hImageList, nullptr);
    }

    bool Reset() noexcept
    {
        if (_hImageList == nullptr)
            return true;

        const BOOL Result = ::ImageList_Destroy(_hImageList);

        _hImageList = nullptr;

        return Result != FALSE;
    }

private:
    HIMAGELIST _hImageList = nullptr;
};

/// <summary>
/// Implements a RAII wrapper for HICON.
/// </summary>
class hicon_t
{
public:
    hicon_t() noexcept : _hIcon(nullptr) { }
    hicon_t(HICON hIcon) noexcept : _hIcon(hIcon) { } // Take ownership

    // Move-only type
    hicon_t(const hicon_t & other) = delete;
    hicon_t & operator=(const hicon_t & other) = delete;

    hicon_t(hicon_t && other) noexcept : _hIcon(other.Release()) { }

    hicon_t & operator =(HICON hIcon) noexcept
    {
        _hIcon = hIcon;

        return *this;
    }

    hicon_t & operator =(hicon_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _hIcon = other.Release();
        }

        return *this;
    }

    ~hicon_t()
    {
        Reset();
    }

    HICON Get() const noexcept
    {
        return _hIcon;
    }

    operator HICON() const noexcept
    {
        return _hIcon;
    }

    explicit operator bool() const noexcept
    {
        return _hIcon != nullptr;
    }

    HICON Release() noexcept
    {
        return std::exchange(_hIcon, nullptr);
    }

    bool Reset() noexcept
    {
        if (_hIcon == nullptr)
            return true;

        const BOOL Result = ::DestroyIcon(_hIcon);

        _hIcon = nullptr;

        return Result != FALSE;
    }

private:
    HICON _hIcon = nullptr;
};

/// <summary>
/// Implements a RAII wrapper for HBRUSH.
/// </summary>
class hbrush_t
{
public:
    explicit hbrush_t() : _hBrush(nullptr) { }

    explicit hbrush_t(COLORREF color) : _hBrush(::CreateSolidBrush(color))
    {
        if (_hBrush == nullptr)
            throw std::runtime_error("CreateSolidBrush() failed");
    }

    hbrush_t(int style, COLORREF color) : _hBrush(::CreateHatchBrush(style, color))
    {
        if (_hBrush == nullptr)
            throw std::runtime_error("CreateHatchBrush() failed");
    }

    explicit hbrush_t(HBRUSH hBrush) noexcept : _hBrush(hBrush) { } // Take ownership

    // Move-only type
    hbrush_t(const hbrush_t & other) = delete;
    hbrush_t & operator=(const hbrush_t & other) = delete;

    hbrush_t(hbrush_t && other) noexcept : _hBrush(std::exchange(other._hBrush, nullptr)) { }

    hbrush_t & operator =(hbrush_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _hBrush = std::exchange(other._hBrush, nullptr);
        }

        return *this;
    }


    ~hbrush_t() noexcept
    {
        Reset();
    }

    HBRUSH Get() const noexcept
    {
        return _hBrush;
    }

    explicit operator HBRUSH() const noexcept
    {
        return _hBrush;
    }

    operator HGDIOBJ() const noexcept
    {
        return (HGDIOBJ) _hBrush;
    }

    explicit operator bool() const noexcept
    {
        return _hBrush != nullptr;
    }

    bool Reset() noexcept
    {
        if (_hBrush == nullptr)
            return true;

        const BOOL Result = ::DeleteObject(_hBrush);

        _hBrush = nullptr;

        return Result != FALSE;
    }

private:
    HBRUSH _hBrush = nullptr;
};

inline HGDIOBJ SelectObject(HDC hDC, const hbrush_t & brush) noexcept
{
    return ::SelectObject(hDC, brush.Get());
}

/// <summary>
/// Implements a RAII wrapper for HPEN.
/// </summary>
class hpen_t
{
public:
    explicit hpen_t() : _hPen(nullptr) { }

    explicit hpen_t(int width, COLORREF color) : _hPen(::CreatePen(PS_SOLID, width, color))
    {
        if (_hPen == nullptr)
            throw std::runtime_error("CreatePen() failed");
    }

    explicit hpen_t(HPEN hPen) noexcept : _hPen(hPen) { } // Take ownership

    // Move-only type
    hpen_t(const hpen_t & other) = delete;
    hpen_t & operator=(const hpen_t & other) = delete;

    hpen_t(hpen_t && other) noexcept : _hPen(std::exchange(other._hPen, nullptr)) { }

    hpen_t & operator =(hpen_t && other) noexcept
    {
        if (this != &other)
        {
            Reset();

            _hPen = std::exchange(other._hPen, nullptr);
        }

        return *this;
    }


    ~hpen_t() noexcept
    {
        Reset();
    }

    HPEN Get() const noexcept
    {
        return _hPen;
    }

    explicit operator HPEN() const noexcept
    {
        return _hPen;
    }

    operator HGDIOBJ() const noexcept
    {
        return (HGDIOBJ) _hPen;
    }

    explicit operator bool() const noexcept
    {
        return _hPen != nullptr;
    }

    bool Reset() noexcept
    {
        if (_hPen == nullptr)
            return true;

        const BOOL Result = ::DeleteObject(_hPen);

        _hPen = nullptr;

        return Result != FALSE;
    }

private:
    HPEN _hPen = nullptr;
};

inline HGDIOBJ SelectObject(HDC hDC, const hpen_t & pen) noexcept
{
    return ::SelectObject(hDC, pen.Get());
}

}
