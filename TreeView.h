
/** $VER: TreeView.h (2026.08.13) P. Stuer **/

#pragma once

#define NOMINMAX

#include <SDKDDKVer.h>
#include <Windows.h>

class tree_view_t
{
public:
    tree_view_t() : _hTreeView(), _Id(), _IsAttached(false) { };

    tree_view_t(const tree_view_t &) = delete;
    tree_view_t(const tree_view_t &&) = delete;
    tree_view_t & operator=(const tree_view_t &) = delete;
    tree_view_t & operator=(tree_view_t &&) = delete;

    virtual ~tree_view_t() noexcept { };

    bool Create(HWND hWndParent, size_t id) noexcept;
    void Destroy() noexcept;

    tree_view_t(HWND hTreeView)
    {
        Attach(hTreeView);
    };

    void Attach(HWND hTreeView) noexcept;

    HWND Get() const noexcept
    {
        return _hTreeView;
    }

    HTREEITEM AddItem(HTREEITEM hParent, HTREEITEM hInsertAfter, UINT state, const void * data) const noexcept;

    bool GetText(HTREEITEM hItem, std::string & text) const noexcept;
    bool GetText(HTREEITEM hItem, std::wstring & text) const noexcept;
    bool GetState(HTREEITEM hItem, UINT & state) const noexcept;
    void * GetData(HTREEITEM hItem) const noexcept;

    bool SetText(HTREEITEM hItem, const std::string & text) const noexcept;
    bool SetState(HTREEITEM hItem, UINT state, UINT stateMask) const noexcept;

    bool SelectItem(HTREEITEM hTreeItem) const noexcept
    {
        if (!TreeView_SelectItem(_hTreeView, hTreeItem))
            return false;

        return EnsureVisible(hTreeItem);
    }

    bool EnsureVisible(HTREEITEM hTreeItem) const noexcept
    {
        return (bool) TreeView_EnsureVisible(_hTreeView, hTreeItem);
    }

    HTREEITEM GetSelectedItem() const noexcept
    {
        return TreeView_GetSelection(_hTreeView);
    }

    HTREEITEM GetDropHilight() const noexcept
    {
        return TreeView_GetDropHilight(_hTreeView);
    }

    HWND GetEditControl() const noexcept
    {
        return TreeView_GetEditControl(_hTreeView);
    }

    HTREEITEM GetHighlightedItem(const POINT & point) const noexcept;

    HTREEITEM GetParentItem(HTREEITEM hItem) const
    {
        return TreeView_GetParent(_hTreeView, hItem);
    }

    HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const
    {
        return TreeView_GetNextSibling(_hTreeView, hItem);
    }

    HTREEITEM GetPreviousSiblingItem(HTREEITEM hItem) const
    {
        return TreeView_GetPrevSibling(_hTreeView, hItem);
    }

    bool RemoveSelectedItem() const noexcept
    {
        return (bool) RemoveItem(GetSelectedItem());
    }

    bool RemoveItem(HTREEITEM hItem) const noexcept
    {
        return (bool) TreeView_DeleteItem(_hTreeView, hItem);
    }

    HWND EditSelectedItem() const noexcept
    {
        return EditItem(GetSelectedItem());
    }

    HWND EditItem(HTREEITEM hItem) const noexcept
    {
        return TreeView_EditLabel(_hTreeView, hItem);
    }

    bool DeleteAllItems() const noexcept
    {
        return (bool) TreeView_DeleteAllItems(_hTreeView);
    }

    bool ExpandItem(HTREEITEM hItem) const noexcept
    {
        return (bool) TreeView_Expand(_hTreeView, hItem, TVE_EXPAND);
    }

    void ExpandAll(HTREEITEM hItem) const noexcept
    {
        Walk(hItem, [&](HTREEITEM hItem, void * context) -> bool
        {
            ExpandItem(hItem);

            return true; // Continue walking.
        }, nullptr);
    }

    bool CollapseItem(HTREEITEM hItem) const noexcept
    {
        return (bool) TreeView_Expand(_hTreeView, hItem, TVE_COLLAPSE);
    }

    void CollapseAll(HTREEITEM hItem) const noexcept
    {
        Walk(hItem, [&](HTREEITEM hItem, void * context) -> bool
        {
            CollapseItem(hItem);

            return true; // Continue walking.
        }, nullptr);
    }

    bool ToggleItem(HTREEITEM hItem) const noexcept
    {
        return (bool) TreeView_Expand(_hTreeView, hItem, TVE_TOGGLE);
    }

    bool Sort(HTREEITEM hParent = TVI_ROOT) const noexcept
    {
        return (bool) TreeView_SortChildren(_hTreeView, hParent, FALSE);
    }

    HIMAGELIST SetNormalImageList(HIMAGELIST hImageList) const noexcept
    {
        return TreeView_SetImageList(_hTreeView, hImageList, TVSIL_NORMAL);
    }

    HIMAGELIST SetStateImageList(HIMAGELIST hImageList) const noexcept
    {
        return TreeView_SetImageList(_hTreeView, hImageList, TVSIL_STATE);
    }

    HIMAGELIST GetNormalImageList() const noexcept
    {
        return TreeView_GetImageList(_hTreeView, TVSIL_NORMAL);
    }

    HIMAGELIST GetStateImageList() const noexcept
    {
        return TreeView_GetImageList(_hTreeView, TVSIL_STATE);
    }

