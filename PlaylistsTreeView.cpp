
/** $VER: PlaylistsTreeView.cpp (2026.07.06) P. Stuer **/

#include "pch.h"

#include "PlaylistsTreeView.h"

#pragma hdrstop

/// <summary>
/// Gets the text of the specified node.
/// </summary>
std::wstring playlists_tree_view_t::GetText(_In_ const GUID & id) const noexcept
{
    HTREEITEM hItem = FindNode(id);

    if (hItem == NULL)
        return { };

    return tree_view_t::GetText(hItem);
}
