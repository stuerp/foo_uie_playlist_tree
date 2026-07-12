
/** $VER: UIElement.cpp (2026.07.04) P. Stuer **/

#include "pch.h"

#include "UIElement.h"

#include "Theme.h"

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
uielement_t::uielement_t() : m_bMsgHandled(FALSE)
{
    _Theme.Initialize(_DarkMode);
}

/// <summary>
/// Deletes this instance.
/// </summary>
uielement_t::~uielement_t()
{
    OnDestroy();
}

/// <summary>
/// Creates the window.
/// </summary>
LRESULT uielement_t::OnCreate(LPCREATESTRUCT cs) noexcept
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
void uielement_t::OnColorsChanged()
{
    GetColors();
}

/// <summary>
/// Shows the preferences page.
/// </summary>
void uielement_t::ShowPreferences() noexcept
{
    static constexpr GUID _GUID = GUID_PREFERENCES;

    static_api_ptr_t<ui_control> uc;

    uc->show_preferences(_GUID);
}

#pragma region CWindowImpl<UIElement>

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
            NULL, // Background brush
            NULL, // Menu
            TEXT(STR_WINDOW_CLASS_NAME), // Class name
            NULL // Small Icon
        },
        NULL, NULL, IDC_ARROW, TRUE, 0, L""
    };

    return wci;
}

#pragma endregion
