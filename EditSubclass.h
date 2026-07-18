
/** $VER: EditSubclass.h (2026.07.11) P. Stuer **/

#pragma once

/// <summary>
/// Subclasses an Edit control.
/// </summary>
class edit_subclass_t
{
public:
    edit_subclass_t() : _OldWndProc(nullptr) { }

    void Attach(HWND hEdit)
    {
        if (hEdit == NULL)
            return;

        ::SetWindowLongPtrW(hEdit, GWLP_USERDATA, (LONG_PTR) this);

        _OldWndProc = (WNDPROC) ::SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR) &edit_subclass_t::EditProc);
    }

private:
    static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
