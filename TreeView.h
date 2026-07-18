
/** $VER: TreeView.h (2026.07.16) P. Stuer **/

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

    HIMAGELIST SetNormalImageList(HIMAGELIST hImageList) const noexcept
    {
        return TreeView_SetImageList(_hTreeView, hImageList, TVSIL_NORMAL);
    }

    HIMAGELIST SetStateImageList(HIMAGELIST hImageList) const noexcept
    {
        return TreeView_SetImageList(_hTreeView, hImageList, TVSIL_STATE);
    }

    bool SelectItem(HTREEITEM hTreeItem) const noexcept
    {
        if (TreeView_SelectItem(_hTreeView, hTreeItem) != TRUE)
            return false;

        TreeView_EnsureVisible(_hTreeView, hTreeItem);

        return true;
    }

    /// <summary>
    /// Gets the selected item.
    /// </summary>
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

    HTREEITEM GetItem(const POINT & point) const noexcept;

    bool GetText(HTREEITEM hItem, std::string & text) const noexcept;
    bool GetState(HTREEITEM hItem, UINT & state) const noexcept;
    void * GetData(HTREEITEM hItem) const noexcept;

    void SetText(HTREEITEM hItem, const std::string & text) const noexcept;
    bool SetState(HTREEITEM hItem, UINT state, UINT stateMask = 0xFF) const noexcept;

    HTREEITEM AddItem(HTREEITEM hParent, HTREEITEM hInsertAfter, UINT state, const void * data) const noexcept;

    bool RemoveSelectedItem() const noexcept
    {
        return RemoveItem(GetSelectedItem());
    }

    bool RemoveItem(HTREEITEM hItem) const noexcept
    {
        return (TreeView_DeleteItem(_hTreeView, hItem) == TRUE);
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
        return (TreeView_DeleteAllItems(_hTreeView) == TRUE);
    }

    bool ExpandItem(HTREEITEM hItem) const noexcept
    {
        return (TreeView_Expand(_hTreeView, hItem, TVE_EXPAND) != 0);
    }

    bool CollapseItem(HTREEITEM hItem) const noexcept
    {
        return (TreeView_Expand(_hTreeView, hItem, TVE_COLLAPSE) != 0);
    }

    bool ToggleItem(HTREEITEM hItem) const noexcept
    {
        return (TreeView_Expand(_hTreeView, hItem, TVE_TOGGLE) != 0);
    }

    bool Sort(HTREEITEM hParent = TVI_ROOT) const noexcept
    {
        return (TreeView_SortChildren(_hTreeView, hParent, FALSE) == TRUE);
    }

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

protected:
    virtual bool AllowDrop(DropZone dropZone) noexcept = 0;

private:
    DropZone GetDropZone(const RECT & r, const POINT & pt) const noexcept;

protected:
    HTREEITEM _hDragSource = NULL;
    HTREEITEM _hDropTarget = NULL;

private:
    HWND _hTreeView;

    HIMAGELIST _hDragImageList = NULL;
    DropZone _DropZone = DropZone::Unknown;
};
