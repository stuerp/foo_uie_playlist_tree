
/** $VER: Theme.h (2026.07.14) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>
#include <Windows.h>

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

    COLORREF GetWindowColor() const noexcept { return _ColorWindow; }
    COLORREF GetWindowTextColor() const noexcept { return _ColorWindowText; }

    COLORREF GetSelectionColor() const noexcept { return _ColorSelection; }
    COLORREF GetSelectionTextColor() const noexcept { return _ColorSelectionText; }

    COLORREF GetInactiveSelectionColor() const noexcept { return _ColorInactiveSelection; }
    COLORREF GetInactiveSelectionTextColor() const noexcept { return _ColorInactiveSelectionText; }

    COLORREF GetHighlightColor() const noexcept { return _ColorHighLight; }
    COLORREF GetHighlightTextColor() const noexcept { return _ColorHighLightText; }

    void SetWindowColor(COLORREF color) noexcept { _ColorWindow = color; }
    void SetWindowTextColor(COLORREF color) noexcept { _ColorWindowText = color; }

    void SetSelectionColor(COLORREF color) noexcept { _ColorSelection = color; }
    void SetSelectionTextColor(COLORREF color) noexcept { _ColorSelectionText = color; }

    void SetInactiveSelectionColor(COLORREF color) noexcept { _ColorInactiveSelection = color; }
    void SetInactiveSelectionTextColor(COLORREF color) noexcept { _ColorInactiveSelectionText = color; }

    void SetHighlightColor(COLORREF color) noexcept { _ColorHighLight = color; }
    void SetHighlightTextColor(COLORREF color) noexcept { _ColorHighLightText = color; }

    void SetActiveItemFrameColor(COLORREF color) noexcept { _ColorActiveItemFrame = color; }

private:
    bool _DarkMode;

    COLORREF _ColorWindow                = 0x202020;
    COLORREF _ColorWindowText            = 0xC0C0C0;

    COLORREF _ColorSelection             = 0x747474;
    COLORREF _ColorSelectionText         = 0xFFFFFF;

    COLORREF _ColorInactiveSelection     = 0x747474;
    COLORREF _ColorInactiveSelectionText = 0xFFFFFF;

    COLORREF _ColorHighLight             = 0x747474;
    COLORREF _ColorHighLightText         = 0xFFFFFF;

    COLORREF _ColorActiveItemFrame       = 0xFFFFFF;

    COLORREF _ColorHotLight              = 0xD69C56;
};

extern theme_t _Theme;
