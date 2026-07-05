
/** $VER: TreeView.cpp (2026.07.05) P. Stuer **/

#include "pch.h"

#include "TreeView.h"

#pragma hdrstop

/// <summary>
/// Creates the control.
/// </summary>
bool tree_view_t::Create(_In_ HWND hWndParent, _In_ size_t id) noexcept
{
    const DWORD Styles = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_EX_MULTISELECT;

    _hTreeView = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_TREEVIEW, L"", Styles, 0, 0, 0, 0, hWndParent, (HMENU) id, THIS_HINSTANCE, nullptr);

    return (_hTreeView != NULL);
}

/// <summary>
/// Destroys the control.
/// </summary>
void tree_view_t::Destroy() noexcept
{
    if (_hTreeView == NULL)
        return;

    ::DestroyWindow(_hTreeView);
    _hTreeView = NULL;
}

/// <summary>
/// Adds the specified item to the treeview.
/// </summary>
HTREEITEM tree_view_t::AddItem(_In_ HTREEITEM hParent, _In_ HTREEITEM hInsertAfter, _In_ int iconIndex, _In_ const node_t * node) const noexcept
{
    std::wstring Text = msc::UTF8ToWide(node->Name);

    const TVINSERTSTRUCTW tvis
    {
        .hParent      = hParent,
        .hInsertAfter = hInsertAfter,
        .item         =
        {
            .mask           = Mask,
            .pszText        = (LPWSTR) Text.c_str(),
            .iImage         = iconIndex,
            .iSelectedImage = iconIndex,
            .lParam         = (LPARAM) node,
        },
    };

    auto hTreeItem = TreeView_InsertItem(_hTreeView, &tvis);

    return hTreeItem;
}

/// <summary>
/// Removes the specified item from the treeview.
/// </summary>
bool tree_view_t::RemoveItem(_In_ HTREEITEM hItem) const noexcept
{
    return (TreeView_DeleteItem(_hTreeView, hItem) == TRUE);
}

/// <summary>
/// Moves an item into another.
/// </summary>
void tree_view_t::MoveItem(_In_ HTREEITEM hTreeItem, _In_ HTREEITEM hDropTarget) const noexcept
{
    wchar_t Text[512] = { };

    TVINSERTSTRUCTW tvis =
    {
        .hParent = hDropTarget,
        .hInsertAfter = TVI_LAST,
        .item =
        {
            .mask = Mask,
            .hItem = hTreeItem,
            .pszText = Text,
            .cchTextMax = _countof(Text),
        }
    };

    TreeView_GetItem(_hTreeView, &tvis.item);

    // Insert a new version of the item.
    HTREEITEM hNew = TreeView_InsertItem(_hTreeView, &tvis);

    if (hNew == NULL)
        return;

    // Recursively move the children.
    HTREEITEM hChild = TreeView_GetChild(_hTreeView, hTreeItem);

    while (hChild != NULL)
    {
        HTREEITEM hNextChild = TreeView_GetNextSibling(_hTreeView, hChild);

        MoveItem(hChild, hNew);

        hChild = hNextChild;
    }

    // Delete the original item.
    TreeView_DeleteItem(_hTreeView, hTreeItem);
}

/// <summary>
/// Begins a drag operation.
/// </summary>
void tree_view_t::BeginDrag(_In_ const NMTREEVIEW * nmtv) noexcept
{
    _hDragImageList = TreeView_CreateDragImage(_hTreeView, nmtv->itemNew.hItem);

    if (_hDragImageList == NULL)
        return;

    RECT rcItem;

    TreeView_GetItemRect(_hTreeView, nmtv->itemNew.hItem, &rcItem, TRUE);

    ::ImageList_BeginDrag(_hDragImageList, 0, 0, 0);
    ::ImageList_DragEnter(_hTreeView, nmtv->ptDrag.x, nmtv->ptDrag.y);

    ::ShowCursor(FALSE);
    ::SetCapture(::GetParent(_hTreeView));

    _hDraggedItem = nmtv->itemNew.hItem;
}

/// <summary>
/// Moves the drag item.
/// </summary>
void tree_view_t::DragMove(_In_ const CPoint & point) noexcept
{
    if (_hDraggedItem == NULL)
        return;

    POINT pt = point;

    {
        ::ClientToScreen(::GetParent(_hTreeView), &pt);
        ::ScreenToClient(_hTreeView, &pt);

        ::ImageList_DragMove(pt.x, pt.y);
    }

    // Determine the drop target and highlight it.
    {
        ::ImageList_DragShowNolock(FALSE);

        const TVHITTESTINFO tvht = { .pt = pt };

        const HTREEITEM hDropTarget = TreeView_HitTest(_hTreeView, &tvht);

        if (hDropTarget != NULL)
            TreeView_SelectDropTarget(_hTreeView, hDropTarget);

        ::ImageList_DragShowNolock(TRUE);
    }
}

/// <summary>
/// Ends the drag operation.
/// </summary>
void tree_view_t::EndDrag() noexcept
{
    if (_hDraggedItem == NULL)
        return;

    ::ImageList_EndDrag();

    {
        const HTREEITEM hDropTarget = TreeView_GetDropHilight(_hTreeView);

        if ((hDropTarget != NULL) && (hDropTarget != _hDraggedItem))
            MoveItem(_hDraggedItem, hDropTarget);

        TreeView_SelectDropTarget(_hTreeView, NULL);
    }

    ::ReleaseCapture();
    ::ShowCursor(TRUE);

    _hDraggedItem = NULL;

    if (_hDragImageList)
    {
        ::ImageList_Destroy(_hDragImageList);
        _hDragImageList = NULL;
    }
}
