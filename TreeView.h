
/** $VER: TreeView.h (2026.07.08) P. Stuer **/

#pragma once

#define NOMINMAX

#include <SDKDDKVer.h>
#include <Windows.h>

class tree_view_t
{
public:
    tree_view_t() : _hTreeView() { };

    tree_view_t(const tree_view_t &) = delete;
    tree_view_t(const tree_view_t &&) = delete;
    tree_view_t & operator=(const tree_view_t &) = delete;
    tree_view_t & operator=(tree_view_t &&) = delete;

    virtual ~tree_view_t() noexcept { };

    bool Create(HWND hWndParent, size_t id) noexcept;
    void Destroy() noexcept;

    HWND Get() const noexcept
    {
        return _hTreeView;
    }

    void SetImageList(HIMAGELIST hImageList) const noexcept
    {
        TreeView_SetImageList(_hTreeView, hImageList, TVSIL_NORMAL);
        TreeView_SetImageList(_hTreeView, hImageList, TVSIL_STATE);
    }

    void SelectItem(HTREEITEM hTreeItem) const noexcept
    {
        if (TreeView_SelectItem(_hTreeView, hTreeItem) != TRUE)
            return;

        TreeView_EnsureVisible(_hTreeView, hTreeItem);
    }

    HTREEITEM GetSelectedItem() const noexcept
    {
        return TreeView_GetSelection(_hTreeView);
    }

    HTREEITEM GetItem(const POINT & point) noexcept;

    std::string GetText(HTREEITEM hItem) const noexcept;
    void * GetData(HTREEITEM hItem) const noexcept;

    void SetText(HTREEITEM hItem, const std::string & text) const noexcept;

    HTREEITEM AddItem(HTREEITEM hParent, HTREEITEM hInsertAfter, UINT state, const void * data) const noexcept;
    bool RemoveItem(HTREEITEM hItem) const noexcept;

    virtual void Clear() const noexcept;

    // Forces an update of the item.
    void RefreshItem(HTREEITEM hItem) const noexcept
    {
        RECT r;

        if (!TreeView_GetItemRect(_hTreeView, hItem, &r, FALSE))
            return;

        ::InvalidateRect(Get(), &r, TRUE);
    }

    enum DropZone
    {
        Unknown = -1,

        Top,
        Middle,
        Bottom
    };

    void MoveItem(HTREEITEM hParentItem, HTREEITEM hChildItem, DropZone dropZone) const noexcept;

    void BeginDrag(const NMTREEVIEW * nmtv) noexcept;
    void DragMove(const CPoint & point) noexcept;
    void EndDrag(bool cancel) noexcept;

    /// <summary>
    /// Removes the insert marker.
    /// </summary>
    void RemoveInsertMarker() const noexcept
    {
        TreeView_SetInsertMark(_hTreeView, NULL, FALSE);
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

        while (hItem)
        {
            if (!visitor(hItem, context))
                return false;

            if (!Walk(hItem, visitor, context))
                return false;

            hItem = TreeView_GetNextSibling(_hTreeView, hItem);
        }

        return true;
    }

private:
    /// <summary>
    /// Gets the item zone that contains the specified point.
    /// </summary>
    DropZone GetItemZone(const RECT & r, const POINT & pt) const noexcept
    {
        // Divide the item into 3 zones.
        const LONG ZoneHeight = (r.bottom - r.top) / 3;

        if (pt.y < r.top + ZoneHeight)
            return DropZone::Top;

        if (pt.y >= r.bottom - ZoneHeight)
            return DropZone::Bottom;

        return DropZone::Middle;
    }

private:
    HWND _hTreeView;

    HIMAGELIST _hDragImageList = NULL;
    HTREEITEM _hDragItem = NULL;

    HTREEITEM _hDropTarget = NULL;
    DropZone _DropZone = DropZone::Unknown;

    const UINT Mask =  TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
};
