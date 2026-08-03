
/** $VER: PlaylistsUIElement.cpp (2026.08.03) P. Stuer **/

#include "pch.h"

#include "PlaylistUIElement.h"

#include "ImageList.h"
#include "TitleFormat.h"
#include "Node.h"
#include "State.h"
#include "Theme.h"
#include "Toggle.h"
#include "Log.h"

#include <unordered_map>

#pragma hdrstop

/// <summary>
/// Initializes a new instance. Note to self: Don't put anything expensive here. Brain-dead CUI constructs and destructs UI elements at will.
/// </summary>
playlist_uielement_t::playlist_uielement_t() : multi_select_tree_view_t(IDC_TREEVIEW)
{
    HRESULT hr = ::OleInitialize(nullptr);

    if (!SUCCEEDED(hr))
        Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to initialize OLE: 0x%08X.", hr);

    _PlaylistManager->register_callback(this, (t_uint32) playlist_callback::flag_all);
    _FolderManager->RegisterCallback(this);
}

/// <summary>
/// Deletes this instance. Note to self: Don't put anything expensive here. Brain-dead CUI constructs and destructs UI elements at will.
/// </summary>
playlist_uielement_t::~playlist_uielement_t()
{
    _FolderManager->UnregisterCallback(this);
    _PlaylistManager->unregister_callback(this);

    ::OleUninitialize();
}

/// <summary>
/// Creates the window.
/// </summary>
LRESULT playlist_uielement_t::OnCreate(CREATESTRUCT * cs) noexcept
{
    auto Result = __super::OnCreate(cs);

    if (Result != 0)
        return Result;

   _UIElementTracker.Add(this);

    // Create the tree view.
    {
        if (!_TreeView.Create(m_hWnd, IDC_TREEVIEW))
            return -1;

        _TreeViewSubclass.Attach(_TreeView.Get());

        {
            HRESULT hr = InitImageList();

            if (!SUCCEEDED(hr))
                Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to initialize image list: 0x%08X.", hr);
        }
    }

    // Create the edit box.
    {
        constexpr DWORD Styles = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
        constexpr DWORD ExStyles = WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE;

        if (!_EditBox.Create(m_hWnd, NULL, nullptr, Styles, ExStyles, IDC_EDITBOX, nullptr))
            return -1;

        // Add Auto Complete to the edit box.
        {
            // Enumerates the node names for Auto Complete to display.
            _StringEnumerator = new string_enumerator_t();

            IAutoComplete * ac = nullptr;

            HRESULT hr = ::CoCreateInstance(CLSID_AutoComplete, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ac));

            if (SUCCEEDED(hr))
            {
                hr = ::SHAutoComplete(_EditBox, 0);

                if (SUCCEEDED(hr))
                {
                    ac->Init(_EditBox, _StringEnumerator, nullptr, nullptr);

                    {
                        IAutoComplete2 * ac2 = nullptr;

                        hr = ac->QueryInterface(IID_PPV_ARGS(&ac2));

                        if (SUCCEEDED(hr))
                        {
                            ac2->SetOptions(ACO_AUTOSUGGEST | ACO_UPDOWNKEYDROPSLIST);

                            ac2->Release();
                        }
                        else
                            Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to set auto complete options: 0x%08X.", hr);
                    }
                }

                ac->Release();
            }
            else
                Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to install auto complete on edit box: 0x%08X.", hr);
        }

        _EditBox.ShowWindow(_State._IsQuickSearchVisible ? SW_SHOW : SW_HIDE);
    }

    // Create the drop target.
    {
        _DropTarget = new drop_target_t(_TreeView.Get(), this);

        HRESULT hr = ::RegisterDragDrop(m_hWnd, _DropTarget);

        if (!SUCCEEDED(hr))
            Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to register drop target: 0x%08X.", hr);
    }

//  _State._Object.clear(); // Uncomment to reset the state.

    // Deserialize the tree view nodes.
    FromJSON(_State._Object);

    ResetAutoComplete();

    // Select the active playlist.
    {
        const size_t Index = _PlaylistManager->get_active_playlist();

        if (Index != SIZE_MAX)
            SelectPlaylist(Index);
    }

    // Set the horizontal scroll position to 0 to start with. Items with long texts tend to force scrolling.
    SCROLLINFO si =
    {
        .cbSize = sizeof(si),
        .fMask = SIF_POS,
    };

    ::SetScrollInfo(_TreeView.Get(), SB_HORZ, &si, TRUE);

    _DarkMode.AddControls(m_hWnd);

    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void playlist_uielement_t::OnDestroy() noexcept
{
    // Destroy the drop target.
    {
        if (m_hWnd != NULL)
        {
            HRESULT hr = ::RevokeDragDrop(m_hWnd);

            if (!SUCCEEDED(hr))
                Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to revoke drop target: 0x%08X.", hr);
        }

        if (_DropTarget != nullptr)
        {
            delete _DropTarget;

            _DropTarget = nullptr;
        }
    }

    // Destroy the string enumerator.
    if (_StringEnumerator != nullptr)
    {
        delete _StringEnumerator;

        _StringEnumerator = nullptr;
    }

    // Destroy the edit box.
    if (_EditBox.IsWindow())
        _EditBox.DestroyWindow();

    // Destroy the tree view.
    if (_TreeView.Get() != NULL)
    {
        auto Scope = toggle_t(_IgnoreNotifications, true);

        _ImageList.Reset();

        _TreeViewSubclass.Detach(_TreeView.Get());

        _TreeView.Destroy();
    }

    _UIElementTracker.Remove(this);

    __super::OnDestroy();

    SetMsgHandled(TRUE);
}

/// <summary>
/// Handles the WM_SIZE message.
/// </summary>
void playlist_uielement_t::OnSize(UINT type, CSize size) noexcept
{
    uielement_t::OnSize(type, size);

    LONG Height = 0;

    if (_State._IsQuickSearchVisible)
    {
        constexpr LONG Gap = 4;

        Height = CalculateEditHeight(_EditBox, _Theme.GetPlaylistFont());

        _EditBox.MoveWindow(4, size.cy - Height - Gap, size.cx - (Gap * 2), Height, TRUE);

        Height += (Gap * 2);
    }

    ::MoveWindow(_TreeView.Get(), 0, 0, size.cx, size.cy - Height, TRUE);
}

/// <summary>
/// Handles the WM_PAINT message.
/// </summary>
void playlist_uielement_t::OnPaint(CDCHandle dc) noexcept
{
    _TreeView.Redraw();

    SetMsgHandled(FALSE);
}

