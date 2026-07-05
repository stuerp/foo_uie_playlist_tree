
/** $VER: TreeView.h (2026.07.05) P. Stuer **/

#pragma once

#include <SDKDDKVer.h>

#define NOMINMAX

#include <Windows.h>

class tree_view_t
{
public:
    tree_view_t() : _hTreeView() { };

    bool Create(_In_ HWND hWndParent, _In_ size_t id) noexcept
    {
        const DWORD Styles = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;

        _hTreeView = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_TREEVIEW, L"", Styles, 0, 0, 0, 0, hWndParent, (HMENU) id, THIS_HINSTANCE, nullptr);

        return (_hTreeView != NULL);
    }

    void Destroy() noexcept
    {
        if (_hTreeView == NULL)
            return;

        ::DestroyWindow(_hTreeView);
        _hTreeView = NULL;
    }

    HWND GetHandle() const
    {
        return _hTreeView;
    }

    void SetImageList(_In_ HIMAGELIST hImageList) const
    {
        TreeView_SetImageList(_hTreeView, hImageList, TVSIL_NORMAL);
        TreeView_SetImageList(_hTreeView, hImageList, TVSIL_STATE);
    }

    HTREEITEM Additem(_In_ const std::wstring & text, _In_ size_t playlistIndex, _In_ int iconIndex) const noexcept;

    void SelectItem(_In_ HTREEITEM hTreeItem) const noexcept
    {
        TreeView_SelectItem(_hTreeView, hTreeItem);
        TreeView_EnsureVisible(_hTreeView, hTreeItem);
    }

private:
    HWND _hTreeView;
};
