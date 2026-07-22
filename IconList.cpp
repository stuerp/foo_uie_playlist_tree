
/** $VER: IconList.cpp (2026.07.22) P. Stuer **/

#include "pch.h"

#include "IconList.h"

#include "Theme.h"

#pragma hdrstop

#define IDC_LISTVIEW  100

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace icon_list_t
{

/// <summary>
/// Registers the control.
/// </summary>
bool Register(HINSTANCE hInstance) noexcept
{
    const WNDCLASSEXW wcx =
    {
        .cbSize        = sizeof(wcx),
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = WndProc,
        .hInstance     = hInstance,
        .hCursor       = ::LoadCursorW(NULL, IDC_ARROW),
        .lpszClassName = WC_ICON_LIST
    };

    return (::RegisterClassExW(&wcx) != 0);
}

}

struct instance_t
{
    instance_t() : hListView(NULL), hImageList(NULL) { }

    HWND hListView;
    HIMAGELIST hImageList;

    int IconSize = 16;
};

const int xPadding = 2;
const int yPadding = 2;

/// <summary>
/// 
/// </summary>
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            auto Instance = new instance_t();

            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR) Instance);

            {
                const DWORD Styles = WS_CHILD | WS_VISIBLE | LVS_ICON | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;
                const DWORD ExStyles = WS_EX_CLIENTEDGE | LVS_EX_DOUBLEBUFFER | LVS_EX_FLATSB;

                Instance->hListView = ::CreateWindowExW(ExStyles, WC_LISTVIEW, L"", Styles, 0, 0, 0, 0, hWnd, (HMENU) IDC_LISTVIEW, ::GetModuleHandleW(NULL), NULL);

                if (Instance->hListView == NULL)
                    return -1;

                ::SetWindowTheme(Instance->hListView, ui_config_manager::get()->is_dark_mode() ? L"DarkMode_Explorer" : L"Explorer", nullptr);
            }

            return 0;
        }

        case WM_DESTROY:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance != nullptr)
            {
                ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);

                delete Instance;
            }

            break;
        }

        case WM_SIZE:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            const LONG Width  = LOWORD(lParam);
            const LONG Height = HIWORD(lParam);

            ::MoveWindow(Instance->hListView, 0, 0, Width, Height, TRUE);

            return 0;
        }

        case WM_SETFOCUS:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            ::SetFocus(Instance->hListView); // Forward the focus.
            break;
        }

        case WM_NOTIFY:
        {
            auto nmhd = (NMHDR *) lParam;

            if (nmhd->idFrom != IDC_LISTVIEW)
                return FALSE;

            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            switch (nmhd->code)
            {
                case NM_CUSTOMDRAW:
                {
                    auto lvcd = (NMLVCUSTOMDRAW *) lParam;

                    const auto hListView = lvcd->nmcd.hdr.hwndFrom;
                    const auto hDC       = lvcd->nmcd.hdc;

                    switch (lvcd->nmcd.dwDrawStage)
                    {
                        case CDDS_PREPAINT:
                        {
                            // Draw the control background ourselves because a light/dark switch is not handled by fb2k::CCoreDarkModeHooks.
                            {
                                RECT rc;

                                ::GetClientRect(hListView, &rc);

                                ::FillRect(hDC, &rc, _Theme.GetWindowBrush());
                            }

                            return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.
                        }

                        case CDDS_ITEMPREPAINT:
                        {
                            const RECT & rcItem = lvcd->nmcd.rc;

                            const LVITEMW lvi
                            {
                                .mask       = LVIF_STATE,
                                .iItem      = (int) lvcd->nmcd.dwItemSpec,
                                .stateMask  = 0xFF,
                            };

                            ListView_GetItem(hListView, &lvi);

                            const auto IsSelected    = ((lvi.state & LVIS_SELECTED) != 0); //ListView_GetItemState(hListView, (int) lvcd->nmcd.dwItemSpec, LVIS_SELECTED); // Don't use CDIS_SELECTED (see https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-nmcustomdraw)
                            const auto IsHighlighted = ((lvi.state & LVIS_DROPHILITED) != 0);
                            const auto IsHot         = ((lvcd->nmcd.uItemState & CDIS_HOT) != 0);
                            const auto IsFocused     = ((lvcd->nmcd.uItemState & CDIS_FOCUS) != 0);

                            // Draw the background.
                            if (IsSelected)
                            {
                                auto & hBrush = _Theme.GetHighlightBrush();

                                ::FillRect(hDC, &rcItem, hBrush);

                                // Draw the focus rectangle.
                                if (IsFocused)
                                {
                                    auto & hPen = _Theme.GetHighlightPen();

                                    auto hOldBrush = ::SelectObject(hDC, hBrush);
                                    auto hOldPen = ::SelectObject(hDC, hPen);

                                    ::RoundRect(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, 2, 2);

                                    ::SelectObject(hDC, hOldPen);
                                    ::SelectObject(hDC, hOldBrush);
                                }
                            }
                            else
                            if (IsHot || IsHighlighted)
                            {
                                auto & hBrush = _Theme.GetHighlightBrush();

                                ::FillRect(hDC, &rcItem, hBrush);
                            }

                            ::ImageList_Draw(Instance->hImageList, (int) lvcd->nmcd.dwItemSpec, hDC, rcItem.left + xPadding, rcItem.top + yPadding, ILD_NORMAL);

                            return CDRF_SKIPDEFAULT; // Skip all other stages because we've drawn the complete item.
                        }
                    }
                    break;
                }

                case LVN_ITEMCHANGED:
                {
                    const auto nmlv = (NMLISTVIEW *) nmhd;

                    if (nmlv->uNewState & LVIS_SELECTED)
                    {
                        // Notify the parent of the selection change.
                        const NMHDR nh
                        {
                            .hwndFrom = hWnd,
                            .idFrom   = (UINT_PTR) ::GetWindowLongPtrW(hWnd, GWLP_ID),
                            .code     = ILN_SELCHANGE
                        };

                        ::SendMessageW(::GetParent(hWnd), WM_NOTIFY, (WPARAM) nh.idFrom, (LPARAM) &nh);
                    }

                    break;
                }
            }
            break;
        }

        case ILM_ADDITEM:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            const LVITEM lvi =
            {
                .mask   = LVIF_IMAGE,
                .iItem  = (int) lParam,
                .iImage = (int) lParam,
            };

            ListView_InsertItem(Instance->hListView, &lvi);

            return TRUE;
        }

        case ILM_SELECTITEM:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            // Deselect the old item.
            const int Index = ListView_GetNextItem(Instance->hListView, -1, (LPARAM) LVNI_SELECTED);

            if (Index != -1)
                ListView_SetItemState(Instance->hListView, Index, 0, LVIS_FOCUSED | LVIS_SELECTED);

            // Select the new item.
            ListView_SetItemState(Instance->hListView, wParam, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_EnsureVisible(Instance->hListView, wParam, FALSE);

            return TRUE;
        }

        case ILM_GETSELECTEDITEM:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            const int Index = ListView_GetNextItem(Instance->hListView, -1, (LPARAM) LVNI_SELECTED);

            return Index;
        }

        case ILM_SETIMAGELIST:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            ListView_DeleteAllItems(Instance->hListView);

            ::ImageList_Destroy(Instance->hImageList);

            Instance->hImageList = (HIMAGELIST) lParam;

            if (Instance->hImageList == NULL)
                return FALSE;

            int cx, cy;

            if (!::ImageList_GetIconSize(Instance->hImageList, &cx, &cy))
                return FALSE;

            Instance->IconSize = std::max(cx, cy);

            ListView_SetIconSpacing(Instance->hListView, Instance->IconSize + (xPadding * 2), Instance->IconSize + (yPadding * 2));

            const int ImageCount = ::ImageList_GetImageCount(Instance->hImageList);

            for (int i = 0; i < ImageCount; ++i)
            {
                const LVITEM lvi =
                {
                    .mask   = LVIF_IMAGE,
                    .iItem  = i,
                    .iImage = i,
                };

                ListView_InsertItem(Instance->hListView, &lvi);
            }

            return TRUE;
        }
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
