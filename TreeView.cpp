
/** $VER: TreeView.cpp (2026.07.11) P. Stuer **/

#include "pch.h"

#include "TreeView.h"
#include "Theme.h"

#pragma hdrstop

/// <summary>
/// Creates the control.
/// </summary>
bool tree_view_t::Create(HWND hWndParent, size_t id) noexcept
{
    const DWORD Styles = WS_CHILD | WS_VISIBLE | WS_VSCROLL | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS | TVS_SHOWSELALWAYS;// | TVS_TRACKSELECT | TVS_SINGLEEXPAND | TVS_INFOTIP | TVS_FULLROWSELECT;

    _hTreeView = ::CreateWindowExW(0, WC_TREEVIEW, L"", Styles, 0, 0, 0, 0, hWndParent, (HMENU) id, THIS_HINSTANCE, nullptr);

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
HTREEITEM tree_view_t::AddItem(HTREEITEM hParent, HTREEITEM hInsertAfter, UINT state, const void * data) const noexcept
{
    const TVINSERTSTRUCTW tvis
    {
        .hParent      = hParent,
        .hInsertAfter = hInsertAfter,
        .item         =
        {
            .mask           = Mask,
            .state          = state,
            .pszText        = LPSTR_TEXTCALLBACKW,
            .iImage         = I_IMAGECALLBACK,
            .iSelectedImage = I_IMAGECALLBACK,
            .lParam         = (LPARAM) data,
        },
    };

    auto hTreeItem = TreeView_InsertItem(_hTreeView, &tvis);

    return hTreeItem;
}

// <summary>
/// Removes the specified item from the tree.
/// </summary>
bool tree_view_t::RemoveItem(HTREEITEM hItem) const noexcept
{
    return (TreeView_DeleteItem(_hTreeView, hItem) == TRUE);
}

/// <summary>
/// Removes all items.
/// </summary>
void tree_view_t::Clear() const noexcept
{
    TreeView_DeleteAllItems(_hTreeView);
}

/// <summary>
/// Moves an item.
/// </summary>
void tree_view_t::MoveItem(HTREEITEM hPivotItem, HTREEITEM hChildItem, DropZone dropZone) const noexcept
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
HTREEITEM tree_view_t::GetItem(const POINT & pt) const noexcept
{
    TVHITTESTINFO ht = { .pt = pt };

    ::ScreenToClient(_hTreeView, &ht.pt);

    auto hItem = TreeView_HitTest(_hTreeView, &ht);

    return hItem;
}

/// <summary>
/// Gets the text of the specified item.
/// </summary>
std::string tree_view_t::GetText(HTREEITEM hItem) const noexcept
{
    std::wstring Text;

    Text.resize(256);

    TVITEMEXW tvix =
    {
        .mask       = TVIF_TEXT,
        .hItem      = hItem,
        .pszText    = (LPWSTR) Text.c_str(),
        .cchTextMax = (int) Text.size(),
    };

    if (!TreeView_GetItem(_hTreeView, &tvix))
        return { };

    return msc::WideToUTF8(Text);
}

/// <summary>
/// Sets the text of the specified item.
/// </summary>
void tree_view_t::SetText(HTREEITEM hItem, const std::string & text) const noexcept
{
    std::wstring Text = msc::UTF8ToWide(text.c_str());

    TVITEMEXW tvix =
    {
        .mask    = TVIF_TEXT,
        .hItem   = hItem,
        .pszText = (LPWSTR) Text.c_str(),
    };

    TreeView_SetItem(_hTreeView, &tvix);
}

/// <summary>
/// Gets the data associated with the item.
/// </summary>
void * tree_view_t::GetData(HTREEITEM hItem) const noexcept
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
void tree_view_t::BeginDrag(const NMTREEVIEW * nmtv) noexcept
{
    TreeView_SetInsertMarkColor(_hTreeView, _Theme.GetColor(COLOR_WINDOWTEXT));

    // Create the drag image.
    _hDragImageList = TreeView_CreateDragImage(_hTreeView, nmtv->itemNew.hItem);

    if (_hDragImageList == NULL)
        return;

    // Begin the drag operation.
    if (!::ImageList_BeginDrag(_hDragImageList, 0, 0, 0))
        return;

    // Loch the tree view.
    if (!::ImageList_DragEnter(_hTreeView, nmtv->ptDrag.x, nmtv->ptDrag.y))
        return;

    ::ShowCursor(FALSE);
    ::SetCapture(::GetParent(_hTreeView));

    _hDragItem = nmtv->itemNew.hItem;
}

/// <summary>
/// Moves the drag item.
/// </summary>
void tree_view_t::DragMove(const CPoint & point) noexcept
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
                _DropZone = GetDropZone(rc, pt);

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
void tree_view_t::EndDrag(bool cancel) noexcept
{
    if (_hDragItem == NULL)
        return;

    // Unlock the tree view.
    ::ImageList_DragLeave(_hTreeView);

    // Stop the drag operation.
    ::ImageList_EndDrag();

    // Remove the insertion marker.
    TreeView_SetInsertMark(_hTreeView, NULL, FALSE);

    // Move the item and its children.
    if ((_hDropTarget != NULL) && (_hDropTarget != _hDragItem) && !cancel)
        MoveItem(_hDropTarget, _hDragItem, _DropZone);

    // Remove the drop target highlight.
    TreeView_SelectDropTarget(_hTreeView, NULL);

    // Destroy the drag image.
    if (_hDragImageList != NULL)
    {
        ::ImageList_Destroy(_hDragImageList);
        _hDragImageList = NULL;
    }

    ::ReleaseCapture();
    ::ShowCursor(TRUE);

    _hDragItem = NULL;

    _hDropTarget = NULL;
    _DropZone = DropZone::Unknown;
}