    void SetColors(COLORREF backgroundColor, COLORREF foregroundColor) const noexcept
    {
        TreeView_SetBkColor  (_hTreeView, backgroundColor);
        TreeView_SetTextColor(_hTreeView, foregroundColor);
    }

    void SetFont(HFONT hFont) const noexcept
    {
        ::SendMessageW(_hTreeView, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);
    }

    size_t GetChildCount(HTREEITEM hItem) const noexcept;

    virtual void MeasureDragImage(HTREEITEM hItem, RECT & rc) const noexcept { }
    virtual void DrawDragImage(HDC hDC, HTREEITEM hItem, const RECT & rc) const noexcept { }

    void Redraw() const noexcept;
    bool RedrawItem(HTREEITEM hItem) const noexcept;

    bool RefreshItem(HTREEITEM hItem) const noexcept;

    bool RefreshAllItems() const noexcept;

    enum DropZone
    {
        Unknown = -1,

        Top,
        Middle,
        Bottom
    };

    void MoveItem(HTREEITEM hParentItem, HTREEITEM hChildItem, DropZone dropZone) const noexcept;

    void BeginDrag(const NMTREEVIEWW * nmtv) noexcept;
    void DragMove(const POINT & point) noexcept;
    void EndDrag(bool cancel) noexcept;

    /// <summary>
    /// Removes the insert marker.
    /// </summary>
    bool RemoveInsertMarker() const noexcept
    {
        return (bool) TreeView_SetInsertMark(_hTreeView, NULL, FALSE);
    }

    /// <summary>
    /// Returns true if a drag & drop operation is ongoing.
    /// </summary>
    bool IsDragging() const noexcept
    {
        return (_hDragSource != NULL);
    }

    /// <summary>
    /// Recursively walks the tree starting from the root item.
    /// </summary>
    template<typename Visitor> bool Walk(Visitor && visitor, void * context = nullptr) const noexcept
    {
        // Visit the root and its siblings.
        HTREEITEM hItem = TreeView_GetRoot(_hTreeView);

        while (hItem != NULL)
        {
            if (!visitor(hItem, context))
                return false;

            if (!Walk(hItem, visitor, context))
                return false;

            hItem = TreeView_GetNextSibling(_hTreeView, hItem);
        }

        return true;
    }

    /// <summary>
    /// Recursively walks the branch starting with the specified item.
    /// </summary>
    template<typename Visitor> bool Walk(HTREEITEM hParent, Visitor && visitor, void * context = nullptr) const noexcept
    {
        HTREEITEM hItem = TreeView_GetChild(_hTreeView, hParent);

        while (hItem != NULL)
        {
            if (!visitor(hItem, context))
                return false;

            if (!Walk(hItem, visitor, context))
                return false;

            hItem = TreeView_GetNextSibling(_hTreeView, hItem);
        }

        return true;
    }

    /// <summary>
    /// Sends a notification.
    /// </summary>
    void SendNotification(UINT code) const noexcept
    {
        const NMHDR nmhd =
        {
            .hwndFrom = _hTreeView,
            .idFrom   = _Id,
            .code     = code,
        };

        ::SendMessageW(::GetParent(_hTreeView), WM_NOTIFY, 0, (LPARAM) &nmhd);
    }

    /// <summary>
    /// Sets the focus to the tree view.
    /// </summary>
    void SetFocus() const noexcept
    {
        ::SetFocus(_hTreeView);
    }

    /// <summary>
    /// Shows or hides the horizontal scrollbar.
    /// </summary>
    void SetHorizontalScrollbar(bool visible) const noexcept
    {
        const auto Style = ::GetWindowLongPtrW(_hTreeView, GWL_STYLE);

        auto NewStyle = Style;

        if (visible)
            NewStyle &= ~(LONG_PTR) TVS_NOHSCROLL;
        else
            NewStyle |=  (LONG_PTR) TVS_NOHSCROLL;

        if (NewStyle == Style)
            return;

        ::SetWindowLongPtrW(_hTreeView, GWL_STYLE, NewStyle);

        ::SetWindowPos(_hTreeView, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        // Work-around tree view bug when modifying the style.
        ::ShowScrollBar(_hTreeView, SB_HORZ, visible ? TRUE : FALSE);
        ::InvalidateRect(_hTreeView, nullptr, TRUE);
    }
/*
    // Remove the vertical scroll bar.
    {
        auto Style = ::GetWindowLongPtrW(_TreeView.Get(), GWL_STYLE) | TVS_NOSCROLL;

        ::SetWindowLongPtrW(_TreeView.Get(), GWL_STYLE, Style);

        ::SetWindowPos(_TreeView.Get(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
*/
    /// <summary>
    /// Enables or disables expanding the drop target after a drop.
    /// </summary>
    void SetExpandDropTarget(bool expandDropTarget) noexcept
    {
        _ExpandDropTarget = expandDropTarget;
    }

protected:
    virtual bool AllowDrop(DropZone dropZone) noexcept { return false; };

private:
    DropZone GetDropZone(const RECT & rc, const POINT & pt) const noexcept;
    HIMAGELIST CreateDragImage(HTREEITEM hItem, const RECT & rc) const noexcept;

protected:
    HTREEITEM _hDragSource = NULL;
    HTREEITEM _hDropTarget = NULL;
    bool _PlaceAfter = FALSE;
    bool _ExpandDropTarget = true;

private:
    HWND _hTreeView;
    size_t _Id;
    bool _IsAttached;

    HIMAGELIST _hDragImageList = NULL;
    DropZone _DropZone = DropZone::Unknown;
};
