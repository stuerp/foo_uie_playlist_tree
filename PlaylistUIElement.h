
/** $VER: PlaylistsUIElement.h (2026.08.02) P. Stuer **/

#pragma once

#include "pch.h"

#include "DropTarget.h"
#include "EditSubclass.h"
#include "FolderManager.h"
#include "MultiSelectTreeView.h"
#include "PlaylistLockManager.h"
#include "PlaylistTreeView.h"
#include "Resources.h"
#include "StringEnumerator.h"
#include "Tracker.h"
#include "TreeViewSubclass.h"
#include "UIElement.h"

#include <sdk/playlist.h>

/// <summary>
/// Implements the user interface element base class.
/// </summary>
class playlist_uielement_t : public uielement_t, private playlist_callback, private play_callback_impl_base, private multi_select_tree_view_t, private folder_manager_callback_t
{
public:
    playlist_uielement_t();

    playlist_uielement_t(const playlist_uielement_t &) = delete;
    playlist_uielement_t & operator=(const playlist_uielement_t &) = delete;
    playlist_uielement_t(playlist_uielement_t &&) = delete;
    playlist_uielement_t & operator=(playlist_uielement_t &&) = delete;

    virtual ~playlist_uielement_t();

    DWORD GetDropEffect(DWORD keyState, const POINT & pt) noexcept;
    void DropFiles(IDataObject * dataObject) noexcept;

    void Refresh() noexcept;

    virtual void OnFontsChanged() noexcept override;

protected:
    void SetConfiguration(const char * data, size_t size) noexcept;
    std::string GetConfiguration() const noexcept;

    static std::string GetDefaultConfiguration() noexcept;

private:
    #pragma region CWindowImpl

    virtual LRESULT OnCreate(CREATESTRUCT * cs) noexcept override;
    virtual void OnDestroy() noexcept override;
    virtual void OnSize(UINT nType, CSize size) noexcept override;

    void OnPaint(CDCHandle dc) noexcept;
    void OnCommand(UINT notifyCode, int id, CWindow wnd) noexcept;
    void OnSetFocus(CWindow wndOld) noexcept;

    LRESULT OnCustomDraw(NMHDR * nmhd) noexcept;
    LRESULT OnRightClick(NMHDR * nmhd) noexcept;
    LRESULT OnMiddleClick(NMHDR * nmhd) noexcept;
    LRESULT OnKillFocus(NMHDR * nmhd) noexcept;

    LRESULT OnKeyDown(NMHDR * nmhd) noexcept;
    LRESULT OnGetInfoTip(NMHDR * nmhd) noexcept;
    LRESULT OnSelectionChanged(NMHDR * nmhd) noexcept;
    LRESULT OnGetDisplayInfo(NMHDR * nmhd) noexcept;
    LRESULT OnItemExpanded(NMHDR * nmhd) noexcept;
    LRESULT OnBeginDrag(NMHDR * nmhd) noexcept;
    LRESULT OnDeletingItem(NMHDR * nmhd) noexcept;
    LRESULT OnBeginLabelEdit(NMHDR * nmhd) noexcept;
    LRESULT OnEndLabelEdit(NMHDR * nmhd) noexcept;

    void OnEditChange(UINT notifyCode, int id, CWindow wnd);

    void OnMouseMove(UINT flags, CPoint point) noexcept;
    void OnMouseLeave() noexcept;
    void OnLButtonUp(UINT flags, CPoint point) noexcept;

    BEGIN_MSG_MAP_EX(playlist_uielement_t)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_SETFOCUS(OnSetFocus)

        COMMAND_HANDLER_EX(IDC_EDITBOX, EN_CHANGE, OnEditChange)

        MSG_WM_COMMAND(OnCommand)

        NOTIFY_HANDLER_EX(IDC_TREEVIEW, NM_CUSTOMDRAW, OnCustomDraw)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, NM_RCLICK, OnRightClick)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, NM_MCLICK, OnMiddleClick)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, NM_KILLFOCUS, OnKillFocus)

        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_KEYDOWN, OnKeyDown)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_GETINFOTIP, OnGetInfoTip)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_SELCHANGED, OnSelectionChanged)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_GETDISPINFO, OnGetDisplayInfo)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_DELETEITEM, OnDeletingItem)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_BEGINLABELEDIT, OnBeginLabelEdit)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_ENDLABELEDIT, OnEndLabelEdit)
        NOTIFY_HANDLER_EX(IDC_TREEVIEW, TVN_BEGINDRAG, OnBeginDrag)

        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONUP(OnLButtonUp)

