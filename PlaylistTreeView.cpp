
/** $VER: PlaylistsTreeView.cpp (2026.07.08) P. Stuer **/

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

    RefreshItem(hItem);
}

/// <summary>
/// Adds an item.
/// </summary>
void playlist_tree_view_t::AddItem(const GUID & id, const std::string & name, bool isFolder, bool isExpanded, const GUID & parentId) const noexcept
{
    auto Node = new node_t(id, name, isFolder, isExpanded);

    HTREEITEM hParent = FindItem(parentId);

    tree_view_t::AddItem((hParent == NULL) ? TVI_ROOT : hParent, TVI_LAST, TVIS_EXPANDED, Node);
}

/// <summary>
/// Removes the specified item.
/// </summary>
void playlist_tree_view_t::RemoveItem(const GUID id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return;

    auto Node = (const node_t *) tree_view_t::GetData(hItem);

    if (Node != nullptr)
        delete Node;

    tree_view_t::RemoveItem(hItem);
}

/// <summary>
/// Removes all items.
/// </summary>
void playlist_tree_view_t::Clear() const noexcept
{
    tree_view_t::Walk([&](HTREEITEM hItem, void * context) -> bool
    {
        auto Node = (const node_t *) GetData(hItem);

        if (Node != nullptr)
            delete Node;

        return true; // Continue enumerating
    });

    tree_view_t::Clear();
}

/// <summary>
/// Find the specified node.
/// </summary>
HTREEITEM playlist_tree_view_t::FindItem(const GUID & id) const noexcept
{
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
