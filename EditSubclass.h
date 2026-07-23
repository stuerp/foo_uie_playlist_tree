
/** $VER: EditSubclass.h (2026.07.23) P. Stuer **/

#pragma once

#define NOMINMAX

#include <SDKDDKVer.h>
#include <Windows.h>

/// <summary>
/// Subclasses an Edit control.
/// </summary>
class edit_subclass_t
{
public:
    edit_subclass_t() : _OldWndProc(nullptr) { }

    /// <summary>
    /// Attaches an existing edit control.
    /// </summary>
    void Attach(HWND hWnd)
    {
        if (hWnd == NULL)
            return;

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR) this);

        _OldWndProc = (WNDPROC) ::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR) &edit_subclass_t::WndProc);
    }

    /// <summary>
    /// Detaches an existing edit control.
    /// </summary>
    void Detach(HWND hWnd)
    {
        if (hWnd == NULL)
            return;

        (WNDPROC) ::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR) _OldWndProc);

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR) nullptr);
    }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto self = (edit_subclass_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

        if ((self == nullptr) || (self->_OldWndProc == nullptr))
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);

        if (msg == WM_GETDLGCODE)
            return DLGC_WANTALLKEYS | CallWindowProc(self->_OldWndProc, hWnd, msg, wParam, lParam); // Request all keys so the Edit control doesn't swallow Enter/Escape.

        return ::CallWindowProcW(self->_OldWndProc, hWnd, msg, wParam, lParam);
    }

private:
    WNDPROC _OldWndProc;
};
