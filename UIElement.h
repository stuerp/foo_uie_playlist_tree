
/** $VER: UIElement.h (2026.07.06) P. Stuer **/

#pragma once

#include "pch.h"

#include "Resources.h"

#include <SDK\coreDarkMode.h>

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

    void OnColorsChanged();

    #pragma endregion

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

protected:
    #pragma region CWindowImpl

    virtual LRESULT OnCreate(LPCREATESTRUCT cs) noexcept;
    virtual void OnDestroy() noexcept;
    virtual void OnSize(UINT nType, CSize size) noexcept;

    BEGIN_MSG_MAP_EX(uielement_t)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

    #pragma endregion

private:
    void ShowPreferences() noexcept;

protected:
    COLORREF _ForegroundColor;
    COLORREF _BackgroundColor;

    fb2k::CCoreDarkModeHooks _DarkMode;
};
