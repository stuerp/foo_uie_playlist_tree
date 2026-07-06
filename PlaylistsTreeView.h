
/** $VER: PlaylistsTreeView.h (2026.07.06) P. Stuer **/

#pragma once

#include "TreeView.h"

#include "Node.h"

/// <summary>
/// Implements a tree view that understands nodes.
/// </summary>
class playlists_tree_view_t : public tree_view_t
{
public:
    playlists_tree_view_t() : tree_view_t() { };

    playlists_tree_view_t(const playlists_tree_view_t &) = delete;
    playlists_tree_view_t(const playlists_tree_view_t &&) = delete;
    playlists_tree_view_t & operator=(const playlists_tree_view_t &) = delete;
    playlists_tree_view_t & operator=(playlists_tree_view_t &&) = delete;

    virtual ~playlists_tree_view_t() noexcept { };

    /// <summary>
    /// Renames the specified node.
    /// </summary>
    void RenameNode(_In_ const GUID & id, _In_ const std::wstring & newName) const noexcept
    {
        HTREEITEM hItem = FindNode(id);

        if (hItem != NULL)
            RenameItem(hItem, newName);
    }

    /// <summary>
    /// Renames the specified node.
    /// </summary>
    void RemoveNode(_In_ const GUID & id) const noexcept
    {
        HTREEITEM hItem = FindNode(id);

        if (hItem != NULL)
            RemoveItem(hItem);
    }

    /// <summary>
    /// Find the specified node.
    /// </summary>
    HTREEITEM FindNode(_In_ const GUID & id) const noexcept
    {
        HTREEITEM hFoundItem = NULL;

        Walk([&](HTREEITEM hItem) -> bool
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
};