/// <summary>
/// Handles the WM_COMMAND message.
/// </summary>
void playlist_uielement_t::OnCommand(UINT notifyCode, int id, CWindow wnd) noexcept
{
    if (notifyCode != 0)
        return;

    SetMsgHandled(TRUE);

    switch (id)
    {
        // Handles the "New Folder" command.
        case IDM_NEW_FOLDER:
        {
            HRESULT hr = _FolderManager->CreateFolder();

            if (!SUCCEEDED(hr))
                Log.AtError().Write(STR_COMPONENT_BASENAME " failed to create folder: 0x%08X.", hr);

            return;
        }

        // Handles the "New Playlist" command.
        case IDM_NEW_PLAYLIST:
        {
            const size_t NewIndex = _PlaylistManager->create_playlist_autoname();

            if (NewIndex == SIZE_MAX)
                Log.AtError().Write(STR_COMPONENT_BASENAME " failed to create playlist.");

            return;
        }

        // Handles the "Rename" command.
        case IDM_RENAME:
        {
            _TreeView.EditItem(_hHighlightedtem);

            return;
        }

        // Handles the "Remove" command.
        case IDM_REMOVE:
        {
            {
                auto Scope = toggle_t(_IsUser, true);

                _TreeView.RemoveItem(_hHighlightedtem);
            }

            ResetAutoComplete();

            return;
        }

        // Handles the lock commands.
        case IDM_LOCK_ADD_ITEMS:
        {
            ModifyFilterMask(playlist_lock::filter_add);

            return;
        }

        case IDM_LOCK_REMOVE_ITEMS:
        {
            ModifyFilterMask(playlist_lock::filter_remove);

            return;
        }

        case IDM_LOCK_REORDER_ITEMS:
        {
            ModifyFilterMask(playlist_lock::filter_reorder);

            return;
        }

        case IDM_LOCK_REPLACE_ITEMS:
        {
            ModifyFilterMask(playlist_lock::filter_replace);

            return;
        }

        case IDM_LOCK_RENAME:
        {
            ModifyFilterMask(playlist_lock::filter_rename);

            return;
        }

        case IDM_LOCK_REMOVE:
        {
            ModifyFilterMask(playlist_lock::filter_remove_playlist);

            _TreeView.RefreshAllItems(); // Force the item images to be re-assessed.

            return;
        }

        case IDM_LOCK_DEFAULT_ACTION:
        {
            ModifyFilterMask(playlist_lock::filter_default_action);

            return;
        }

        case IDM_LOCK_ALL:
        {
            ModifyFilterMask(~0u);

            _TreeView.RefreshAllItems(); // Force the item images to be re-assessed.

            return;
        }

        case IDM_LOCK_NONE:
        {
            ModifyFilterMask(0);

            _TreeView.RefreshAllItems(); // Force the item images to be re-assessed.

            return;
        }

        // Handles the "Sort" command.
        case IDM_SORT:
        {
            if (!_TreeView.Sort(_hHighlightedtem))
                Log.AtError().Write(STR_COMPONENT_BASENAME " failed to sort.");

            return;
        }

        // Handles the "Save all playlists..." command.
        case IDM_SAVE_ALL:
        {
            if (!standard_commands::main_save_all_playlists())
                Log.AtError().Write(STR_COMPONENT_BASENAME " failed to save all playlists.");

            return;
        }

        // Handles the "Save playlist..." command.
        case IDM_SAVE:
        {
            const auto Node = (node_t *) _TreeView.GetData(_hHighlightedtem);

            if (Node == nullptr)
                return;

            const auto Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == SIZE_MAX)
                return;

            pfc::string FileName;

            if (!_PlaylistManager->playlist_get_name(Index, FileName))
                return;

            pfc::list_t<metadb_handle_ptr> Items;

            _PlaylistManager->playlist_get_all_items(Index, Items);

            pfc::string_formatter Extensions;
            uint32_t DefaultExtension = 0;

            // Create a string containing all the supported external playlist formats for the Windows dialog.
            {
                service_enum_t<playlist_loader> LoaderEnumerator;
                service_ptr_t<playlist_loader> Loader;

                for (uint32_t i = 0; LoaderEnumerator.next(Loader); )
                {
                    if (!Loader->can_write())
                        continue;

                    const char * Extension = Loader->get_extension();

                    Extensions << Extension << " files|*." << Extension << "|";

                    if (::stricmp_utf8(Extension, "fpl") == 0)
                        DefaultExtension = i;

                    i++;
                }
            }

            if (::uGetOpenFileName(wnd, Extensions, DefaultExtension, "fpl", "Save playlist...", nullptr, FileName, TRUE))
            {
                try
                {
                    playlist_loader::g_save_playlist(FileName, Items, fb2k::noAbort);
                }
                catch (pfc::exception & e)
                {
                    popup_message::g_show(msc::FormatText("Failed to save playlist:\n%s", e.what()).c_str(), STR_COMPONENT_NAME, popup_message::icon_error);
                }
            }

            return;
        }

        // Handles the "Load playlist..." command.
        case IDM_LOAD:
        {
            if (!standard_commands::main_load_playlist())
                Log.AtError().Write(STR_COMPONENT_BASENAME " failed to load playlist.");

            return;
        }

        // Handles the "Autoplaylist..." command.
        case IDM_AUTOPLAYLIST:
        {
            const auto Node = (node_t *) _TreeView.GetData(_hHighlightedtem);

            if (Node == nullptr)
                return;

            const auto Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == SIZE_MAX)
                return;

            const auto apm = autoplaylist_manager::get();

            if (apm->is_client_present(Index))
            {
                auto Client = apm->query_client(Index);

                if (Client.is_valid())
                    Client->show_ui(Index);
            }

            return;
        }

        // Handles the "Clear history" command.
        case IDM_CLEAR_HISTORY:
        {
            int Result = popup_message_v3::get()->messageBox(m_hWnd, "Delete removed playlists permanently?", STR_COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

            if (Result == IDYES)
                _PlaylistManager->recycler_purge(bit_array_true());

            return;
        }

        // Handles the "Dump configuration" command.
        case IDM_DUMP:
        {
            console::print(GetConfiguration().c_str());

            return;
        }

        default:
        {
            if (id >= IDM_HISTORY)
            {
                const auto Index = (size_t) (id - IDM_HISTORY);

                _PlaylistManager->recycler_restore(Index);
            }
        }
    }
}

/// <summary>
/// Handles the WM_SETFOCUS message.
/// </summary>
void playlist_uielement_t::OnSetFocus(CWindow wndOld) noexcept
{
    _TreeView.SetFocus(); // Forward the focus.
}

/// <summary>
/// Handles the EN_CHANGE notification.
/// </summary>
void playlist_uielement_t::OnEditChange(UINT notifyCode, int id, CWindow wnd)
{
    wchar_t Text[256] = { };

    if (_EditBox.GetWindowTextW(Text, _countof(Text)) == 0)
        return;

    _TreeView.SelectItem(msc::WideToUTF8(Text));

    SetMsgHandled(TRUE);
}

/// <summary>
/// Handles the WM_MOUSEMOVE message.
/// </summary>
void playlist_uielement_t::OnMouseMove(UINT flags, CPoint point) noexcept
{
    _TreeView.DragMove(point);
}

/// <summary>
/// Handles the WM_MOUSELEAVE message.
/// </summary>
void playlist_uielement_t::OnMouseLeave() noexcept
{
    _TreeView.RemoveInsertMarker();
}

/// <summary>
/// Handles the WM_LBUTTONUP message.
/// </summary>
void playlist_uielement_t::OnLButtonUp(UINT flags, CPoint point) noexcept
{
    _TreeView.EndDrag(false);
}

