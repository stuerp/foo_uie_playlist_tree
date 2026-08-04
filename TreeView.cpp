
/** $VER: TreeView.cpp (2026.08.04) P. Stuer **/

#include "pch.h"

#include "TreeView.h"
#include "Theme.h"

#pragma hdrstop

/// <summary>
/// Creates the control.
/// </summary>
bool tree_view_t::Create(HWND hWndParent, size_t id) noexcept
{
    _Id = id;

    constexpr DWORD Styles = WS_CHILD | WS_VISIBLE | WS_VSCROLL | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS | TVS_SHOWSELALWAYS | TVS_TRACKSELECT | TVS_INFOTIP; // | TVS_SINGLEEXPAND | TVS_FULLROWSELECT;
    constexpr DWORD ExStyles = TVS_EX_DOUBLEBUFFER;

    _hTreeView = ::CreateWindowExW(ExStyles, WC_TREEVIEW, L"", Styles, 0, 0, 0, 0, hWndParent, (HMENU) id, THIS_HINSTANCE, nullptr);

    if (_hTreeView == NULL)
        return false;

    return true;
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
/// Attaches an existing tree view control to this instance.
/// </summary>
void tree_view_t::Attach(HWND hTreeView) noexcept
{
    Destroy();

    _hTreeView = hTreeView;
    _Id = (size_t) ::GetWindowLongPtrW(_hTreeView, GWLP_ID);

    _IsAttached = false;
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
            .mask           = TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
            .state          = state,
            .stateMask      = 0xFF,
            .pszText        = LPSTR_TEXTCALLBACKW,
            .iImage         = I_IMAGECALLBACK,
            .iSelectedImage = I_IMAGECALLBACK,
            .lParam         = (LPARAM) data,
        },
    };

    auto hTreeItem = TreeView_InsertItem(_hTreeView, &tvis);

    return hTreeItem;
}

/// <summary>
/// Returns the number of direct children of an item.
/// </summary>
size_t tree_view_t::GetChildCount(HTREEITEM hItem) const noexcept
{
    auto hChild = TreeView_GetChild(_hTreeView, hItem);

    if (hChild == NULL)
        return SIZE_MAX;

    size_t Count = 0;

    while (hChild != 0)
    {
        ++Count;

        hChild = TreeView_GetNextSibling(_hTreeView, hChild);
    }

    return Count;
}

/// <summary>
/// Redraws the control.
/// </summary>
void tree_view_t::Redraw() const noexcept
{
    ::InvalidateRect(_hTreeView, nullptr, FALSE);
    ::UpdateWindow(_hTreeView);
}

/// <summary>
/// Redraws an item.
/// </summary>
bool tree_view_t::RedrawItem(HTREEITEM hItem) const noexcept
{
    RECT r;

    if (!TreeView_GetItemRect(_hTreeView, hItem, &r, FALSE))
        return false;

    return (bool) ::InvalidateRect(Get(), &r, TRUE);
}

/// <summary>
/// Refreshes the specified item.
/// </summary>
bool tree_view_t::RefreshItem(HTREEITEM hItem) const noexcept
{
    const TVITEMEXW tvi =
    {
        .mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, // | TVIF_CHILDREN,
        .hItem          = hItem,
        .pszText        = LPSTR_TEXTCALLBACK,
        .iImage         = I_IMAGECALLBACK,
        .iSelectedImage = I_IMAGECALLBACK,
//      .cChildren      = I_CHILDRENCALLBACK
    };

    return (bool) TreeView_SetItem(_hTreeView, &tvi);
}

