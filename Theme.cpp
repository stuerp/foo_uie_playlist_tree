
/** $VER: Theme.cpp (2026.07.14) P. Stuer **/

#include "pch.h"

#include "Theme.h"

#pragma hdrstop

/// <summary>
/// Sets the current color of the specified display element, taking dark mode into account (Taken from libppui)
/// </summary>
void theme_t::SetColor(int index, COLORREF color) noexcept
{
    if (!_DarkMode)
        return;

    switch (index)
    {
        case COLOR_WINDOW:
            _ColorWindow = color;
            break;

        case COLOR_WINDOWTEXT:
            _ColorWindowText = color;
            break;

        case COLOR_HIGHLIGHT:
            _ColorHighLight = color;
            break;

        case COLOR_HIGHLIGHTTEXT:
            _ColorHighLightText = color;
            break;

        case COLOR_MENUHILIGHT:
            _ColorInactiveSelection = color;
            break;

        case COLOR_GRAYTEXT:
            _ColorInactiveSelectionText = color;
            break;

        case COLOR_HOTLIGHT:
            _ColorHotLight = color;
            break;
    }
}

/// <summary>
/// Retrieves the current color of the specified display element, taking dark mode into account (Taken from libppui)
/// </summary>
COLORREF theme_t::GetColor(int index) const noexcept
{
    if (!_DarkMode)
        return ::GetSysColor(index);

    switch (index)
    {
        case COLOR_MENU:
        case COLOR_BTNFACE:
        case COLOR_WINDOW:
        case COLOR_MENUBAR:
            return _ColorWindow;

        case COLOR_BTNSHADOW:
            return 0;

        case COLOR_WINDOWTEXT:
        case COLOR_MENUTEXT:
        case COLOR_BTNTEXT:
        case COLOR_CAPTIONTEXT:
            return _ColorWindowText;

        case COLOR_BTNHIGHLIGHT:
        case COLOR_MENUHILIGHT:
            return 0x383838;

        case COLOR_HIGHLIGHT:
            return _ColorHighLight;

        case COLOR_HIGHLIGHTTEXT:
            return 0xFFFFFF; //0x101010;

        case COLOR_GRAYTEXT:
            return 0x777777;

        case COLOR_HOTLIGHT:
            return _ColorHotLight;

        default:
            return ::GetSysColor(index);
    }
}

/*
#4E4E4E // Selection, no focus
#909090 // Focus Rect, No selection

// Active selection
#BBBBBB // Focus Rect
#747474 // Back
#FFFFFF // Fore

// Inactive selection
#484848 // Back
#FFFFFF // Fore
*/

theme_t _Theme;
