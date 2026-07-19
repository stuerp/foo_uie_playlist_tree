
/** $VER: TreeViewSubClass.h (2026.07.19) P. Stuer **/

#pragma once

#define NOMINMAX

#include <SDKDDKVer.h>
#include <Windows.h>
#include <CommCtrl.h>

#define NM_MCLICK   ((0U-9999U))

/// <summary>
/// Subclasses a TreeView control.
/// </summary>
class treeview_subclass_t
{
public:
    treeview_subclass_t() : _OldWndProc(nullptr) { }

    /// <summary>
    /// Attaches an existing tree view control.
    /// </summary>
    void Attach(HWND hWnd)
    {
        if (hWnd == NULL)
            return;

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR) this);

        _OldWndProc = (WNDPROC) ::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR) &treeview_subclass_t::WndProc);
    }

private:
    /// <summary>
    /// Handles Windows messages.
    /// </summary>
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto self = (treeview_subclass_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

        if ((self == nullptr) || (self->_OldWndProc == nullptr))
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);

        // Send a middle mouse button up notification.
        if (msg == WM_MBUTTONUP)
        {
            const NMHDR nmhd
            {
                .hwndFrom = hWnd,
                .idFrom   = (UINT_PTR) ::GetWindowLongPtrW(hWnd, GWLP_ID),
                .code     = NM_MCLICK
            };

            ::SendMessageW(::GetParent(hWnd), WM_NOTIFY, (WPARAM) nmhd.idFrom, (LPARAM) &nmhd);

            return 0;
        }

        return ::CallWindowProcW(self->_OldWndProc, hWnd, msg, wParam, lParam);
    }

private:
    WNDPROC _OldWndProc;
};
