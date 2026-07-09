
/** $VER: IconList.cpp (2026.07.09) P. Stuer **/

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

            const DWORD Styles = WS_CHILD | WS_VISIBLE | LVS_ICON | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;

            Instance->hListView = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", Styles, 0, 0, 0, 0, hWnd, (HMENU) IDC_LISTVIEW, ::GetModuleHandleW(NULL), NULL);

            if (Instance->hListView == NULL)
                return -1;

            ::SetWindowTheme(Instance->hListView, _Theme.IsDark() ? L"DarkMode_Explorer" : nullptr, nullptr);

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

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            if (!(pnmh->idFrom == IDC_LISTVIEW) && (pnmh->code == NM_CUSTOMDRAW))
                return FALSE;

            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            auto lvcd = (NMLVCUSTOMDRAW *) lParam;

            HDC & hDC = lvcd->nmcd.hdc;

            switch (lvcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                {
                    RECT rc;

                    ::GetClientRect(lvcd->nmcd.hdr.hwndFrom, &rc);

                    // Draw the control background.
                    HBRUSH hBrush = ::CreateSolidBrush(_Theme.GetSysColor(COLOR_WINDOW));

                    ::FillRect(hDC, &rc, hBrush);

                    ::DeleteObject(hBrush);

                    return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.
                }

                case CDDS_ITEMPREPAINT:
                    return CDRF_NOTIFYPOSTPAINT; // Request Post-Paint notifications.

                case CDDS_ITEMPOSTPAINT:
                {
                    RECT & rc = lvcd->nmcd.rc;

                    // Don't use CDIS_SELECTED (see https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-nmcustomdraw)
                    if (ListView_GetItemState(Instance->hListView, (int) lvcd->nmcd.dwItemSpec, LVIS_SELECTED))
                    {
                        HBRUSH hBrush = ::CreateSolidBrush(_Theme.GetSysColor(COLOR_HIGHLIGHT));

                        ::FillRect(hDC, &rc, hBrush);

                        ::DeleteObject(hBrush);
                    }

                    ::ImageList_Draw(Instance->hImageList, (int) lvcd->nmcd.dwItemSpec, hDC, rc.left + xPadding, rc.top + yPadding, ILD_NORMAL);

                    if (lvcd->nmcd.uItemState & CDIS_FOCUS)
                        ::DrawFocusRect(hDC, &rc);

                    return CDRF_DODEFAULT;
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

            int Index = ListView_GetNextItem(Instance->hListView, -1, (LPARAM) LVNI_SELECTED);

            if (Index != -1)
                ListView_SetItemState(Instance->hListView, Index, 0, LVIS_FOCUSED | LVIS_SELECTED);

            ListView_SetItemState(Instance->hListView, wParam, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            ListView_EnsureVisible(Instance->hListView, wParam, FALSE);

            return TRUE;
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

            ListView_SetIconSpacing(Instance->hListView, cx + (xPadding * 2), (cy + yPadding * 2));

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
