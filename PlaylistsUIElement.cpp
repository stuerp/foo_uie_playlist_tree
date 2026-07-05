
/** $VER: PlaylistsUIElement.cpp (2026.07.05) P. Stuer **/

#include "pch.h"

#include "PlaylistsUIElement.h"
#include "ImageList.h"
#include "TitleFormat.h"

#include <SDK\playlist.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
playlists_uielement_t::playlists_uielement_t()
{
    playlist_manager::get()->register_callback(this, (t_uint32) flag_all);
}

/// <summary>
/// Deletes this instance.
/// </summary>
playlists_uielement_t::~playlists_uielement_t()
{
    playlist_manager::get()->unregister_callback(this);
}

/// <summary>
/// Creates the window.
/// </summary>
LRESULT playlists_uielement_t::OnCreate(CREATESTRUCT * cs) noexcept
{
_State.Reset(); // TODO: Remove me

    auto Result = uielement_t::OnCreate(cs);

    if (Result != 0)
        return Result;

    if (!_TreeView.Create(m_hWnd, IDC_TREEVIEW))
        return -1;

    _DarkMode.AddCtrlAuto(_TreeView.GetHandle());

    {
        const auto IconSize = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

        HIMAGELIST hImageList = image_list_factory_t::Create(L"imageres.dll", IconSize);

        if (hImageList == NULL)
            return false;

        _TreeView.SetImageList(hImageList);
    }

    GetPlaylists();

    {
        size_t Index = playlist_manager::get()->get_active_playlist();

        if (Index != ~0u)
        {
            HTREEITEM hTreeItem = _Items[Index];

            if (hTreeItem != NULL)
                _TreeView.SelectItem(hTreeItem);
        }
    }

    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void playlists_uielement_t::OnDestroy() noexcept
{
    _TreeView.Destroy();

    uielement_t::OnDestroy();
}

/// <summary>
/// Handles the WM_SIZE message.
/// </summary>
void playlists_uielement_t::OnSize(UINT type, CSize size) noexcept
{
    uielement_t::OnSize(type, size);

    ::MoveWindow(_TreeView.GetHandle(), 0, 0, size.cx, size.cy, TRUE);
}

/// <summary>
/// Handles the WM_COMMAND message.
/// </summary>
void playlists_uielement_t::OnCommand(_In_ UINT notifyCode, _In_ int id, _In_ CWindow wnd) noexcept
{
    switch (id)
    {
        case IDM_NEW_FOLDER:
        {
            HTREEITEM hTreeItem = _TreeView.Additem(L"New Folder", 0, Icon::Folder);

            if (hTreeItem != NULL)
                _Items.push_back(hTreeItem);
            break;
        }

        case IDM_NEW_PLAYLIST:
        {
            HTREEITEM hTreeItem = _TreeView.Additem(L"New Playlist", 0, Icon::File);

            if (hTreeItem != NULL)
                _Items.push_back(hTreeItem);
            break;
        }
    }
}

/// <summary>
/// Handles the WM_NOTIFY message.
/// </summary>
LRESULT playlists_uielement_t::OnNotify(_In_ int id, _In_ NMHDR * nmhd) noexcept
{
    if (nmhd->idFrom != IDC_TREEVIEW)
        return 0;

    switch (nmhd->code)
    {
        case NM_RCLICK:
        {
            const DWORD Position = ::GetMessagePos();

            const POINT pt = { GET_X_LPARAM(Position), GET_Y_LPARAM(Position) };

            {
                const TVHITTESTINFO ht = { .pt = pt };

                HTREEITEM hTreeItem = (HTREEITEM) ::SendMessageW(_TreeView.GetHandle(), TVM_HITTEST, 0, (LPARAM) &ht);

                if (hTreeItem != NULL)
                    ::SendMessageW(_TreeView.GetHandle(), TVM_SELECTITEM, TVGN_CARET, (LPARAM) hTreeItem);
            }

            {
                const HMENU hMenu = ::LoadMenuW(THIS_HINSTANCE, MAKEINTRESOURCE(IDM_CONTEXT_MENU));

                if (hMenu == NULL)
                    break;

                const HMENU hPopup = ::GetSubMenu(hMenu, 0);

                if (hPopup != NULL)
                {
                    ::TrackPopupMenu(hPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);

                    ::PostMessageW(m_hWnd, WM_NULL, 0, 0);
                }

                ::DestroyMenu(hMenu);
            }
            break;
        }

        case TVN_SELCHANGED:
        {
            const auto nmtv = (NMTREEVIEWW *) nmhd;

            const auto Index = (size_t) nmtv->itemNew.lParam;

            playlist_manager::get()->set_active_playlist(Index);
            break;
        }
    }

    return 0;
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_added(t_size p_playlist,t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_reordered(t_size p_playlist,const t_size * p_order,t_size p_count) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_removing(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_removed(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_selection_change(t_size p_playlist,const bit_array & p_affected,const bit_array & p_state) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_item_focus_change(t_size p_playlist,t_size p_from,t_size p_to) noexcept
{}
	
/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_modified(t_size p_playlist,const bit_array & p_mask) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_modified_fromplayback(t_size p_playlist,const bit_array & p_mask,play_control::t_display_level p_level) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_replaced(t_size p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_item_ensure_visible(t_size p_playlist,t_size p_idx) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlist_activate(t_size oldIndex, t_size newIndex) noexcept
{
    _TreeView.SelectItem(_Items[newIndex]);
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlist_created(t_size index, const char * name, t_size size) noexcept
{
    if ((index == ~0u) || (name == nullptr) || (size == 0))
        return;

    const size_t ItemCount = playlist_manager::get()->playlist_get_item_count(index);

    const std::wstring Text = msc::FormatText(L"%S (%d)", name, ItemCount);

    _TreeView.Additem(Text, index, Icon::File);
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlists_reorder(const t_size * p_order,t_size p_count) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlists_removing(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlists_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlist_renamed(t_size p_index,const char * p_new_name,t_size p_new_name_len) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_default_format_changed() noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playback_order_changed(t_size p_new_index) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlist_locked(t_size p_playlist,bool p_locked) noexcept
{
}

/// <summary>
/// Gets all the playlists and adds them to the treeview.
/// </summary>
void playlists_uielement_t::GetPlaylists() noexcept
{
    static_api_ptr_t<playlist_manager> pm;

    const size_t PlaylistCount = pm->get_playlist_count();

    for (size_t PlaylistIndex = 0; PlaylistIndex < PlaylistCount; ++PlaylistIndex)
    {
        pfc::string Result;

        HRESULT hResult = title_formatter_t::Evaluate(_State._NameFormat, PlaylistIndex, Result);

        if (!SUCCEEDED(hResult))
            return;

        {
            auto hTreeItem = _TreeView.Additem(msc::UTF8ToWide(Result.c_str()), PlaylistIndex, Icon::File);

            if (hTreeItem != NULL)
                _Items.push_back(hTreeItem);
        }
    }
}
