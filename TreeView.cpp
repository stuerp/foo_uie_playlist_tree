
/** $VER: TreeView.cpp (2026.07.05) P. Stuer **/

#include "pch.h"

#include "TreeView.h"

#pragma hdrstop

/// <summary>
/// Adds an item to the treeview.
/// </summary>
HTREEITEM tree_view_t::Additem(_In_ const std::wstring & text, _In_ size_t playlistIndex, _In_ int iconIndex) const noexcept
{
    const TVINSERTSTRUCTW tvis
    {
        .hParent      = TVI_ROOT,
        .hInsertAfter = TVI_LAST,
        .item         =
        {
            .mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM,
            .pszText        = (LPWSTR) text.c_str(),
            .iImage         = iconIndex,
            .iSelectedImage = iconIndex,
            .lParam         = (LPARAM) playlistIndex,
        },
    };

    auto hTreeItem = TreeView_InsertItem(_hTreeView, &tvis);

    return hTreeItem;
}
