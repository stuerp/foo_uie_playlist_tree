
/** $VER: PlaylistsTreeView.h (2026.07.06) P. Stuer **/

#pragma once

#include "TreeView.h"

class playlists_tree_view_t : public tree_view_t
{
public:
    playlists_tree_view_t() : tree_view_t() { };

    playlists_tree_view_t(const playlists_tree_view_t &) = delete;
    playlists_tree_view_t(const playlists_tree_view_t &&) = delete;
    playlists_tree_view_t & operator=(const playlists_tree_view_t &) = delete;
    playlists_tree_view_t & operator=(playlists_tree_view_t &&) = delete;

    virtual ~playlists_tree_view_t() noexcept { };
};