/// <summary>
/// Refreshes all items.
/// </summary>
bool tree_view_t::RefreshAllItems() const noexcept
{
    return (bool) ::RedrawWindow(_hTreeView, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
/*
    auto hItem = TreeView_GetRoot(_hTreeView);

    while (hItem != NULL)
    {
        RefreshItem(hItem);

        auto hChild = TreeView_GetChild(_hTreeView, hItem);

        if (hChild != NULL)
            hItem = hChild;
        else
        {
            while (hItem != NULL)
            {
                auto hNext = TreeView_GetNextSibling(_hTreeView, hItem);

                if (hNext != NULL)
                {
                    hItem = hNext;
                    break;
                }

                hItem = TreeView_GetParent(_hTreeView, hItem);
            }
        }
    }

    return (::InvalidateRect(_hTreeView, nullptr, TRUE) != 0);
*/
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
            .mask       = TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
            .hItem      = hChildItem,
            .stateMask  = TVIS_EXPANDED | TVIS_EXPANDEDONCE | TVIS_EXPANDPARTIAL,
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
HTREEITEM tree_view_t::GetHighlightedItem(const POINT & pt) const noexcept
{
    TVHITTESTINFO ht = { .pt = pt };

    ::ScreenToClient(_hTreeView, &ht.pt);

    auto hItem = TreeView_HitTest(_hTreeView, &ht);

    return hItem;
}

/// <summary>
/// Gets the text of the specified item.
/// </summary>
bool tree_view_t::GetText(HTREEITEM hItem, std::string & text) const noexcept
{
    std::wstring Text;

    if (!GetText(hItem, Text))
        return false;

    text = msc::WideToUTF8(Text);

    return true;
}

/// <summary>
/// Gets the text of the specified item.
/// </summary>
bool tree_view_t::GetText(HTREEITEM hItem, std::wstring & text) const noexcept
{
    text.resize(256);

    TVITEMEXW tvi =
    {
        .mask       = TVIF_TEXT,
        .hItem      = hItem,
        .pszText    = (LPWSTR) text.c_str(),
        .cchTextMax = (int) text.size(),
    };

    if (!TreeView_GetItem(_hTreeView, &tvi))
        return false;

    return true;
}

/// <summary>
/// Sets the text of the specified item.
/// </summary>
bool tree_view_t::SetText(HTREEITEM hItem, const std::string & text) const noexcept
{
    std::wstring Text = msc::UTF8ToWide(text.c_str());

    const TVITEMEXW tvi =
    {
        .mask    = TVIF_TEXT,
        .hItem   = hItem,
        .pszText = (LPWSTR) Text.c_str(),
    };

    return (bool) TreeView_SetItem(_hTreeView, &tvi);
}

/// <summary>
/// Gets the state of the specified item.
/// </summary>
bool tree_view_t::GetState(HTREEITEM hItem, UINT & state) const noexcept
{
    TVITEMEXW tvi =
    {
        .mask      = TVIF_STATE,
        .hItem     = hItem,
        .stateMask = 0xFF,
    };

    if (!TreeView_GetItem(_hTreeView, &tvi))
        return false;

    state = tvi.state; 

    return true;
}

/// <summary>
/// Sets the state of the specified item.
/// </summary>
bool tree_view_t::SetState(HTREEITEM hItem, UINT state, UINT stateMask) const noexcept
{
    TVITEMEXW tvi =
    {
        .mask      = TVIF_STATE,
        .hItem     = hItem,
        .state     = state,
        .stateMask = stateMask,
    };

    if (!TreeView_SetItem(_hTreeView, &tvi))
        return false;

    return true;
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
void tree_view_t::BeginDrag(const NMTREEVIEWW * nmtv) noexcept
{
    TreeView_SetInsertMarkColor(_hTreeView, _Theme.GetWindowTextColor());

    const auto & hItem = nmtv->itemNew.hItem;

    // Measure the drag image.
    RECT rcDrag;

    MeasureDragImage(hItem, rcDrag);

    // Create the drag image.
    _hDragImageList = CreateDragImage(hItem, rcDrag);

    if (_hDragImageList == NULL)
        return;

    const int dx = nmtv->ptDrag.x - rcDrag.left;
    const int dy = nmtv->ptDrag.y - rcDrag.top;

    // Begin the drag operation.
    if (!::ImageList_BeginDrag(_hDragImageList, 0, dx, dy))
        return;

    // Lock the tree view.
    if (!::ImageList_DragEnter(_hTreeView, nmtv->ptDrag.x, nmtv->ptDrag.y))
        return;

//  ::ShowCursor(FALSE);
    ::SetCapture(::GetParent(_hTreeView));

    _hDragSource = nmtv->itemNew.hItem;
}

/// <summary>
/// Moves the drag item.
/// </summary>
void tree_view_t::DragMove(const POINT & point) noexcept
{
    if (_hDragSource == NULL)
        return;

    POINT pt = point;

    ::ClientToScreen(::GetParent(_hTreeView), &pt);
    ::ScreenToClient(_hTreeView, &pt);

    // Hide the drag image.
    ::ImageList_DragShowNolock(FALSE);

    // Remove the insertion marker.
    TreeView_SetInsertMark(_hTreeView, NULL, _PlaceAfter);

    // Determine the drop target and highlight it.
    const TVHITTESTINFO tvhi = { .pt = pt };

    const auto hHitItem = TreeView_HitTest(_hTreeView, &tvhi);

    if (hHitItem != NULL)
    {
        if ((hHitItem != _hDropTarget) || (_hDropTarget == NULL))
        {
            TreeView_SelectDropTarget(_hTreeView, hHitItem);

             _hDropTarget = hHitItem;
        }
    }

    if ((_hDropTarget != NULL) && (_hDropTarget != _hDragSource))
    {
        RECT rc = { };

        if (TreeView_GetItemRect(_hTreeView, _hDropTarget, &rc, TRUE))
        {
            _DropZone = GetDropZone(rc, pt);

            if (_DropZone != DropZone::Middle)
            {
                _PlaceAfter = (_DropZone == DropZone::Bottom) ? TRUE : FALSE;

                // Add the insertion marker.
                TreeView_SetInsertMark(_hTreeView, _hDropTarget, _PlaceAfter);
            }
        }
    }

    // Show the drag image.
    ::ImageList_DragShowNolock(TRUE);

    // Move the drag image.
    ::ImageList_DragMove(pt.x, pt.y);

    ::SetCursor(::LoadCursorW(NULL, (hHitItem != _hDragSource) ? IDC_ARROW : IDC_NO));

    // Scroll the treeview up or down if necessary.
    {
        SCROLLINFO si =
        {
            .cbSize = sizeof(si),
            .fMask = SIF_PAGE | SIF_POS | SIF_RANGE
        };

        if (::GetScrollInfo(_hTreeView, SB_VERT, &si))
        {
            if ((tvhi.flags == TVHT_BELOW) && (si.nPos < (si.nMax - std::max((int) si.nPage - 1, 0))))
            {
                ::SendMessageW(_hTreeView, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0L);

                RefreshAllItems();
            }
            else
            if ((tvhi.flags == TVHT_ABOVE) && (si.nPos > si.nMin))
            {
                ::SendMessageW(_hTreeView, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0L);

                RefreshAllItems();
            }
        }
    }
}

/// <summary>
/// Ends the drag operation.
/// </summary>
void tree_view_t::EndDrag(bool isDragCancelled) noexcept
{
    if (_hDragSource == NULL)
        return;

    // Unlock the tree view.
    ::ImageList_DragLeave(_hTreeView);

    // Stop the drag operation.
    ::ImageList_EndDrag();

    // Remove the insertion marker.
    TreeView_SetInsertMark(_hTreeView, NULL, FALSE);

    // Move the item and its children. If hDropTarget is NULL the node gets added to the end of the tree.
    if (((_hDropTarget == NULL) || ((_hDropTarget != NULL) && (_hDropTarget != _hDragSource) && AllowDrop(_DropZone))) && !isDragCancelled)
    {
        MoveItem(_hDropTarget, _hDragSource, _DropZone);

        TreeView_Expand(_hTreeView, _hDropTarget, TVE_EXPAND);
    }

    // Remove the drop target highlight.
    TreeView_SelectDropTarget(_hTreeView, NULL);

    // Destroy the drag image.
    if (_hDragImageList != NULL)
    {
        ::ImageList_Destroy(_hDragImageList);
        _hDragImageList = NULL;
    }

    ::ReleaseCapture();
//  ::ShowCursor(TRUE);
    ::SetCursor(::LoadCursorW(NULL, IDC_ARROW));

    _hDragSource = NULL;

    _hDropTarget = NULL;
    _DropZone = DropZone::Unknown;
}

/// <summary>
/// Gets the drop zone that contains the specified point.
/// </summary>
tree_view_t::DropZone tree_view_t::GetDropZone(const RECT & rc, const POINT & pt) const noexcept
{
    // Divide the item into 3 zones, the middle zone being twice as high.
    const float ZoneHeight = (float) (rc.bottom - rc.top) / 4.f;

    if ((float) pt.y < (float) rc.top + ZoneHeight)
        return DropZone::Top;

    if ((float) pt.y >= (float) rc.bottom - ZoneHeight)
        return DropZone::Bottom;

    return DropZone::Middle;
}

/// <summary>
/// Creates a drag image.
/// </summary>
HIMAGELIST tree_view_t::CreateDragImage(HTREEITEM hItem, const RECT & rc) const noexcept
{
    HIMAGELIST hImageList = NULL;

    const LONG Width  = rc.right  - rc.left;
    const LONG Height = rc.bottom - rc.top;

    const HDC hDCScreen = ::GetDC(NULL);

    const BITMAPINFO bmi =
    {
        .bmiHeader =
        {
            .biSize        = sizeof(BITMAPINFOHEADER),
            .biWidth       = Width,
            .biHeight      = -Height,
            .biPlanes      = 1,
            .biBitCount    = 24, // Don't use 32bpp
            .biCompression = BI_RGB,
        }
    };

    void * Bits = nullptr;

    const auto hBitmap = ::CreateDIBSection(hDCScreen, &bmi, DIB_RGB_COLORS, &Bits, NULL, 0);

    if ((hBitmap != NULL) && (Bits != nullptr))
    {
        const HDC hDCMem = ::CreateCompatibleDC(hDCScreen);

        if (hDCMem != NULL)
        {
            const auto hOldBitmap = ::SelectObject(hDCMem, hBitmap);

            DrawDragImage(hDCMem, hItem, rc);

            ::SelectObject(hDCMem, hOldBitmap);

            ::DeleteDC(hDCMem);

            {
                hImageList = ::ImageList_Create(Width, Height, ILC_COLOR32 | ILC_MASK, 1, 0);

                ::ImageList_Add(hImageList, hBitmap, nullptr);

                ::DeleteObject(hBitmap);
            }
        }
    }
    else
        hImageList = TreeView_CreateDragImage(_hTreeView, hItem); // Fall-back

    ::ReleaseDC(NULL, hDCScreen);

    return hImageList;
}
