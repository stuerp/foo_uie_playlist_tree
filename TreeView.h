
/** $VER: TreeView.h (2026.07.07) P. Stuer **/

#pragma once

#include <SDKDDKVer.h>

#define NOMINMAX

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

    bool Create(_In_ HWND hWndParent, _In_ size_t id) noexcept;
    void Destroy() noexcept;

    HWND Get() const noexcept
    {
        return _hTreeView;
    }

    void SetImageList(_In_ HIMAGELIST hImageList) const noexcept
    {
        TreeView_SetImageList(_hTreeView, hImageList, TVSIL_NORMAL);
        TreeView_SetImageList(_hTreeView, hImageList, TVSIL_STATE);
    }

    void SelectItem(_In_ HTREEITEM hTreeItem) const noexcept
    {
        if (TreeView_SelectItem(_hTreeView, hTreeItem) != TRUE)
            return;

        TreeView_EnsureVisible(_hTreeView, hTreeItem);
    }

    HTREEITEM GetSelectedItem() const noexcept
    {
        return TreeView_GetSelection(_hTreeView);
    }

    HTREEITEM GetItem(_In_ const POINT & point) noexcept;

    std::string GetText(_In_ HTREEITEM hItem) const noexcept;
    void * GetData(_In_ HTREEITEM hItem) const noexcept;

    void SetText(_In_ HTREEITEM hItem, _In_ const std::string & text) const noexcept;

    HTREEITEM AddItem(_In_ HTREEITEM hParent, _In_ HTREEITEM hInsertAfter, _In_ const std::string & text, _In_ int iconIndex, _In_ const void * data) const noexcept;
    bool RemoveItem(_In_ HTREEITEM hItem) const noexcept;

    virtual void Clear() const noexcept;

    enum DropZone
    {
        Unknown = -1,

        Top,
        Middle,
        Bottom
    };

    void MoveItem(_In_ HTREEITEM hParentItem, _In_ HTREEITEM hChildItem, _In_ DropZone dropZone) const noexcept;

    void BeginDrag(_In_ const NMTREEVIEW * nmtv) noexcept;
    void DragMove(_In_ const CPoint & point) noexcept;
    void EndDrag(_In_ bool cancel) noexcept;

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
    template<typename Visitor> bool Walk(_In_ Visitor && visitor, void * context = nullptr) const noexcept
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
    template<typename Visitor> bool Walk(HTREEITEM hParent, _In_ Visitor && visitor, void * context = nullptr) const noexcept
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
    DropZone GetItemZone(_In_ const RECT & r, _In_ const POINT & pt) const noexcept
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
