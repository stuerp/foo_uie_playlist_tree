
/** $VER: TreeViewSubClass.h (2026.07.18) P. Stuer **/

#pragma once

#define NOMINMAX

#include <SDKDDKVer.h>
#include <Windows.h>
#include <CommCtrl.h>

#define TVN_MBUTTONUP   (TVN_LAST + 1)

/// <summary>
/// Subclasses a TreeView control.
/// </summary>
class treeview_subclass_t
{
public:
    treeview_subclass_t() : _OldWndProc(nullptr) { }

    void Attach(HWND hEdit)
    {
        if (hEdit == NULL)
            return;

        ::SetWindowLongPtrW(hEdit, GWLP_USERDATA, (LONG_PTR) this);

        _OldWndProc = (WNDPROC) ::SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR) &treeview_subclass_t::EditProc);
    }

private:
    static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto self = (treeview_subclass_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

        if ((self == nullptr) || (self->_OldWndProc == nullptr))
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);

        // Send a middle mouse button up notification.
        if (msg == WM_MBUTTONUP)
        {
            TVHITTESTINFO ht = { .pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) } };

            auto hItem = TreeView_HitTest(hWnd, &ht);

            if (hItem == NULL)
                return -1;

            const NMTREEVIEWW nmtv =
            {
                .hdr =
                {
                    .hwndFrom = hWnd,
                    .idFrom   = (UINT_PTR) ::GetWindowLongPtrW(hWnd, GWLP_ID),
                    .code     = TVN_MBUTTONUP
                },
            //  .action,
                .itemOld =
                {
                    .mask  = TVIF_PARAM,
                    .hItem = hItem,
                },
            //  .itemNew,
            //  .ptDrag
            };

            if (TreeView_GetItem(hWnd, &nmtv.itemOld) == FALSE)
                return -1;

            ::SendMessageW(::GetParent(hWnd), WM_NOTIFY, (WPARAM) nmtv.hdr.idFrom, (LPARAM) &nmtv);

            return 0;
        }

        return ::CallWindowProcW(self->_OldWndProc, hWnd, msg, wParam, lParam);
    }

private:
    WNDPROC _OldWndProc;
};
