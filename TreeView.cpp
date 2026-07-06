
/** $VER: TreeView.cpp (2026.07.06) P. Stuer **/

#include "pch.h"

#include "TreeView.h"
#include "Log.h"

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
HTREEITEM tree_view_t::AddItem(_In_ HTREEITEM hParent, _In_ HTREEITEM hInsertAfter, _In_ const wchar_t * text, _In_ int iconIndex, _In_ const void * data) const noexcept
{
    const TVINSERTSTRUCTW tvis
    {
        .hParent      = hParent,
        .hInsertAfter = hInsertAfter,
        .item         =
        {
            .mask           = Mask,
            .pszText        = (LPWSTR) text,
            .iImage         = iconIndex,
            .iSelectedImage = iconIndex,
            .lParam         = (LPARAM) data,
        },
    };

    auto hTreeItem = TreeView_InsertItem(_hTreeView, &tvis);

    return hTreeItem;
}

/// <summary>
/// Removes the specified item from the tree.
/// </summary>
bool tree_view_t::RemoveItem(_In_ HTREEITEM hItem) const noexcept
{
    return (TreeView_DeleteItem(_hTreeView, hItem) == TRUE);
}

/// <summary>
/// Moves an item.
/// </summary>
void tree_view_t::MoveItem(_In_ HTREEITEM hPivotItem, _In_ HTREEITEM hChildItem, _In_ DropZone dropZone) const noexcept
{
    wchar_t Text[512] = { };

    TVINSERTSTRUCTW tvis = 
    {
        .item =
        {
            .mask       = Mask,
            .hItem      = hChildItem,
            .pszText    = Text,
            .cchTextMax = _countof(Text),
        }
    };

    switch (dropZone)
    {
        case DropZone::Top:
        {
            tvis.hParent      = TreeView_GetParent(_hTreeView, hPivotItem);
            tvis.hInsertAfter = TreeView_GetPrevSibling(_hTreeView, hPivotItem);

            if (tvis.hInsertAfter == NULL)
                tvis.hInsertAfter = TVI_FIRST;
            break;
        }

        case DropZone::Middle:
        {
            tvis.hParent      = hPivotItem;
            tvis.hInsertAfter = TVI_LAST;
            break;
        }

        case DropZone::Bottom:
        {
            tvis.hParent      = TreeView_GetParent(_hTreeView, hPivotItem);
            tvis.hInsertAfter = hPivotItem;
            break;
        }

        case DropZone::Unknown:
            return;
    }

    TreeView_GetItem(_hTreeView, &tvis.item);

    // Insert a new version of the item.
    HTREEITEM hNew = TreeView_InsertItem(_hTreeView, &tvis);

    if (hNew == NULL)
        return;

    // Recursively move the children.
    HTREEITEM hChild = TreeView_GetChild(_hTreeView, hChildItem);

    while (hChild != NULL)
    {
        HTREEITEM hNextChild = TreeView_GetNextSibling(_hTreeView, hChild);

        MoveItem(hNew, hChild, DropZone::Middle);

        hChild = hNextChild;
    }

    // Delete the original item.
    TreeView_DeleteItem(_hTreeView, hChildItem);
}

/// <summary>
/// Gets the item at the specified point.
/// </summary>
HTREEITEM tree_view_t::GetItem(_In_ const POINT & point) noexcept
{
    TVHITTESTINFO ht = { .pt = point };

    ::ScreenToClient(_hTreeView, &ht.pt);

    auto hTreeItem = TreeView_HitTest(_hTreeView, &ht);

    return hTreeItem;
}

/// <summary>
/// Gets the text of the specified item.
/// </summary>
std::wstring tree_view_t::GetText(_In_ HTREEITEM hItem) const noexcept
{
    std::wstring Text;

    Text.resize(256);

    TVITEMEXW tvix =
    {
        .mask    = TVIF_TEXT,
        .hItem   = hItem,
        .pszText = (LPWSTR) Text.c_str(),
    };

    if (!TreeView_GetItem(_hTreeView, &tvix))
        return { };

    return Text;
}

/// <summary>
/// Sets the text of the specified item.
/// </summary>
void tree_view_t::SetText(_In_ HTREEITEM hItem, _In_ const std::wstring & newName) const noexcept
{
    TVITEMEXW tvix =
    {
        .mask    = TVIF_TEXT,
        .hItem   = hItem,
        .pszText = (LPWSTR) newName.c_str(),
    };

    TreeView_SetItem(_hTreeView, &tvix);
}

/// <summary>
/// Gets the data associated with the item.
/// </summary>
void * tree_view_t::GetData(_In_ HTREEITEM hItem) const noexcept
{
    TVITEMEXW tvi
    {
        .mask = TVIF_PARAM,
        .hItem = hItem,
    };

    if (!TreeView_GetItem(_hTreeView, &tvi))
        return nullptr;

    return (void *) tvi.lParam;
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

    if (!TreeView_GetItemRect(_hTreeView, nmtv->itemNew.hItem, &rcItem, TRUE))
        return;

    if (!::ImageList_BeginDrag(_hDragImageList, 0, 0, 0))
        return;

    if (!::ImageList_DragEnter(_hTreeView, nmtv->ptDrag.x, nmtv->ptDrag.y))
        return;

    ::ShowCursor(FALSE);
    ::SetCapture(::GetParent(_hTreeView));

    _hDragItem = nmtv->itemNew.hItem;
}

/// <summary>
/// Moves the drag item.
/// </summary>
void tree_view_t::DragMove(_In_ const CPoint & point) noexcept
{
    if (_hDragItem == NULL)
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

        TreeView_SetInsertMark(_hTreeView, NULL, FALSE); // Remove the insertion marker.

        const TVHITTESTINFO tvht = { .pt = pt };

        const HTREEITEM hTreeItem = TreeView_HitTest(_hTreeView, &tvht);

        if ((hTreeItem != NULL) && (hTreeItem != _hDragItem))
        {
            TreeView_SelectDropTarget(_hTreeView, hTreeItem);

            RECT rc = { };

            if (TreeView_GetItemRect(_hTreeView, hTreeItem, &rc, TRUE))
            {
                _DropZone = GetItemZone(rc, pt);

                if (_DropZone != DropZone::Middle)
                {
                    const BOOL PlaceAfter = (_DropZone == DropZone::Bottom) ? TRUE : FALSE;

                    TreeView_SetInsertMark(_hTreeView, hTreeItem, PlaceAfter); // Add the insertion marker:
                }
            }

            _hDropTarget = hTreeItem;
        }

        ::ImageList_DragShowNolock(TRUE);
    }
}

/// <summary>
/// Ends the drag operation.
/// </summary>
void tree_view_t::EndDrag(_In_ bool cancel) noexcept
{
    if (_hDragItem == NULL)
        return;

    ::ImageList_EndDrag();

    TreeView_SetInsertMark(_hTreeView, NULL, FALSE); // Remove the insertion marker.

    if ((_hDropTarget != NULL) && (_hDropTarget != _hDragItem) && !cancel)
        MoveItem(_hDropTarget, _hDragItem, _DropZone);

    TreeView_SelectDropTarget(_hTreeView, NULL); // Remove the drop target highlight.

    ::ReleaseCapture();
    ::ShowCursor(TRUE);

    _hDragItem = NULL;

    _hDropTarget = NULL;
    _DropZone = DropZone::Unknown;

    if (_hDragImageList != NULL)
    {
        ::ImageList_Destroy(_hDragImageList);
        _hDragImageList = NULL;
    }
}
