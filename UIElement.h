
/** $VER: UIElement.h (2026.08.31) P. Stuer **/

#pragma once

#include "pch.h"

#include "State.h"
#include "Resources.h"

#include <sdk/coreDarkMode.h>

/// <summary>
/// Implements the user interface element base class.
/// </summary>
class uielement_t : public CWindowImpl<uielement_t>
{
public:
    uielement_t();

    uielement_t(const uielement_t &) = delete;
    uielement_t & operator=(const uielement_t &) = delete;
    uielement_t(uielement_t &&) = delete;
    uielement_t & operator=(uielement_t &&) = delete;

    virtual ~uielement_t();

    #pragma region CWindowImpl<UIElement>

    static CWndClassInfo & GetWndClassInfo();

    #pragma endregion

    virtual void OnColorsChanged() noexcept;
    virtual void OnFontsChanged() noexcept;

protected:
    /// <summary>
    /// Retrieves the GUID of the element.
    /// </summary>
    static const GUID & GetGUID() noexcept
    {
        static const GUID guid = GUID_UI_ELEMENT;

        return guid;
    }

    virtual void GetColors() noexcept = 0;
    virtual void GetFonts() noexcept = 0;

protected:
    state_t _State;
    fb2k::CCoreDarkModeHooks _DarkMode;
};