#pragma region playlist_callback

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_added(size_t playlistIndex, size_t start, const pfc::list_base_const_t<metadb_handle_ptr> & handles, const bit_array & selection) noexcept
{
    const auto Id = _PlaylistManager->playlist_get_guid(playlistIndex);

    _TreeView.RefreshItem(Id);
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_reordered(size_t playlistIndex, const size_t * order, size_t count) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_removing(size_t playlistIndex, const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_removed(size_t playlistIndex, const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
    const auto Id = _PlaylistManager->playlist_get_guid(playlistIndex);

    _TreeView.RefreshItem(Id);
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_selection_change(size_t playlistIndex, const bit_array & affected, const bit_array & state) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_item_focus_change(size_t playlistIndex, size_t from, size_t to) noexcept
{
}
    
/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_modified(size_t playlistIndex, const bit_array & mask) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_modified_fromplayback(size_t playlistIndex, const bit_array & mask, play_control::t_display_level level) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_replaced(size_t playlistIndex, const bit_array & mask, const pfc::list_base_const_t<t_on_items_replaced_entry> & data) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_item_ensure_visible(size_t playlistIndex, size_t itemIndex) noexcept
{
}

/// <summary>
/// Handles the activation of a new playlist.
/// </summary>
void playlist_uielement_t::on_playlist_activate(size_t oldIndex, size_t newIndex) noexcept
{
    SelectPlaylist(newIndex);
}

/// <summary>
/// Handles the creation of a new playlist.
/// </summary>
void playlist_uielement_t::on_playlist_created(size_t index, const char * name, size_t size) noexcept
{
    if ((index == SIZE_MAX) || (name == nullptr) || (size == 0))
        return;

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    Log.AtDebug().Write(STR_COMPONENT_BASENAME ": Playlist %s was created as \"%s\".", msc::GUIDToUTF8(Id).c_str(), name);

    // Get the data of the item we were hovering over, if any.
    const auto Parent = (node_t *) _TreeView.GetData(_hHighlightedtem);

    // Add the item.
    auto ParentId = GUID_NULL;
    auto InsertAfterId = GUID_NULL;

    if (Parent != nullptr)
    {
        if (Parent->IsFolder)
            ParentId = Parent->Id;
        else
            InsertAfterId = Parent->Id;
    }

    _TreeView.AddItem(ParentId, InsertAfterId, Id, name, false, false);

    // Activate the newly created playlist.
    _PlaylistManager->set_active_playlist(index);

    _hHighlightedtem = NULL;

    ResetAutoComplete();
}

/// <summary>
/// Handles playlists having been reordered.
/// </summary>
void playlist_uielement_t::on_playlists_reorder(const size_t * order, size_t count) noexcept
{
}

/// <summary>
/// Handles playlists that are being removed.
/// </summary>
void playlist_uielement_t::on_playlists_removing(const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
    Log.AtDebug().Write(STR_COMPONENT_BASENAME ": %d playlist(s) are being removed.", oldCount - newCount);

    if (_IgnoreNotifications)
        return;

    for (size_t Index = mask.find_first(true, 0, oldCount); Index < oldCount; Index = mask.find_next(true, Index, oldCount))
    {
        const auto Id = _PlaylistManager->playlist_get_guid(Index);

        Log.AtDebug().Write(STR_COMPONENT_BASENAME ": Playlist %s is being removed.", msc::GUIDToUTF8(Id).c_str());

        if (!_TreeView.RemoveItem(Id))
            Log.AtError().Write(STR_COMPONENT_BASENAME " failed to remove item %s.", msc::GUIDToUTF8(Id).c_str());
    }
}

/// <summary>
/// Handles removed playlists.
/// </summary>
void playlist_uielement_t::on_playlists_removed(const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
    Log.AtDebug().Write(STR_COMPONENT_BASENAME ": %d playlist(s) were removed.", oldCount - newCount);

    ResetAutoComplete();
}

/// <summary>
/// Handles a renamed playlist.
/// </summary>
void playlist_uielement_t::on_playlist_renamed(size_t index, const char * newName, size_t newSize) noexcept
{
    if ((index == SIZE_MAX) || (newName == nullptr) || (newSize == 0))
        return;

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    Log.AtDebug().Write(STR_COMPONENT_BASENAME ": Playlist %s was renamed to \"%s\".", msc::GUIDToUTF8(Id).c_str(), newName);

    _TreeView.SetName(Id, newName);

    ResetAutoComplete();
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_default_format_changed() noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_playback_order_changed(size_t p_new_index) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_playlist_locked(size_t index, bool isLocked) noexcept
{
    if (index == SIZE_MAX)
        return;

    if (isLocked)
    {
        pfc::string LockName;

        if (!_PlaylistManager->playlist_lock_query_name(index, LockName))
            LockName = "<Unknown>";

        Log.AtDebug().Write(STR_COMPONENT_BASENAME ": Playlist %d was locked with lock \"%s\".", index, LockName.c_str());
    }
    else
        Log.AtDebug().Write(STR_COMPONENT_BASENAME ": Playlist %d was unlocked.", index);

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    _TreeView.RefreshItem(Id);
}

#pragma endregion

#pragma region play_callback

/// <summary>
/// Playback advanced to new track.
/// </summary>
void playlist_uielement_t::on_playback_new_track(metadb_handle_ptr track)
{
    _IsPlaying = true;

    _TreeView.RefreshAllItems();
}

/// <summary>
/// Playback stopped.
/// </summary>
void playlist_uielement_t::on_playback_stop(play_control::t_stop_reason reason)
{
    _IsPlaying = false;

    _TreeView.RefreshAllItems();
}

/// <summary>
/// Playback paused/resumed.
/// </summary>
void playlist_uielement_t::on_playback_pause(bool isPaused)
{
    _IsPlaying = !isPaused;

    _TreeView.RefreshAllItems();
}

#pragma endregion

#pragma region folder_manager_callback_t

/// <summary>
/// Called after a folder has been created.
/// </summary>
void playlist_uielement_t::OnFolderCreated(const GUID & id, const std::string & name) noexcept
{
    if (_IgnoreNotifications || (id == GUID_NULL) || name.empty())
        return;

    Log.AtDebug().Write(STR_COMPONENT_BASENAME " is adding folder %s to the tree.", msc::GUIDToUTF8(id).c_str());

    // Get the node we were hovering over, if any.
    const auto Parent = (node_t *) _TreeView.GetData(_hHighlightedtem);

    // Add the item.
    auto ParentId = GUID_NULL;
    auto InsertAfterId = GUID_NULL;

    if (Parent != nullptr)
    {
        if (Parent->IsFolder)
            ParentId = Parent->Id;
        else
            InsertAfterId = Parent->Id;
    }

    _TreeView.AddItem(ParentId, InsertAfterId, id, name, true, false);

    _TreeView.SelectItem(id);

    _hHighlightedtem = NULL;

    ResetAutoComplete();

    // Redraw the control after adding the first folder because now we need to draw chevrons.
    {
        uint32_t Count;

        _FolderManager->GetFolderCount(Count);

        if (Count == 1)
            ::InvalidateRect(_TreeView.Get(), NULL, TRUE);
    }
};

/// <summary>
/// Called when a folder is about to be removed.
/// </summary>
void playlist_uielement_t::OnFolderRemoving(const GUID & id) noexcept
{
    Log.AtDebug().Write(STR_COMPONENT_BASENAME " is removing folder %s from the tree.", msc::GUIDToUTF8(id).c_str());

    if (_IgnoreNotifications)
        return;
};

/// <summary>
/// Called when a folder is has been removed.
/// </summary>
void playlist_uielement_t::OnFolderRemoved(const GUID & id) noexcept
{
    Log.AtDebug().Write(STR_COMPONENT_BASENAME " removed folder %s from the tree.", msc::GUIDToUTF8(id).c_str());

    ResetAutoComplete();

    // Redraw the control after deleting the last folder because now we don't need to draw chevrons.
    {
        uint32_t Count;

        _FolderManager->GetFolderCount(Count);

        if (Count == 0)
            ::InvalidateRect(_TreeView.Get(), NULL, TRUE);
    }

    if (_IgnoreNotifications)
        return;
};

/// <summary>
/// Called after a folder has been renamed.
/// </summary>
void playlist_uielement_t::OnFolderRenamed(const GUID & id, const std::string & oldName, const std::string & newName) noexcept
{
    Log.AtDebug().Write(STR_COMPONENT_BASENAME " is renamed folder %s from \"%s\" to \"%s\".", msc::GUIDToUTF8(id).c_str(), oldName.c_str(), newName.c_str());

    ResetAutoComplete();

    if (_IgnoreNotifications)
        return;
};

#pragma endregion

/// <summary>
/// Handles the NM_CUSTOMDRAW notification.
/// </summary>
LRESULT playlist_uielement_t::OnCustomDraw(NMHDR * nmhd) noexcept
{
    const auto tvcd = (NMTVCUSTOMDRAW *) nmhd;

    const auto hTreeView = tvcd->nmcd.hdr.hwndFrom;
    const auto hDC       = tvcd->nmcd.hdc;
    const RECT & rcItem  = tvcd->nmcd.rc;

    switch (tvcd->nmcd.dwDrawStage)
    {
        case CDDS_PREPAINT:
        {
            SetMsgHandled(TRUE);

            // Draw the control background ourselves because a light/dark switch is not handled by fb2k::CCoreDarkModeHooks.
            ::FillRect(hDC, &rcItem, _Theme.GetWindowBrush());

            return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.
        }

        case CDDS_ITEMPREPAINT:
        {
            SetMsgHandled(TRUE);

            if ((rcItem.right - rcItem.left) <= 0)
                return CDRF_DODEFAULT;

            const auto hItem = (HTREEITEM) tvcd->nmcd.dwItemSpec;

            // Get information about the item.
            wchar_t Text[512] = { };

            const TVITEMEX tvi
            {
                .mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE | TVIF_CHILDREN,
                .hItem      = hItem,
                .stateMask  = 0xFF,
                .pszText    = Text,
                .cchTextMax = _countof(Text),
            };

            TreeView_GetItem(hTreeView, &tvi);

            const auto HasFocus      = (::GetFocus() == hTreeView);
            const auto HasChildren   = (tvi.cChildren != 0);
            const auto IsSelected    = ((tvi.state & TVIS_SELECTED) != 0); // || ((tvcd->nmcd.uItemState & CDIS_SELECTED) != 0);
            const auto IsHighlighted = ((tvi.state & TVIS_DROPHILITED) != 0);
            const auto IsHot         = ((tvcd->nmcd.uItemState & CDIS_HOT) != 0);
            const auto IsFocused     = ((tvcd->nmcd.uItemState & CDIS_FOCUS) != 0);

            // Get bounding rectangle of the item text.
            RECT rcText;

            TreeView_GetItemRect(hTreeView, hItem, &rcText, TRUE);

            const LONG IconSize = rcText.bottom - rcText.top;

            // Adjust for horizontal scrolling.
            SCROLLINFO si
            {
                .cbSize = sizeof(si),
                .fMask = SIF_POS
            };

            ::GetScrollInfo(hTreeView, SB_HORZ, &si);

            // Calculate the start position of the item content.
            RECT rc = rcItem;

            rc.left += (IconSize * tvcd->iLevel) - si.nPos;

            // Draw a chevron for a Folder item.
            uint32_t Count;

            HRESULT hr = _FolderManager->GetFolderCount(Count);

            if (SUCCEEDED(hr) && (Count > 0))
            {
                if (HasChildren)
                {
                    RECT rcChev = rc;

                    rcChev.right = rcChev.left + IconSize;

                    const auto hOldFont = (HFONT) ::SelectObject(hDC, _Theme.GetIconFont());

                //  const wchar_t * ChevronLeft  = L"\uE76B";
                    const wchar_t * ChevronRight = L"\uE76C";
                    const wchar_t * ChevronDown  = L"\uE70D";
                //  const wchar_t * ChevronUp    = L"\uE70E";

                    const DTTOPTS Options =
                    {
                        .dwSize = sizeof(Options),
                        .dwFlags = DTT_TEXTCOLOR,
                        .crText = _Theme.GetWindowTextColor()
                    };

                    constexpr DWORD Flags = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;

                    ::DrawThemeTextEx(_Theme.GetTextStyle(), hDC, 0, 0, (tvi.state & TVIS_EXPANDED) ? ChevronDown : ChevronRight, -1, Flags, &rcChev, &Options);

                    ::SelectObject(hDC, hOldFont);
                }

                rc.left += IconSize;
            }

            // Draw the background.
            {
                rc.right = rc.left + (1 + IconSize + 1) + 3 + (rcText.right - rcText.left);

                if (IsSelected)
                {
                    auto & hBrush = HasFocus ? _Theme.GetSelectionBrush() : _Theme.GetInactiveSelectionBrush();

                    ::FillRect(hDC, &rc, hBrush);

                    // Draw the focus rectangle.
                    if (IsFocused)
                    {
                        const auto & hPen = _Theme.GetHighlightPen();

                        const auto hOldBrush = ::SelectObject(hDC, hBrush);
                        const auto hOldPen = ::SelectObject(hDC, hPen);

                        ::RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, 2, 2);

                        ::SelectObject(hDC, hOldPen);
                        ::SelectObject(hDC, hOldBrush);
                    }
                }
                else
                if (IsHot || IsHighlighted)
                {
                    auto & hBrush = _Theme.GetHighlightBrush();

                    ::FillRect(hDC, &rc, hBrush);
                }
            }

            // Draw the image.
            {
                const LONG dx = ((1 + IconSize + 1) - (LONG) _State._ImageSize) / 2;
                const LONG dy = (     IconSize      - (LONG) _State._ImageSize) / 2;

                ::ImageList_Draw(_ImageList, tvi.iImage, hDC, rc.left + dx, rc.top + dy, ILD_NORMAL);

                rc.left += (1 + IconSize + 1) + 3;
            }

            // Draw the text.
            {
                const COLORREF Color = IsSelected ? (HasFocus ? _Theme.GetSelectionTextColor() : _Theme.GetInactiveSelectionTextColor()) : ((IsHot || IsHighlighted) ? _Theme.GetHighlightTextColor() : _Theme.GetWindowTextColor());

                const DTTOPTS Options =
                {
                    .dwSize = sizeof(Options),
                    .dwFlags = DTT_TEXTCOLOR,
                    .crText = Color
                };

                rc.right = rc.left + (rcText.right - rcText.left);

                const auto hOldFont = ::SelectObject(hDC, _Theme.GetPlaylistFont());

                constexpr DWORD Flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX;

                ::DrawThemeTextEx(_Theme.GetTextStyle(), hDC, 0, 0, Text, -1, Flags, &rc, &Options);

                ::SelectObject(hDC, hOldFont);
            }

            return CDRF_SKIPDEFAULT; // Skip all other stages because we've drawn the complete item.
        }

        default:
        {
            SetMsgHandled(FALSE);

            return CDRF_DODEFAULT;
        }
    }
}

/// <summary>
/// Handles the NM_RCLICK notification.
/// </summary>
LRESULT playlist_uielement_t::OnRightClick(NMHDR * nmhd) noexcept
{
    const DWORD Position = ::GetMessagePos();

    POINT pt = { GET_X_LPARAM(Position), GET_Y_LPARAM(Position) };

    // In case the context menu gets called using the keyboard.
    if ((pt.x == -1) && (pt.y == -1))
    {
        RECT rc;

        ::GetWindowRect(m_hWnd, &rc);

        pt = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };
    }

    // Remember the item we're hovering over, if any.
    _hHighlightedtem = _TreeView.GetHighlightedItem(pt);

    const HMENU hMenu = ::LoadMenuW(THIS_HINSTANCE, MAKEINTRESOURCE(IDM_CONTEXT_MENU));

    if (hMenu == NULL)
        return FALSE;

    const HMENU hPopup = ::GetSubMenu(hMenu, 0);

    if (hPopup != NULL)
    {
        static_api_ptr_t<contextmenu_manager> ContextMenuManager;

        HMENU hPlaylist = NULL;

        const auto OnItem     = (_hHighlightedtem != NULL);
        const auto Node       = (node_t *) _TreeView.GetData(_hHighlightedtem);
        const auto Index      = (Node != nullptr) ? _PlaylistManager->find_playlist_by_guid(Node->Id) : SIZE_MAX;
        const bool IsPlaylist = (Index != SIZE_MAX);

        // Disable the Lock menu when we're not over a playlist.
        ::EnableMenuItem(hPopup, 4, (UINT) (MF_BYPOSITION | (IsPlaylist ? MF_ENABLED : MF_DISABLED | MF_GRAYED)));

        ::EnableMenuItem(hPopup, IDM_REMOVE, !IsProhibited(Node, playlist_lock::filter_remove_playlist) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

        if (IsPlaylist)
        {
            const auto FilterMask = _PlaylistManager->playlist_lock_get_filter_mask(Index);

            if (FilterMask != 0)
            {
                ::EnableMenuItem(hPopup, IDM_RENAME, !playlist_lock_t::IsRenamePlaylistProhibited(FilterMask) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

                // Update the Lock submenu.
                {
                    ::CheckMenuItem(hPopup, IDM_LOCK_ADD_ITEMS,      playlist_lock_t::IsAddProhibited(FilterMask)            ? MF_CHECKED : 0);
                    ::CheckMenuItem(hPopup, IDM_LOCK_REMOVE_ITEMS,   playlist_lock_t::IsRemoveProhibited(FilterMask)         ? MF_CHECKED : 0);
                    ::CheckMenuItem(hPopup, IDM_LOCK_REORDER_ITEMS,  playlist_lock_t::IsReorderProhibited(FilterMask)        ? MF_CHECKED : 0);
                    ::CheckMenuItem(hPopup, IDM_LOCK_REPLACE_ITEMS,  playlist_lock_t::IsReplaceProhibited(FilterMask)        ? MF_CHECKED : 0);

                    ::CheckMenuItem(hPopup, IDM_LOCK_RENAME,         playlist_lock_t::IsRenamePlaylistProhibited(FilterMask) ? MF_CHECKED : 0);
                    ::CheckMenuItem(hPopup, IDM_LOCK_REMOVE,         playlist_lock_t::IsRemovePlaylistProhibited(FilterMask) ? MF_CHECKED : 0);

                    ::CheckMenuItem(hPopup, IDM_LOCK_DEFAULT_ACTION, playlist_lock_t::IsDefaultActionProhibited(FilterMask)  ? MF_CHECKED : 0);

                    const auto IsOurLock = _LockManager->IsLockedByMe(Node->Id);

                    ::EnableMenuItem(hPopup, IDM_LOCK_ADD_ITEMS,      IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                    ::EnableMenuItem(hPopup, IDM_LOCK_REMOVE_ITEMS,   IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                    ::EnableMenuItem(hPopup, IDM_LOCK_REORDER_ITEMS,  IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                    ::EnableMenuItem(hPopup, IDM_LOCK_REPLACE_ITEMS,  IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

                    ::EnableMenuItem(hPopup, IDM_LOCK_RENAME,         IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                    ::EnableMenuItem(hPopup, IDM_LOCK_REMOVE,         IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

                    ::EnableMenuItem(hPopup, IDM_LOCK_DEFAULT_ACTION, IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

                    ::EnableMenuItem(hPopup, IDM_LOCK_ALL,            IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                    ::EnableMenuItem(hPopup, IDM_LOCK_NONE,           IsOurLock ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

                    pfc::string LockName;

                    if (!_PlaylistManager->playlist_lock_query_name(Index, LockName))
                        LockName = "<Unknown>";

                    const HMENU hLock = ::GetSubMenu(hPopup, 4);

                    ::AppendMenuW(hLock, MF_SEPARATOR, 0, NULL);
                    ::AppendMenuW(hLock, MF_STRING | MF_GRAYED | MF_DISABLED, 0, msc::FormatText(L"Locked with %S", LockName.c_str()).c_str());
                }
            }

            // Create and append the "Playlist" submenu. It contains the other foobar2000 context menu items for a playlist.
            {
                metadb_handle_list Handles;

                _PlaylistManager->playlist_get_all_items(Index, Handles);

                ContextMenuManager->init_context(Handles, contextmenu_manager::flag_show_shortcuts | contextmenu_manager::flag_view_full);

                hPlaylist = ::CreatePopupMenu();

                ContextMenuManager->win32_build_menu(hPlaylist, IDM_PLAYLIST, ~0);

                ::AppendMenuW(hPopup, MF_SEPARATOR, 0, NULL);
                ::AppendMenuW(hPopup, MF_STRING | MF_POPUP, (UINT_PTR) hPlaylist, L"Playlist");
            }

            // Append a menu item to show the UI of an autoplaylist.
            if (autoplaylist_manager::get()->is_client_present(Index))
                ::AppendMenuW(hPopup, MF_STRING, IDM_AUTOPLAYLIST, L"Autoplaylist...");
        }

        {
            ::EnableMenuItem(hPopup, IDM_SAVE,     IsPlaylist ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

            ::EnableMenuItem(hPopup, IDM_SAVE_ALL, !OnItem ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
            ::EnableMenuItem(hPopup, IDM_LOAD,     !OnItem ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        }

        // Create and append the Restore submenu.
        {
            const size_t RecycleCount = _PlaylistManager->recycler_get_count();

            if (RecycleCount != 0)
            {
                const HMENU hRestore = ::CreatePopupMenu();

                for (size_t RecycleIndex = 0; RecycleIndex < RecycleCount; ++RecycleIndex)
                {
                    pfc::string Name;

                    _PlaylistManager->recycler_get_name(RecycleIndex, Name);

                    ::AppendMenuW(hRestore, MF_STRING, IDM_HISTORY + RecycleIndex, msc::UTF8ToWide(Name.c_str()).c_str());
                }

                ::AppendMenuW(hRestore, MF_SEPARATOR, 0, NULL);
                ::AppendMenuW(hRestore, MF_STRING, IDM_CLEAR_HISTORY, L"Clear history");

                // Append the Restore menu to the popup menu.
                ::AppendMenuW(hPopup, MF_SEPARATOR, 0, NULL);

                const MENUITEMINFOW mii =
                {
                    .cbSize     = sizeof(mii),
                    .fMask      = MIIM_STRING | MIIM_SUBMENU,
                    .hSubMenu   = hRestore,
                    .dwTypeData = (LPWSTR) L"Restore",
                };

                ::InsertMenuItemW(hPopup, (UINT) ::GetMenuItemCount(hPopup), TRUE, &mii);
            }
        }

        // Append a troubleshooting menu item.
        if ((::GetKeyState(VK_CONTROL) & 0x8000) && (::GetKeyState(VK_SHIFT) & 0x8000))
        {
            ::AppendMenuW(hPopup, MF_SEPARATOR, 0, NULL);
            ::AppendMenuW(hPopup, MF_STRING, IDM_DUMP, L"Dump configuration");
        }

        const auto Command = (int) ::TrackPopupMenu(hPopup, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, 0, m_hWnd, nullptr); 

        ::PostMessageW(m_hWnd, WM_NULL, 0, 0);

        if (Command >= IDM_PLAYLIST)
            ContextMenuManager->execute_by_id((unsigned int) (Command - IDM_PLAYLIST));
        else
        if (Command > 0)
            ::PostMessageW(m_hWnd, WM_COMMAND, MAKEWPARAM(Command, 0), 0);

        if (hPlaylist != NULL)
            ::DestroyMenu(hPlaylist);

        SetMsgHandled(TRUE);
    }

    ::DestroyMenu(hMenu);

    return FALSE;
}

/// <summary>
/// Handles the NM_MCLICK notification.
/// </summary>
LRESULT playlist_uielement_t::OnMiddleClick(NMHDR * nmhd) noexcept
{
    const DWORD Position = ::GetMessagePos();

    const POINT pt = { GET_X_LPARAM(Position), GET_Y_LPARAM(Position) };

    _hHighlightedtem = _TreeView.GetHighlightedItem(pt);

    if (_hHighlightedtem == NULL)
        return -1;

    auto Node = (const node_t *) _TreeView.GetData(_hHighlightedtem);

    // Check for a playlist lock.
    if (IsProhibited(Node, playlist_lock::filter_remove_playlist))
        return -1;

    {
        auto Scope = toggle_t(_IsUser, true);

        _TreeView.RemoveItem(_hHighlightedtem);
    }

    SetMsgHandled(TRUE);

    return 0;
}

/// <summary>
/// Handles the NM_KILLFOCUS notification.
/// </summary>
LRESULT playlist_uielement_t::OnKillFocus(NMHDR * nmhd) noexcept
{
    _TreeView.EndDrag(true);

    return 0;
}

/// <summary>
/// Handles the TVN_GETDISPINFO notification.
/// </summary>
LRESULT playlist_uielement_t::OnGetDisplayInfo(NMHDR * nmhd) noexcept
{
    auto & tvi = ((NMTVDISPINFOW *) nmhd)->item;

    const auto Node = (node_t *) tvi.lParam;

    if (Node == nullptr)
        return FALSE;

    if (tvi.mask & TVIF_TEXT)
    {
        pfc::string Text;

        HRESULT hr = title_formatter_t::Evaluate(_State._TextFormat, &_TreeView, Node->Id, Text);

        if (!SUCCEEDED(hr))
            return FALSE;

        ::wcscpy_s(tvi.pszText, (size_t) tvi.cchTextMax, msc::UTF8ToWide(Text.c_str()).c_str());
    }

    if (tvi.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
    {
        auto Image = ItemImage::Playlist;

        if (Node->IsFolder)
        {
            Image = ItemImage::Folder;

            // Is the folder locked?
            if (IsProhibited(Node, playlist_lock::filter_remove_playlist))
                Image = ItemImage::FolderLocked;
        }
        else
        {
            Image = ItemImage::Playlist;

            // Is the playlist locked?
            auto Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if ((Index != SIZE_MAX) && _PlaylistManager->playlist_lock_is_present(Index))
                Image = ItemImage::PlaylistLocked;

            // Is the playlist playing?
            if (_IsPlaying)
            {
                Index = _PlaylistManager->get_playing_playlist();

                if (Index != SIZE_MAX)
                {
                    const auto Id = _PlaylistManager->playlist_get_guid(Index);

                    if (Id == Node->Id)
                        Image = ItemImage::PlaylistPlaying;
                }
            }
        }

        tvi.iImage = tvi.iSelectedImage = Image;
    }

    SetMsgHandled(TRUE);

    return FALSE;
}

/// <summary>
/// Handles the TVN_SELCHANGED notification.
/// </summary>
LRESULT playlist_uielement_t::OnSelectionChanged(NMHDR * nmhd) noexcept
{
    const auto nmtv = (NMTREEVIEWW *) nmhd;

    const auto Node = (node_t *) nmtv->itemNew.lParam;

    if (Node == nullptr)
        return FALSE;

    const size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

    if (Index == SIZE_MAX)
        return FALSE;

    _PlaylistManager->set_active_playlist(Index);

    SetMsgHandled(FALSE);

    return FALSE;
}

/// <summary>
/// Handles the TVN_DELETEITEM notification.
/// </summary>
LRESULT playlist_uielement_t::OnDeletingItem(NMHDR * nmhd) noexcept
{
    // Don't respond to Delete notifications caused by moving items after a drop.
    if (_TreeView.IsDragging())
        return FALSE;

    const auto nmtv = (NMTREEVIEWW *) nmhd;

    const auto Node = (node_t *) nmtv->itemOld.lParam;

    if (Node == nullptr)
        return FALSE;

    if (Node->IsFolder)
    {
        auto Scope = toggle_t(_IgnoreNotifications, true);

        _FolderManager->RemoveFolder(Node->Id);
    }
    else
    {
        // Remove the playlist only if triggered by a user action.
        if (_IsUser)
        {
            const size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == SIZE_MAX)
                return FALSE;

            {
                auto Scope = toggle_t(_IgnoreNotifications, true);

                _PlaylistManager->remove_playlist(Index);
            }
        }
    }

    delete Node;

    SetMsgHandled(FALSE);

    return FALSE;
}

/// <summary>
/// Handles the TVN_BEGINLABELEDIT notification.
/// </summary>
LRESULT playlist_uielement_t::OnBeginLabelEdit(NMHDR * nmhd) noexcept
{
    const auto nmdi = (NMTVDISPINFOW *) nmhd;

    const auto Node = (node_t *) nmdi->item.lParam;

    if (Node == nullptr)
        return TRUE;

    // Check for a playlist lock.
    if (IsProhibited(Node, playlist_lock::filter_rename))
        return TRUE;

    {
        const auto hEdit = _TreeView.GetEditControl();

        if (hEdit == NULL)
            return TRUE;

        _EditSubclass.Attach(hEdit);

        ::SetWindowTextW(hEdit, (LPCWSTR) msc::UTF8ToWide(Node->Name).c_str());

        ::SetFocus(hEdit);
    }

    SetMsgHandled(FALSE);

    return FALSE;
}

/// <summary>
/// Handles the TVN_ENDLABELEDIT notification.
/// </summary>
LRESULT playlist_uielement_t::OnEndLabelEdit(NMHDR * nmhd) noexcept
{
    const auto nmdi = (NMTVDISPINFOW *) nmhd;

    if (nmdi->item.pszText == nullptr)
        return FALSE;

    auto Node = (node_t *) nmdi->item.lParam;

    if (Node == nullptr)
        return FALSE;

    Node->Name = msc::WideToUTF8(nmdi->item.pszText);

    if (Node->IsFolder)
        _FolderManager->SetFolderName(Node->Id, Node->Name);
    else
    {
        const size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

        if (Index == SIZE_MAX)
            return FALSE;

        _PlaylistManager->playlist_rename(Index, Node->Name.c_str(), Node->Name.size());
    }

    // Recalculate the item rectangle.
    _TreeView.RefreshItem(Node->Id);

    SetMsgHandled(FALSE);

    return TRUE; // Keep the text.
}

/// <summary>
/// Handles the TVN_KEYDOWN notification.
/// </summary>
LRESULT playlist_uielement_t::OnKeyDown(NMHDR * nmhd) noexcept
{
    const auto Node = _TreeView.GetSelectedItem();

    if (Node == nullptr)
        return 0;

    const auto nmkd = (NMTVKEYDOWN *) nmhd;

    switch (nmkd->wVKey)
    {
        case VK_F2:
        {
            _TreeView.EditSelectedItem(); // The lock is checked in OnBeginLabelEdit().
            break;
        }

        case VK_DELETE:
        {
            if (IsProhibited(Node, playlist_lock::filter_remove_playlist))
                return 0;

            {
                auto Scope = toggle_t(_IsUser, true);

                _TreeView.RemoveSelectedItem();
            }
            break;
        }
    }

    SetMsgHandled(FALSE);

    return 0;
}

/// <summary>
/// Handles the TVN_GETINFOTIP notification.
/// </summary>
LRESULT playlist_uielement_t::OnGetInfoTip(NMHDR * nmhd) noexcept
{
    if (_State._ToolTipFormat.empty())
        return TRUE;

    auto nmgi = (NMTVGETINFOTIPW *) nmhd;

    const auto Node = (node_t *) nmgi->lParam;

    if (Node == nullptr)
        return TRUE;

    pfc::string Text;

    HRESULT hr = title_formatter_t::Evaluate(_State._ToolTipFormat, &_TreeView, Node->Id, Text);

    if (!SUCCEEDED(hr))
        return TRUE;

    ::wcscpy_s(nmgi->pszText, (rsize_t) nmgi->cchTextMax, msc::UTF8ToWide(Text.c_str()).c_str());

    SetMsgHandled(FALSE);

    return FALSE;
}

/// <summary>
/// Handles the TVN_BEGINDRAG notification.
/// </summary>
LRESULT playlist_uielement_t::OnBeginDrag(NMHDR * nmhd) noexcept
{
    const auto nmtv = (NMTREEVIEWW *) nmhd;

    _TreeView.BeginDrag(nmtv);

    SetMsgHandled(FALSE);

    return FALSE;
}

/// <summary>
/// Deserializes this instance from a JSON object.
/// </summary>
void playlist_uielement_t::FromJSON(json object) noexcept
{
    FromJSON(object, GUID_NULL);

    // Add all playlists that are missing in the loaded configuration to the root.
    const size_t PlaylistCount = _PlaylistManager->get_playlist_count();

    for (size_t PlaylistIndex = 0; PlaylistIndex < PlaylistCount; ++PlaylistIndex)
    {
        const auto Id = _PlaylistManager->playlist_get_guid(PlaylistIndex);

        if (_TreeView.FindItem(Id) != NULL)
            continue;

        pfc::string Name;

        _PlaylistManager->playlist_get_name(PlaylistIndex, Name);

        _TreeView.AddItem(GUID_NULL, GUID_NULL, Id, Name.c_str(), false, false);
    }
}

/// <summary>
/// Deserializes the tree nodes from a JSON object.
/// </summary>
void playlist_uielement_t::FromJSON(json object, const GUID & parentId) noexcept
{
    // Add all nodes from the JSON object.
    const auto & Nodes = object["nodes"];

    for (auto Node : Nodes)
    {
        std::string IdText    = Node.value("id",   "");
        std::string Name      = Node.value("name", "");

        const bool IsFolder   = Node.value("isFolder",   false);
        const bool IsExpanded = Node.value("isExpanded", false);

        const uint32_t FilterMask = Node.value("filterMask", 0u);

        const GUID Id = msc::UTF8ToGUID(IdText);

        if (Id == GUID_NULL)
            continue; // Should not happen...

        if (IsFolder)
        {
            {
                auto Scope = toggle_t(_IgnoreNotifications, true);

                _FolderManager->CreateFolder(Id, Name);
            }

            _TreeView.AddItem(parentId, { }, Id, Name, IsFolder, IsExpanded);

            const auto & Children = Node["nodes"];

            if (!Children.is_null())
                FromJSON(Node, Id);
        }
        else
        {
            const size_t Index = _PlaylistManager->find_playlist_by_guid(Id);

            if (Index == SIZE_MAX)
                continue; // TODO: Use a grayed out image to indicate this playlist was removed when the component was not installed and add a command to restore it.

            // Restore our lock.
            _LockManager->LockPlaylist(Id, FilterMask);

            _TreeView.AddItem(parentId, GUID_NULL, Id, Name, IsFolder, IsExpanded);
        }
    }
}

/// <summary>
/// Selects the specified playlist in the tree view.
/// </summary>
void playlist_uielement_t::SelectPlaylist(size_t index) const noexcept
{
    const auto Id = _PlaylistManager->playlist_get_guid(index);

    _TreeView.SelectItem(Id);
}

/// <summary>
/// Determines the drop effect.
/// </summary>
DWORD playlist_uielement_t::GetDropEffect(DWORD keyState, const POINT & pt) noexcept
{
    const auto hItem = _TreeView.GetHighlightedItem(pt);

    // Highlight the drop target.
    if (hItem != _hDropTarget)
    {
        _TreeView.SetState(_hDropTarget, 0, TVIS_DROPHILITED);

        _hDropTarget = hItem;

        _TreeView.SetState(_hDropTarget, TVIS_DROPHILITED, TVIS_DROPHILITED);
    }

    // Determine the effect.
    const auto Node = (node_t *) _TreeView.GetData(hItem);

    if (IsProhibited(Node, playlist_lock::filter_add))
        return DROPEFFECT_NONE;

    if (keyState & MK_CONTROL)
        return DROPEFFECT_MOVE;

    return DROPEFFECT_COPY;
}

/// <summary>
/// Drops the specified data object on the tree view.
/// </summary>
void playlist_uielement_t::DropFiles(IDataObject * dataObject) noexcept
{
    _hHighlightedtem = _hDropTarget;

    const auto Node = (node_t *) _TreeView.GetData(_hDropTarget);

    _hDropTarget = NULL;

    // Create or find the playlist that will receive the dropped items.
    size_t Index = SIZE_MAX;

    {
        if ((Node == nullptr) || Node->IsFolder)
            Index = _PlaylistManager->create_playlist_autoname();
        else
            Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

        if (Index == SIZE_MAX)
            return;
    }

    ::SetCursor(::LoadCursorW(NULL, IDC_APPSTARTING));

    static_api_ptr_t<playlist_incoming_item_filter_v2>()->process_dropped_files_async
    (
        dataObject,
        playlist_incoming_item_filter_v2::op_flag_delay_ui,
        core_api::get_main_window(),
        new service_impl_t<incoming_item_filter_callback_t>(Index, true)
    );

    _PlaylistManager->set_active_playlist(Index);

    ::SetCursor(::LoadCursorW(NULL, IDC_ARROW));
}

/// <summary>
/// Refreshes this instance.
/// </summary>
void playlist_uielement_t::Refresh() noexcept
{
    _Theme.Initialize(m_hWnd);

    HRESULT hr = InitImageList();

    if (!SUCCEEDED(hr))
        Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to initialize image list: 0x%08X.", hr);

    _TreeView.RefreshAllItems();

    // Quick Search visible or not?
    {
        _EditBox.ShowWindow(_State._IsQuickSearchVisible ? SW_SHOW : SW_HIDE);

        RECT rc;

        ::GetClientRect(m_hWnd, &rc);

        ::SendMessageW(m_hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));
    }
}

/// <summary>
/// Handles a change of the fonts.
/// </summary>
void playlist_uielement_t::OnFontsChanged() noexcept
{
    __super::OnFontsChanged();

    _TreeView.SetFont(_Theme.GetPlaylistFont());
    _EditBox.SetFont(_Theme.GetPlaylistFont());
}

/// <summary>
/// Sets the configuration.
/// </summary>
void playlist_uielement_t::SetConfiguration(const char * data, size_t size) noexcept
{
    try
    {
        _State.FromJSON(data, size);
    }
    catch (exception & ex)
    {
        Log.AtError().Write(STR_COMPONENT_BASENAME " failed to read configuration: %s", ex.what());

        _State.Reset();
    }
}

/// <summary>
/// Gets the configuration.
/// </summary>
std::string playlist_uielement_t::GetConfiguration() const noexcept
{
    try
    {
        // Save the state to a JSON object.
        auto Object = _State.ToJSON();

        json::array_t Nodes;

        _TreeView.ToJSON([&](HTREEITEM hItem, json::object_t * node) -> bool
        {
            const auto Node = (node_t *) _TreeView.GetData(hItem);

            if ((Node == nullptr) || ((Node != nullptr) && (Node->Id == GUID_NULL)))
                return true; // Continue walking. Should not occur.

            uint32_t FilterMask = 0;

            _LockManager->GetFilterMask(Node->Id, FilterMask);

            (*node)["id"]       = msc::GUIDToUTF8(Node->Id);
            (*node)["name"]     = Node->Name;
            (*node)["isFolder"] = Node->IsFolder;

            if (Node->IsFolder)
                (*node)["isExpanded"] = _TreeView.IsExpanded(Node->Id);

            (*node)["filterMask"] = FilterMask;

            return true; // Continue walking.
        }, &Nodes);

        Object["nodes"] = Nodes;

        #ifdef _DEBUG
//      console::print(Object.dump(4).c_str());
        #endif

        const auto Config = Object.dump(-1);

        return Config;
    }
    catch (exception & ex)
    {
        Log.AtError().Write(STR_COMPONENT_BASENAME " failed to write configuration: %s", ex.what());

        return std::string();
    }
}

/// <summary>
/// Gets the default configuration.
/// </summary>
std::string playlist_uielement_t::GetDefaultConfiguration() noexcept
{
    state_t DefaultState;

    const auto Config = DefaultState.ToJSON().dump(-1);

    return Config;
}

/// <summary>
/// Initializes the image list.
/// </summary>
HRESULT playlist_uielement_t::InitImageList() noexcept
{
    _ImageList.Reset();

    _ImageList = ::ImageList_Create((int) _State._ImageSize, (int) _State._ImageSize, ILC_COLOR32 | ILC_MASK, (int) _State._Images.size(), 0);

    if (!_ImageList)
        return HRESULT_FROM_WIN32(::GetLastError());

    std::unordered_map<std::string, imagelist_t> ImageLists;

    for (const auto & Image : _State._Images)
    {
        const auto LowerView = Image._FilePath | std::views::transform([](unsigned char c) { return (char) std::tolower(c); });

        const std::string Lower(LowerView.begin(), LowerView.end());

        // Only create an image list if the file path hasn't been seen yet.
        imagelist_t * SrcImageList = nullptr;

        const auto Iter = ImageLists.find(Lower);

        if (Iter == ImageLists.end())
        {
            imagelist_t NewImageList = image_list_factory_t::Create(Image._FilePath, _State._ImageSize);

            if (!NewImageList)
                return HRESULT_FROM_WIN32(::GetLastError());

            auto Result = ImageLists.emplace(Lower, std::move(NewImageList));

            SrcImageList = &Result.first->second;
        }
        else
            SrcImageList = &Iter->second;

        icon_t Icon = ::ImageList_GetIcon(*SrcImageList, (int) Image._IconIndex, ILD_TRANSPARENT);

        if (!Icon)
            return HRESULT_FROM_WIN32(::GetLastError());

        ::ImageList_ReplaceIcon(_ImageList, -1, Icon);
    }

    _TreeView.SetNormalImageList(_ImageList);
    _TreeView.SetStateImageList(_ImageList);

    return S_OK;
}

/// <summary>
/// Modifies the filter mask of one of our locks.
/// </summary>
void playlist_uielement_t::ModifyFilterMask(uint32_t newFilterMask) const noexcept
{
    const auto Node = (node_t *) _TreeView.GetData(_hHighlightedtem);

    if (Node == nullptr)
        return;

    const auto Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

    if (Index == SIZE_MAX)
        return;

    auto FilterMask = _PlaylistManager->playlist_lock_get_filter_mask(Index);

    if ((FilterMask != 0u) && !_LockManager->IsLockedByMe(Node->Id))
        return; // The playlist is locked but not by me.

    // Modify the lock.
    if ((newFilterMask == 0u) || (newFilterMask == ~0u))
        FilterMask = newFilterMask;
    else
    if (FilterMask & newFilterMask)
        FilterMask &= ~newFilterMask;
    else
        FilterMask |= newFilterMask;

    if (FilterMask != 0u)
    {
        HRESULT hr = _LockManager->LockPlaylist(Node->Id, FilterMask);

        if (!SUCCEEDED(hr))
            Log.AtError().Write(STR_COMPONENT_BASENAME " failed to lock playlist %d: 0x%08X.", Index, hr);
    }
    else
    {
        HRESULT hr = _LockManager->UnlockPlaylist(Node->Id);

        if (!SUCCEEDED(hr))
            Log.AtError().Write(STR_COMPONENT_BASENAME " failed to unlock playlist %d: 0x%08X.", Index, hr);
    }
}

/// <summary>
/// Returns true if the specified action is prohibited.
/// </summary>
bool playlist_uielement_t::IsProhibited(const node_t * node, uint32_t filterMask) const noexcept
{
    if (node == nullptr)
        return false;

    if (node->IsFolder)
    {
        // Only check the "Remove Playlist" lock for a folder.
        if ((filterMask & playlist_lock::filter_remove_playlist) == 0)
            return false;

        // A folder should not be removed if it contains at least one playlist that has a removal lock.
        bool FoundLockedPlaylist = false;

        auto StartNode = node;

        _TreeView.WalkBranch([&](node_t * node) -> bool
        {
            if (IsProhibited(node, playlist_lock::filter_remove_playlist))
            {
                FoundLockedPlaylist = true;

                return false; // Stop walking.
            }

            return true; // Continue walking.
        }, StartNode);

        return FoundLockedPlaylist;
    }
    else
    {
        const size_t Index = _PlaylistManager->find_playlist_by_guid(node->Id);

        if (Index == SIZE_MAX)
            return false;

        const auto FilterMask = _PlaylistManager->playlist_lock_get_filter_mask(Index);

        return ((FilterMask & filterMask) != 0);
    }
}

/// <summary>
/// Calculates the ideal height of the edit box.
/// </summary>
LONG playlist_uielement_t::CalculateEditHeight(HWND hWnd, HFONT hFont) noexcept
{
    HDC hDC = ::GetDC(hWnd);

    auto hOldFont = ::SelectObject(hDC, hFont);

    TEXTMETRIC tmFont = {};

    ::GetTextMetricsW(hDC, &tmFont);

    auto hSysFont = (HFONT) ::GetStockObject(SYSTEM_FONT);

    (void) ::SelectObject(hDC, hSysFont);

    TEXTMETRIC tmSysFont = {};

    ::GetTextMetricsW(hDC, &tmSysFont);

    (void) ::SelectObject(hDC, hOldFont);

    ::ReleaseDC(hWnd, hDC);

    const LONG Height = tmFont.tmHeight + (std::min(tmFont.tmHeight, tmSysFont.tmHeight) / 2) + (::GetSystemMetrics(SM_CYEDGE) * 2);

    return Height;
}

/// <summary>
/// Resets AutoComplete.
/// </summary>
void playlist_uielement_t::ResetAutoComplete() noexcept
{
    if (_StringEnumerator == nullptr)
        return;

    Log.AtDebug().Write(STR_COMPONENT_BASENAME " is resetting auto complete.");

    _StringEnumerator->Clear();

    _TreeView.Walk([&](node_t * node) -> bool
    {
        _StringEnumerator->AddItem(msc::UTF8ToWide(node->Name));

        return true; // Continue walking.
    });

    try { _EditBox.SetWindowTextW(L""); } catch (...) { }
}

tracker_t<playlist_uielement_t> _UIElementTracker;
