
/** $VER: Theme.h (2026.07.10) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>
#include <Windows.h>

#include <map>

class theme_t
{
public:
    void Initialize(bool darkMode) noexcept
    {
        _DarkMode = darkMode;
    }

    bool IsDark() const noexcept
    {
        return _DarkMode;
    }

    void SetColor(int index, COLORREF color) noexcept;
    COLORREF GetColor(int index) const noexcept;

private:
    bool _DarkMode;

    COLORREF _ColorWindow       = 0x202020;
    COLORREF _ColorWindowText   = 0xC0C0C0;
    COLORREF _ColorHighLight    = ::GetSysColor(COLOR_HOTLIGHT); //0x777777;
    COLORREF _ColorHotLight     = 0xD69C56;
};

extern theme_t _Theme;
