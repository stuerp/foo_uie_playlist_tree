
/** $VER: IconList.cpp (2026.07.09) P. Stuer **/

#include "pch.h"

#include "IconList.h"

#include <dwmapi.h>
#include <uxtheme.h>

#pragma hdrstop

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

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

            auto pcd = (NMLVCUSTOMDRAW *) lParam;

            switch (pcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.

                case CDDS_ITEMPREPAINT:
                    return CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT; // Do default processing and request Post-Paint notifications.

                case CDDS_ITEMPOSTPAINT:
                {
                    HDC & hDC = pcd->nmcd.hdc;
                    RECT & rc = pcd->nmcd.rc;

                    // Don't use CDIS_SELECTED (see https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-nmcustomdraw)
                    if (ListView_GetItemState(Instance->hListView, (int) pcd->nmcd.dwItemSpec, LVIS_SELECTED))
                    {
                        HBRUSH hBrush = ::GetSysColorBrush(COLOR_HIGHLIGHT);

                        ::FillRect(hDC, &rc, hBrush);
                    }

                    ::ImageList_Draw(Instance->hImageList, (int) pcd->nmcd.dwItemSpec, hDC, rc.left + 1, rc.top + 1, ILD_NORMAL);

                    if (pcd->nmcd.uItemState & CDIS_FOCUS)
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

            ListView_SetItemState(Instance->hListView, wParam, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

            return TRUE;
        }

        case ILM_SETIMAGELIST:
        {
            auto Instance = (instance_t *) ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

            if (Instance == nullptr)
                return FALSE;

            Instance->hImageList = (HIMAGELIST) lParam;

            if (Instance->hImageList == NULL)
                return FALSE;

            int cx, cy;

            if (!::ImageList_GetIconSize(Instance->hImageList, &cx, &cy))
                return FALSE;

            ListView_SetIconSpacing(Instance->hListView, cx + (xPadding * 2), (cy + yPadding * 2));

            return TRUE;
        }
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
