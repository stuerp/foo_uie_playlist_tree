
/** $VER: Theme.h (2026.07.22) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>
#include <Windows.h>

#include <SDK\coreDarkMode.h>

class theme_t
{
public:
    ~theme_t() noexcept;

    HRESULT Initialize() noexcept;

    HFONT GetFont() const noexcept { return _hFont; }
    HTHEME GetTextStyle() const noexcept { return _hTextStyle; }

    COLORREF GetWindowColor() const noexcept { return _ColorWindow; }
    COLORREF GetWindowTextColor() const noexcept { return _ColorWindowText; }

    COLORREF GetSelectionColor() const noexcept { return _ColorSelection; }
    COLORREF GetSelectionTextColor() const noexcept { return _ColorSelectionText; }

    COLORREF GetInactiveSelectionColor() const noexcept { return _ColorInactiveSelection; }
    COLORREF GetInactiveSelectionTextColor() const noexcept { return _ColorInactiveSelectionText; }

    COLORREF GetHighlightColor() const noexcept { return _ColorHighlight; }
    COLORREF GetHighlightTextColor() const noexcept { return _ColorHighlightText; }

    void SetWindowColor(COLORREF color) noexcept;
    void SetWindowTextColor(COLORREF color) noexcept;

    void SetSelectionColor(COLORREF color) noexcept;
    void SetSelectionTextColor(COLORREF color) noexcept { _ColorSelectionText = color; }

    void SetInactiveSelectionColor(COLORREF color) noexcept;
    void SetInactiveSelectionTextColor(COLORREF color) noexcept { _ColorInactiveSelectionText = color; }

    void SetHighlightColor(COLORREF color) noexcept;
    void SetHighlightTextColor(COLORREF color) noexcept { _ColorHighlightText = color; }

    void SetActiveItemFrameColor(COLORREF color) noexcept { _ColorActiveItemFrame = color; }

    msc::brush_t & GetWindowBrush() noexcept { return _hWindowBrush; }
    msc::pen_t & GetWindowTextPen() noexcept { return _hWindowTextPen; }

    msc::brush_t & GetSelectionBrush() noexcept { return _hSelectionBrush; }
    msc::brush_t & GetInactiveSelectionBrush() noexcept { return _hInactiveSelectionBrush; }

    msc::brush_t & GetHighlightBrush() noexcept { return _hHighlightBrush; }
    msc::pen_t & GetHighlightPen() noexcept { return _hHighlightPen; }

private:
    void Dispose() noexcept;

    COLORREF Blend(COLORREF a, COLORREF b) noexcept;

public:
    bool _IsDUI = true; // Ugly but I don't know any API to detect if DUI or CUI is active, independent from a panel.

private:
    HFONT _hFont = NULL;
    HTHEME _hTextStyle = NULL;

    COLORREF _ColorWindow                = 0x202020;
    COLORREF _ColorWindowText            = 0xC0C0C0;

    COLORREF _ColorSelection             = 0x747474;
    COLORREF _ColorSelectionText         = 0xFFFFFF;

    COLORREF _ColorInactiveSelection     = 0x747474;
    COLORREF _ColorInactiveSelectionText = 0xFFFFFF;

    COLORREF _ColorHighlight             = 0x747474;
    COLORREF _ColorHighlightText         = 0xFFFFFF;

    COLORREF _ColorActiveItemFrame       = 0xFFFFFF;

    COLORREF _ColorHotLight              = 0xD69C56;

    msc::brush_t _hWindowBrush;
    msc::pen_t _hWindowTextPen;

    msc::brush_t _hSelectionBrush;
    msc::brush_t _hInactiveSelectionBrush;

    msc::brush_t _hHighlightBrush;
    msc::pen_t _hHighlightPen;
};

extern theme_t _Theme;