//      CHAIN_MSG_MAP(multi_select_tree_view_t)
        CHAIN_MSG_MAP(uielement_t)
    END_MSG_MAP()

    #pragma endregion

    #pragma region playlist_callback

    void on_items_added(size_t playlistIndex, size_t start, const pfc::list_base_const_t<metadb_handle_ptr> & handles, const bit_array & selection) noexcept override;
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
    void on_playlist_created(size_t playlistIndex, const char * name, size_t size) noexcept override;
    void on_playlists_reorder(const size_t * order, size_t count) noexcept override;
    void on_playlists_removing(const bit_array & mask, size_t oldCount, size_t newCount) noexcept override;
    void on_playlists_removed(const bit_array & mask, size_t oldCount, size_t newCount) noexcept override;
    void on_playlist_renamed(size_t playlistIndex, const char * name, size_t size) noexcept override;

    void on_playlist_locked(size_t playlistIndex, bool isLocked) noexcept override;

    void on_default_format_changed() noexcept override;

    void on_playback_order_changed(size_t playlistIndex) noexcept override;

    #pragma endregion

    #pragma region play_callback

    void on_playback_starting(play_control::t_track_command command, bool isPaused) { }
    void on_playback_new_track(metadb_handle_ptr track);
    void on_playback_stop(play_control::t_stop_reason reason);
    void on_playback_seek(double time) { }
    void on_playback_pause(bool isPaused);
    void on_playback_edited(metadb_handle_ptr track) { }
    void on_playback_dynamic_info(const file_info&  info) { }
    void on_playback_dynamic_info_track(const file_info & info) { }
    void on_playback_time(double time) { }
    void on_volume_change(float newValue) { }

    #pragma endregion

    #pragma region folder_callback_t

	void OnFolderCreated(const GUID & id, const std::string & name) noexcept override;
	void OnFolderRemoving(const GUID & id) noexcept override;
	void OnFolderRemoved(const GUID & id) noexcept override;
	void OnFolderRenamed(const GUID & id, const std::string & oldName, const std::string & newName) noexcept override;

    #pragma endregion

    void FromJSON(json object) noexcept;
    void FromJSON(json object, const GUID & parentId) noexcept;

    void SelectPlaylist(size_t playlistIndex) const noexcept;

    HRESULT InitImageList() noexcept;

    void ModifyFilterMask(uint32_t newFilterMask) const noexcept;
    bool IsProhibited(const node_t * node, uint32_t filterMask) const noexcept;

    LONG CalculateEditHeight(HWND hWnd, HFONT hFont) noexcept;
    void ResetAutoComplete() noexcept;

protected:
    playlist_tree_view_t _TreeView;
    CEdit _EditBox;

private:
    imagelist_t _ImageList;

    static_api_ptr_t<folder_manager_t> _FolderManager;
    static_api_ptr_t<playlist_manager_v5> _PlaylistManager;
    static_api_ptr_t<playlist_lock_manager_t> _LockManager;

    HTREEITEM _hDropTarget = NULL;
    HTREEITEM _hHighlightedtem = NULL;

    treeview_subclass_t _TreeViewSubclass;
    edit_subclass_t _EditSubclass;

    bool _IsPlaying = false;
    bool _IgnoreNotifications = false;
    bool _IsUser = false;

    drop_target_t * _DropTarget = nullptr;
    string_enumerator_t * _StringEnumerator = nullptr;
};

/// <summary>
/// Implements a callback for playlist_incoming_item_filter_v2::process_dropped_files_async().
/// </summary>
class incoming_item_filter_callback_t : public process_locations_notify
{
public:
    incoming_item_filter_callback_t(size_t playlistIndex, bool selectDroppedItems) : _PlaylistIndex(playlistIndex), _SelectDroppedItems(selectDroppedItems) { }

    incoming_item_filter_callback_t(const incoming_item_filter_callback_t &) = delete;
    incoming_item_filter_callback_t & operator=(const incoming_item_filter_callback_t &) = delete;
    incoming_item_filter_callback_t(incoming_item_filter_callback_t &&) = delete;
    incoming_item_filter_callback_t & operator=(incoming_item_filter_callback_t &&) = delete;

    virtual ~incoming_item_filter_callback_t() = default;

    /// <summary>
    /// Called when a playlist_incoming_item_filter_v2 object has finished processing dropped files or playlist items.
    /// </summary>
    void on_completion(const pfc::list_base_const_t<metadb_handle_ptr> & items)
    {
        static_api_ptr_t<playlist_manager> PlaylistManager;

        PlaylistManager->playlist_add_items(_PlaylistIndex, items, (_SelectDroppedItems ? (const bit_array &) bit_array_true() : (const bit_array &) bit_array_false()));
    }

    /// <summary>
    /// Called when a playlist_incoming_item_filter_v2 object has aborted processing dropped files or playlist items.
    /// </summary>
    void on_aborted()
    {
        Log.AtWarn().Write(STR_COMPONENT_BASENAME " was notified that the user aborted processing the dropped items.");
    }

private:
    size_t _PlaylistIndex;
    bool _SelectDroppedItems;
};

extern tracker_t<playlist_uielement_t> _UIElementTracker;
