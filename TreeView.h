
/** $VER: TreeView.h (2026.07.05) P. Stuer **/

#pragma once

#include <SDKDDKVer.h>

#define NOMINMAX

#include <Windows.h>
#include <functional>

#include "Node.h"

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

    /// <summary>
    /// Gets the item at the specified point.
    /// </summary>
    HTREEITEM GetItem(_In_ const POINT & point) noexcept
    {
        TVHITTESTINFO ht = { .pt = point };

        ::ScreenToClient(_hTreeView, &ht.pt);

        auto hDropTarget = TreeView_HitTest(_hTreeView, &ht);

        if (hDropTarget == NULL)
            return NULL;
/*
        RECT rcItem;

        TreeView_GetItemRect(_hTreeView, hDropTarget, &rcItem, TRUE);
        
        const auto Center = (rcItem.top + rcItem.bottom) / 2;

        if (point.y < Center - 4)
            _hDropTarget = TreeView_GetPrevSibling(_hTreeView, hDropTarget);    // Insert before drop target
        else
        if (point.y > Center + 4)
            _hDropTarget = TreeView_GetNextSibling(_hTreeView, hDropTarget);    // Insert after drop target
        else
            _hDropTarget = TVI_LAST;                                            // Add as child to drop target
*/
        return hDropTarget;
    }

    /// <summary>
    /// Gets the data associated with the item.
    /// </summary>
    void * GetData(_In_ HTREEITEM hItem) const noexcept
    {
        TVITEMW tvi
        {
            .mask = TVIF_PARAM,
            .hItem = hItem,
        };

        if (!TreeView_GetItem(_hTreeView, &tvi))
            return nullptr;

        return (node_t *) tvi.lParam;
    }

    HTREEITEM AddItem(_In_ HTREEITEM hParent, _In_ HTREEITEM hInsertAfter, _In_ int iconIndex, _In_ const node_t * node) const noexcept;
    bool RemoveItem(_In_ HTREEITEM hItem) const noexcept;

    void MoveItem(_In_ HTREEITEM hTreeItem, _In_ HTREEITEM hDropTarget) const noexcept;

    void BeginDrag(_In_ const NMTREEVIEW * nmtv) noexcept;
    void DragMove(_In_ const CPoint & point) noexcept;
    void EndDrag() noexcept;

    /// <summary>
    /// Recursively walks the tree starting from the root item.
    /// </summary>
    template<typename Visitor> bool Walk(_In_ Visitor && visitor) const noexcept
    {
        // Visit the root and its siblings.
        HTREEITEM hRoot = TreeView_GetRoot(_hTreeView);

        while (hRoot != NULL)
        {
            if (!Walk(hRoot, visitor))
                return false;

            hRoot = TreeView_GetNextSibling(_hTreeView, hRoot);
        }

        return true;
    }

    /// <summary>
    /// Recursively walks the branch starting with the specified item.
    /// </summary>
    template<typename Visitor> bool Walk(HTREEITEM hItem, _In_ Visitor && visitor) const noexcept
    {
        if (_hTreeView == NULL)
            return false;

        if (!visitor(hItem))
            return false;

        // Recurse into its children.
        HTREEITEM hChild = TreeView_GetChild(_hTreeView, hItem);

        while (hChild)
        {
            if (!Walk(hChild, visitor))
                return false;

            hChild = TreeView_GetNextSibling(_hTreeView, hChild);
        }

        return true;
    }

private:
    HWND _hTreeView;

    HIMAGELIST _hDragImageList = NULL;
    HTREEITEM _hDraggedItem = NULL;
    HTREEITEM _hDropTarget = NULL;

    const UINT Mask =  TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
};
