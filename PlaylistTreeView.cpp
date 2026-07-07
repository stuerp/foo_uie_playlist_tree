
/** $VER: PlaylistsTreeView.cpp (2026.07.07) P. Stuer **/

#include "pch.h"

#include "PlaylistTreeView.h"

#pragma hdrstop

/// <summary>
/// Gets the text of the specified item.
/// </summary>
std::wstring playlist_tree_view_t::GetText(_In_ const GUID id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return { };

    return tree_view_t::GetText(hItem);
}

/// <summary>
/// Renames the specified item.
/// </summary>
void playlist_tree_view_t::SetText(_In_ const GUID id, _In_ const std::wstring & text) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem != NULL)
        tree_view_t::SetText(hItem, text);
}

/// <summary>
/// Adds a node.
/// </summary>
void playlist_tree_view_t::AddItem(_In_ const GUID id, _In_ const std::wstring & text, _In_ bool isFolder) const noexcept
{
    auto Node = new node_t(id, false);

    tree_view_t::AddItem(TVI_ROOT, TVI_LAST, text.c_str(), isFolder ? Icon::Folder :  Icon::File, Node);
}

/// <summary>
/// Renames the specified node.
/// </summary>
void playlist_tree_view_t::RemoveItem(_In_ const GUID id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem != NULL)
    {
        auto Node = (node_t *) tree_view_t::GetData(hItem);

        if (Node != nullptr)
            delete Node;

        tree_view_t::RemoveItem(hItem);
    }
}

/// <summary>
/// Find the specified node.
/// </summary>
HTREEITEM playlist_tree_view_t::FindItem(_In_ const GUID id) const noexcept
{
    HTREEITEM hFoundItem = NULL;

    tree_view_t::Walk([&](HTREEITEM hItem) -> bool
    {
        const auto Node = (node_t *) GetData(hItem);

        if ((Node != nullptr) && (Node->Id == id))
        {
            hFoundItem = hItem;

            return false;
        }

        return true; // Continue enumerating
    });

    return hFoundItem;
}
