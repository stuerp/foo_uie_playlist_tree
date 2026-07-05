
/** $VER: PlaylistsUIElement.h (2026.07.05) P. Stuer **/

#pragma once

#include "pch.h"

#include "UIElement.h"
#include "State.h"
#include "TreeView.h"

#include <SDK/playlist.h>

/// <summary>
/// Implements the UIElement and Playback interface.
/// </summary>
class playlists_uielement_t : public uielement_t, public playlist_callback
{
public:
    playlists_uielement_t();

    playlists_uielement_t(const playlists_uielement_t &) = delete;
    playlists_uielement_t & operator=(const playlists_uielement_t &) = delete;
    playlists_uielement_t(playlists_uielement_t &&) = delete;
    playlists_uielement_t & operator=(playlists_uielement_t &&) = delete;

    virtual ~playlists_uielement_t();

private:
    #pragma region CWindowImpl

    virtual LRESULT OnCreate(CREATESTRUCT * cs) noexcept override;
    virtual void OnDestroy() noexcept override;
    virtual void OnSize(UINT nType, CSize size) noexcept override;

    #pragma endregion

    void OnCommand(_In_ UINT notifyCode, _In_ int id, _In_ CWindow wnd) noexcept;
    LRESULT OnNotify(_In_ int id, _In_ NMHDR * nmhd) noexcept;

    BEGIN_MSG_MAP(playlists_uielement_t)
        CHAIN_MSG_MAP(uielement_t)

        MSG_WM_COMMAND(OnCommand);
        MSG_WM_NOTIFY(OnNotify);
    END_MSG_MAP()

    #pragma region playlist_callback

    void on_items_added(t_size p_playlist, t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) noexcept override;
    void on_items_reordered(t_size playlistIndex, const t_size * order, t_size count) noexcept override;
    void on_items_removing(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount) noexcept override;
    void on_items_removed(t_size playlistIndex, const bit_array & mask, t_size oldCount, t_size newCount) noexcept override;

    void on_items_selection_change(t_size playlistIndex, const bit_array & affectedItems, const bit_array & state) noexcept override;

    void on_items_modified(t_size playlistIndex, const bit_array & mask) noexcept override;
    void on_items_modified_fromplayback(t_size playlistIndex, const bit_array & mask, play_control::t_display_level displayLevel) noexcept override;
    void on_items_replaced(t_size playlistIndex, const bit_array & mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & data) noexcept override;

    void on_item_focus_change(t_size playlistIndex, t_size oldItemIndex, t_size newItemIndex) noexcept override;
    void on_item_ensure_visible(t_size playlistIndex, t_size itemIndex) noexcept override;

    void on_playlist_activate(t_size oldIndex, t_size newIndex) noexcept override;
    void on_playlist_created(t_size index, const char * name, t_size size) noexcept override;
    void on_playlists_reorder(const t_size * order, t_size count) noexcept override;
    void on_playlists_removing(const bit_array & mask, t_size oldCount, t_size newCount) noexcept override;
    void on_playlists_removed(const bit_array & mask, t_size oldCount, t_size newCount) noexcept override;
    void on_playlist_renamed(t_size index, const char * name, t_size size) noexcept override;

    void on_playlist_locked(t_size index, bool isLocked) noexcept override;

    void on_default_format_changed() noexcept override;

    void on_playback_order_changed(t_size index) noexcept override;

    #pragma endregion

private:
    void GetPlaylists() noexcept;

protected:
    state_t _State;

private:
    tree_view_t _TreeView;

    std::vector<HTREEITEM> _Items;
/*
    // shell32.dll icons
    enum Icon : int
    {
        File    = 0,
        Folder  = 3
    };
*/
    // imageres.dll icons
    enum Icon : int
    {
        File    = 126,
        Folder  = 4
    };
};
