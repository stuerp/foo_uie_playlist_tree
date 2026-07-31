
/** $VER: UIElement.cpp (2026.07.25) P. Stuer **/

#include "pch.h"

#include "UIElement.h"

#include "Log.h"

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
uielement_t::uielement_t() : m_bMsgHandled(FALSE)
{
}

/// <summary>
/// Deletes this instance.
/// </summary>
uielement_t::~uielement_t()
{
}

/// <summary>
/// Creates the window.
/// </summary>
LRESULT uielement_t::OnCreate(CREATESTRUCT * cs) noexcept
{
    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void uielement_t::OnDestroy() noexcept
{
}

/// <summary>
/// Handles the WM_SIZE message.
/// </summary>
void uielement_t::OnSize(UINT type, CSize size) noexcept
{
}

/// <summary>
/// Handles a change of the user interface colors.
/// </summary>
void uielement_t::OnColorsChanged() noexcept
{
    GetColors();
}

/// <summary>
/// Handles a change of the user interface fonts.
/// </summary>
void uielement_t::OnFontsChanged() noexcept
{
    GetFonts();

    ::InvalidateRect(m_hWnd, nullptr, TRUE);
}

#pragma region CWindowImpl<uielement_t>

/// <summary>
/// Gets the window class definition.
/// </summary>
CWndClassInfo & uielement_t::GetWndClassInfo()
{
    static ATL::CWndClassInfoW wci =
    {
        {
            sizeof(WNDCLASSEX),
            CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
            StartWindowProc,
            0, 0,
            NULL, // Instance,
            NULL, // Icon
            NULL, // Cursor
            NULL, // Background
            NULL, // Menu
            TEXT(STR_WINDOW_CLASS_NAME), // Class name
            NULL // Small Icon
        },
        NULL, NULL, IDC_ARROW, TRUE, 0, L""
    };

    return wci;
}

#pragma endregion
