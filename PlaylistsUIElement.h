
/** $VER: PlaylistsUIElement.h (2026.07.06) P. Stuer **/

#pragma once

#include "pch.h"

#include "UIElement.h"
#include "State.h"
#include "PlaylistsTreeView.h"

#include <SDK/playlist.h>

/// <summary>
/// Implements the user interface element base class.
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

    void OnMouseMove(_In_ UINT flags, _In_ CPoint point) noexcept;
    void OnMouseLeave() noexcept;
    void OnLButtonUp(_In_ UINT flags, _In_ CPoint point) noexcept;

    BEGIN_MSG_MAP(playlists_uielement_t)
        CHAIN_MSG_MAP(uielement_t)

        MSG_WM_COMMAND(OnCommand);
        MSG_WM_NOTIFY(OnNotify);
        MSG_WM_MOUSEMOVE(OnMouseMove);
        MSG_WM_MOUSELEAVE(OnMouseLeave);
        MSG_WM_LBUTTONUP(OnLButtonUp);
    END_MSG_MAP()

    #pragma region playlist_callback

    void on_items_added(size_t p_playlist, size_t p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) noexcept override;
    void on_items_reordered(size_t playlistIndex, const size_t * order, size_t count) noexcept override;
    void on_items_removing(size_t playlistIndex, const bit_array & mask, size_t oldCount, size_t newCount) noexcept override;
    void on_items_removed(size_t playlistIndex, const bit_array & mask, size_t oldCount, size_t newCount) noexcept override;

    void on_items_selection_change(size_t playlistIndex, const bit_array & affectedItems, const bit_array & state) noexcept override;

    void on_items_modified(size_t playlistIndex, const bit_array & mask) noexcept override;
    void on_items_modified_fromplayback(size_t playlistIndex, const bit_array & mask, play_control::t_display_level displayLevel) noexcept override;
    void on_items_replaced(size_t playlistIndex, const bit_array & mask, const pfc::list_base_const_t<playlist_callback::t_on_items_replaced_entry> & data) noexcept override;

    void on_item_focus_change(size_t playlistIndex, size_t oldItemIndex, size_t newItemIndex) noexcept override;
    void on_item_ensure_visible(size_t playlistIndex, size_t itemIndex) noexcept override;

    void on_playlist_activate(size_t oldIndex, size_t newIndex) noexcept override;
    void on_playlist_created(size_t index, const char * name, size_t size) noexcept override;
    void on_playlists_reorder(const size_t * order, size_t count) noexcept override;
    void on_playlists_removing(const bit_array & mask, size_t oldCount, size_t newCount) noexcept override;
    void on_playlists_removed(const bit_array & mask, size_t oldCount, size_t newCount) noexcept override;
    void on_playlist_renamed(size_t index, const char * name, size_t size) noexcept override;

    void on_playlist_locked(size_t index, bool isLocked) noexcept override;

    void on_default_format_changed() noexcept override;

    void on_playback_order_changed(size_t index) noexcept override;

    #pragma endregion

private:
    void GetPlaylists() noexcept;
    void SelectPlaylist(size_t playlistIndex) const noexcept;

protected:
    state_t _State;

private:
    static_api_ptr_t<playlist_manager_v5> _PlaylistManager;

    playlists_tree_view_t _TreeView;
    HTREEITEM _hDropTarget = NULL;
};
