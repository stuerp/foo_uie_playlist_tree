
/** $VER: PlaylistsTreeView.cpp (2026.07.12) P. Stuer **/

#include "pch.h"

#include "PlaylistTreeView.h"
#include "Node.h"

#pragma hdrstop

/// <summary>
/// Gets the text of the specified item.
/// </summary>
std::string playlist_tree_view_t::GetText(const GUID id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return { };

    return tree_view_t::GetText(hItem);
}

/// <summary>
/// Sets the text of the specified item.
/// </summary>
void playlist_tree_view_t::SetName(const GUID id, const std::string & name) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return;

    auto Node = (node_t *) GetData(hItem);

    if (Node == nullptr)
        return;

    Node->Name = name;

    RedrawItem(hItem);
}

/// <summary>
/// Adds an item.
/// </summary>
void playlist_tree_view_t::AddItem(const GUID & parentId, const GUID & insertAfterId, const GUID & id, const std::string & name, bool isFolder, bool isExpanded) const noexcept
{
    auto Node = new node_t(id, name, isFolder, isExpanded);

    HTREEITEM hParent = FindItem(parentId);
    HTREEITEM hInsertAfter = FindItem(insertAfterId);

    tree_view_t::AddItem((hParent == NULL) ? TVI_ROOT : hParent, (hInsertAfter == NULL) ? TVI_LAST : hInsertAfter, TVIS_EXPANDED, Node);
}

/// <summary>
/// Removes the specified item.
/// </summary>
void playlist_tree_view_t::RemoveItem(const GUID id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return;

    tree_view_t::RemoveItem(hItem);
}

/// <summary>
/// Find the specified item.
/// </summary>
HTREEITEM playlist_tree_view_t::FindItem(const GUID & id) const noexcept
{
    if (id == GUID())
        return NULL;

    HTREEITEM hFoundItem = NULL;

    tree_view_t::Walk([&](HTREEITEM hItem, void * context) -> bool
    {
        auto Node = (const node_t *) GetData(hItem);

        if ((Node != nullptr) && (Node->Id == id))
        {
            hFoundItem = hItem;

            return false;
        }

        return true; // Continue enumerating
    });

    return hFoundItem;
}

/// <summary>
/// Gets the item at the specified point.
/// </summary>
HTREEITEM playlist_tree_view_t::GetItem(const POINT & pt) const noexcept
{
    auto hItem = tree_view_t::GetItem(pt);

    return hItem;
}

/// <summary>
/// Refreshes the specified item.
/// </summary>
bool playlist_tree_view_t::RefreshItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    tree_view_t::RefreshItem(hItem);

    return true;
}
