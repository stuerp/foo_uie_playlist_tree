
/** $VER: PlaylistsTreeView.cpp (2026.07.07) P. Stuer **/

#include "pch.h"

#include "PlaylistTreeView.h"

#pragma hdrstop

/// <summary>
/// Gets the text of the specified item.
/// </summary>
std::string playlist_tree_view_t::GetText(_In_ const GUID id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return { };

    return tree_view_t::GetText(hItem);
}

/// <summary>
/// Sets the text of the specified item.
/// </summary>
void playlist_tree_view_t::SetText(_In_ const GUID id, _In_ const std::string & text) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return;

    tree_view_t::SetText(hItem, text);
}

/// <summary>
/// Adds an item.
/// </summary>
void playlist_tree_view_t::AddItem(_In_ const GUID id, _In_ const std::string & name, _In_ const std::string & text, _In_ bool isFolder) const noexcept
{
    auto Node = new node_t(name, id, false);

    tree_view_t::AddItem(TVI_ROOT, TVI_LAST, text, isFolder ? Icon::Folder :  Icon::File, Node);
}

/// <summary>
/// Removes the specified item.
/// </summary>
void playlist_tree_view_t::RemoveItem(_In_ const GUID id) const noexcept
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
    tree_view_t::Walk([&](HTREEITEM hItem) -> bool
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
HTREEITEM playlist_tree_view_t::FindItem(_In_ const GUID id) const noexcept
{
    HTREEITEM hFoundItem = NULL;

    tree_view_t::Walk([&](HTREEITEM hItem) -> bool
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
