
/** $VER: PlaylistsTreeView.cpp (2026.07.30) P. Stuer **/

#include "pch.h"

#include "PlaylistTreeView.h"

#pragma hdrstop

/// <summary>
/// Gets the text of the specified item.
/// </summary>
bool playlist_tree_view_t::GetText(const GUID & id, std::string & text) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    return __super::GetText(hItem, text);
}

/// <summary>
/// Sets the name of the specified item.
/// </summary>
bool playlist_tree_view_t::SetName(const GUID & id, const std::string & name) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    auto Node = (node_t *) GetData(hItem);

    if (Node == nullptr)
        return false;

    Node->Name = name;

    RedrawItem(hItem);

    return true;
}

/// <summary>
/// Adds an item.
/// </summary>
node_t * playlist_tree_view_t::AddItem(const GUID & parentId, const GUID & insertAfterId, const GUID & id, const std::string & name, bool isFolder, bool isExpanded) const noexcept
{
    auto Node = new node_t(id, name, isFolder);

    HTREEITEM hParent = FindItem(parentId);

    if (hParent == NULL)
        hParent = TVI_ROOT;

    HTREEITEM hInsertAfter = FindItem(insertAfterId);

    if (hInsertAfter == NULL)
        hInsertAfter = TVI_LAST;

    const UINT State = isExpanded ? TVIS_EXPANDED : 0;

    auto hNewItem = __super::AddItem(hParent, hInsertAfter, State, Node);

    if (hNewItem == NULL)
    {
        delete Node;

        return nullptr;
    }

    return Node;
}

/// <summary>
/// Removes the specified item.
/// </summary>
bool playlist_tree_view_t::RemoveItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    return __super::RemoveItem(hItem);
}

/// <summary>
/// Selects the specified id.
/// </summary>
bool playlist_tree_view_t::SelectItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    return SelectItem(hItem);
}

/// <summary>
/// Selects the item with the specified name.
/// </summary>
bool playlist_tree_view_t::SelectItem(const std::string & name) const noexcept
{
    bool IsSelected = false;

    __super::Walk([&](HTREEITEM hItem, void * context) -> bool
    {
        auto Node = (const node_t *) GetData(hItem);

        if ((Node != nullptr) && (Node->Name == name))
        {
            IsSelected = SelectItem(hItem);

            return false;
        }

        return true; // Continue walking.
    });

    return IsSelected;
}

/// <summary>
/// Find the item with the specified id.
/// </summary>
HTREEITEM playlist_tree_view_t::FindItem(const GUID & id) const noexcept
{
    if (id == GUID_NULL)
        return NULL;

    HTREEITEM hFoundItem = NULL;

    __super::Walk([&](HTREEITEM hItem, void * context) -> bool
    {
        auto Node = (const node_t *) GetData(hItem);

        if ((Node != nullptr) && (Node->Id == id))
        {
            hFoundItem = hItem;

            return false;
        }

        return true; // Continue walking
    });

    return hFoundItem;
}

/// <summary>
/// Gets the item at the specified point.
/// </summary>
HTREEITEM playlist_tree_view_t::GetHighlightedItem(const POINT & pt) const noexcept
{
    auto hItem = __super::GetHighlightedItem(pt);

    return hItem;
}

/// <summary>
/// Gets the selected item.
/// </summary>
node_t * playlist_tree_view_t::GetSelectedItem() const noexcept
{
    auto hItem = __super::GetSelectedItem();

    if (hItem == NULL)
        return nullptr;

    auto Node = (node_t *) GetData(hItem);

    return Node;
}

/// <summary>
/// Refreshes the specified item.
/// </summary>
bool playlist_tree_view_t::RefreshItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    __super::RefreshItem(hItem);

    return true;
}

/// <summary>
/// Returns true if the specified item is expanded.
/// </summary>
bool playlist_tree_view_t::IsExpanded(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    UINT State = 0;

    if (!GetState(hItem, State))
        return false;

    return ((State & TVIS_EXPANDED) == TVIS_EXPANDED);
}

/// <summary>
/// Returns true if a drop is allowed on the target.
/// </summary>
bool playlist_tree_view_t::AllowDrop(DropZone dropZone) noexcept
{
    if (_hDropTarget == NULL)
        return false;

    auto Node = (const node_t *) GetData(_hDropTarget);

    if (Node == nullptr)
        return false;

    return Node->IsFolder || (!Node->IsFolder && (dropZone != DropZone::Middle));
}
