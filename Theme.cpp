
/** $VER: Theme.cpp (2026.07.17) P. Stuer **/

#include "pch.h"

#include "Theme.h"

#include "Resources.h"
#include "Log.h"

#pragma hdrstop

theme_t _Theme;

/// <summary>
/// Destroys this instance.
/// </summary>
theme_t::~theme_t() noexcept
{
    Dispose();

    if (_hWindowBrush != NULL)
    {
        ::DeleteObject(_hWindowBrush);
        _hWindowBrush = NULL;
    }

    if (_hWindowTextPen != NULL)
    {
        ::DeleteObject(_hWindowTextPen);
        _hWindowTextPen = NULL;
    }

    if (_hSelectionBrush != NULL)
    {
        ::DeleteObject(_hSelectionBrush);
        _hSelectionBrush = NULL;
    }

    if (_hInactiveSelectionBrush != NULL)
    {
        ::DeleteObject(_hInactiveSelectionBrush);
        _hInactiveSelectionBrush = NULL;
    }
}

/// <summary>
/// Initializes some GDI object used by custom draw.
/// </summary>
HRESULT theme_t::Initialize() noexcept
{
    Dispose();

    // Get the font height.
    NONCLIENTMETRICSW ncm { sizeof(ncm) };

    if (::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0) != TRUE)
        Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to get non-client metrics: 0x%08X.", ::GetLastError());

    const LOGFONTW & lf = ncm.lfMessageFont;

    // Create the font.
    _hFont = ::CreateFontW(lf.lfHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe Fluent Icons");

    if (_hFont == NULL)
        Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to create font for custom draw: 0x%08X.", ::GetLastError());

    // Get the text style from the visual style.
    _hTextStyle = ::OpenThemeData(nullptr, L"TEXTSTYLE");

    return S_OK;
}

/// <summary>
/// Disposes the GDI objects.
/// </summary>
void theme_t::Dispose() noexcept
{
    if (_hTextStyle != NULL)
    {
        ::CloseThemeData(_hTextStyle);
        _hTextStyle = NULL;
    }

    if (_hFont != NULL)
    {
        ::DeleteObject(_hFont);
        _hFont = NULL;
    }
}

void theme_t::SetWindowColor(COLORREF color) noexcept
{
    if (_hWindowBrush != NULL)
        ::DeleteObject(_hWindowBrush);

    _ColorWindow = color;

    _hWindowBrush = ::CreateSolidBrush(color);
}

void theme_t::SetWindowTextColor(COLORREF color) noexcept
{
    if (_hWindowTextPen != NULL)
        ::DeleteObject(_hWindowTextPen);

    _ColorWindowText = color;

    _hWindowTextPen = ::CreatePen(PS_SOLID, 1, color);
}

void theme_t::SetSelectionColor(COLORREF color) noexcept
{
    if (_hSelectionBrush != NULL)
        ::DeleteObject(_hSelectionBrush);

    _ColorSelection = _IsDUI ? Blend(GetWindowColor(), color) : color;

    _hSelectionBrush = ::CreateSolidBrush(_ColorSelection);
}

void theme_t::SetInactiveSelectionColor(COLORREF color) noexcept
{
    if (_hInactiveSelectionBrush != NULL)
        ::DeleteObject(_hInactiveSelectionBrush);

    _ColorInactiveSelection = _IsDUI ? Blend(GetWindowColor(), color) : color;

    _hInactiveSelectionBrush = ::CreateSolidBrush(_ColorInactiveSelection);
}

void theme_t::SetHighlightColor(COLORREF color) noexcept
{
    if (_hHighlightBrush != NULL)
        ::DeleteObject(_hHighlightBrush);

    _ColorHighlight = _IsDUI ? Blend(GetWindowColor(), color) : color;

    _hHighlightBrush = ::CreateSolidBrush(_ColorHighlight);
    _hHighlightPen = msc::hpen_t(1, color);
}

/// <summary>
/// Blends two colors.
/// </summary>
COLORREF theme_t::Blend(COLORREF c1, COLORREF c2) noexcept
{
    const bool IsDarkMode = ui_config_manager::get()->is_dark_mode();

    const uint8_t Alpha = IsDarkMode ? 240 : 50;

    const uint32_t rb = (((c1 & 0x00FF00FFu) * (255 - Alpha)) + ((c2 & 0x00FF00FFu) * Alpha)) >> 8;

    const uint32_t g  = (((c1 & 0x0000FF00u) * (255 - Alpha)) + ((c2 & 0x0000FF00u) * Alpha)) >> 8;

    return (rb & 0x00FF00FFu) | (g & 0x0000FF00u);
}
