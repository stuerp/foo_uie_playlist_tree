
/** $VER: PlaylistsUIElement.h (2026.07.12) P. Stuer **/

#pragma once

#include "pch.h"

#include "UIElement.h"
#include "UIElementTracker.h"
#include "PlaylistTreeView.h"
#include "FolderManager.h"
#include "EditSubclass.h"

#include <SDK\playlist.h>

/// <summary>
/// Implements the user interface element base class.
/// </summary>
class playlist_uielement_t : public uielement_t, public playlist_callback, private play_callback_impl_base
{
public:
    playlist_uielement_t();

    playlist_uielement_t(const playlist_uielement_t &) = delete;
    playlist_uielement_t & operator=(const playlist_uielement_t &) = delete;
    playlist_uielement_t(playlist_uielement_t &&) = delete;
    playlist_uielement_t & operator=(playlist_uielement_t &&) = delete;

    virtual ~playlist_uielement_t();

    void ApplyConfiguration() noexcept;

protected:
    void SetConfiguration(const char * data, size_t size) noexcept;
    std::string GetConfiguration() const noexcept;

    static std::string GetDefaultConfiguration() noexcept;

private:
    #pragma region CWindowImpl

    virtual LRESULT OnCreate(CREATESTRUCT * cs) noexcept override;
    virtual void OnDestroy() noexcept override;
    virtual void OnSize(UINT nType, CSize size) noexcept override;

    #pragma endregion

    void OnCommand(UINT notifyCode, int id, CWindow wnd) noexcept;
    LRESULT OnNotify(int id, NMHDR * nmhd) noexcept;

    void OnMouseMove(UINT flags, CPoint point) noexcept;
    void OnMouseLeave() noexcept;
    void OnLButtonUp(UINT flags, CPoint point) noexcept;

    BEGIN_MSG_MAP(playlist_uielement_t)
        CHAIN_MSG_MAP(uielement_t)

        MSG_WM_DESTROY(OnDestroy);

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

    #pragma region play_callback

    void on_playback_starting(play_control::t_track_command command, bool isPaused) { }
    void on_playback_new_track(metadb_handle_ptr track);
    void on_playback_stop(play_control::t_stop_reason reason);
    void on_playback_seek(double time) { }
    void on_playback_pause(bool state);
    void on_playback_edited(metadb_handle_ptr track) { }
    void on_playback_dynamic_info(const file_info&  info) { }
    void on_playback_dynamic_info_track(const file_info & info) { }
    void on_playback_time(double time) { }
    void on_volume_change(float newValue) { }

    #pragma endregion

    void FromJSON(json object) noexcept;
    void FromJSON(json object, const GUID & parentId) noexcept;

    void SelectPlaylist(size_t playlistIndex) const noexcept;
    bool InitImageList() noexcept;

protected:
    playlist_tree_view_t _TreeView;

private:
    himagelist_t _hImageList;

    static_api_ptr_t<playlist_manager_v5> _PlaylistManager;
    static_api_ptr_t<folder_manager_t> _FolderManager;

    HTREEITEM _hDropTarget = NULL;
    HTREEITEM _hPopupItem = NULL;

    edit_subclass_t _EditSubclass;

    bool _IsNotification = false;
    bool _IsUser = false;
};

extern uielement_tracker_t<playlist_uielement_t> _UIElementTracker;
