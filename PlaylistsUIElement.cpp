
/** $VER: PlaylistsUIElement.cpp (2026.07.06) P. Stuer **/

#include "pch.h"

#include "PlaylistsUIElement.h"
#include "ImageList.h"
#include "TitleFormat.h"
#include "Node.h"

#include <SDK\playlist.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
playlists_uielement_t::playlists_uielement_t()
{
    _PlaylistManager->register_callback(this, (t_uint32) flag_all);
}

/// <summary>
/// Deletes this instance.
/// </summary>
playlists_uielement_t::~playlists_uielement_t()
{
    _PlaylistManager->unregister_callback(this);

    OnDestroy();
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

    _DarkMode.AddCtrlAuto(_TreeView.Get());

    {
        const auto IconSize = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

        HIMAGELIST hImageList = image_list_factory_t::Create(L"imageres.dll", IconSize);

        if (hImageList == NULL)
            return false;

        _TreeView.SetImageList(hImageList);
    }

    GetPlaylists();

    // Select the active playlist.
    {
        size_t Index = _PlaylistManager->get_active_playlist();

        if (Index != ~0llu)
            SelectPlaylist(Index);
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

    ::MoveWindow(_TreeView.Get(), 0, 0, size.cx, size.cy, TRUE);
}

/// <summary>
/// Handles the WM_COMMAND message.
/// </summary>
void playlists_uielement_t::OnCommand(_In_ UINT notifyCode, _In_ int id, _In_ CWindow wnd) noexcept
{
    switch (id)
    {
        // Handles the "New Folder" command.
        case IDM_NEW_FOLDER:
        {
            GUID Id;

            HRESULT hResult = ::CoCreateGuid(&Id);

            if (!SUCCEEDED(hResult))
                break;

            pfc::string Name;

            hResult = title_formatter_t::Evaluate(_State._NameFormat, Id, _TreeView, Name);

            if (!SUCCEEDED(hResult))
                return;

            std::wstring Text = msc::UTF8ToWide(Name.c_str());

            _TreeView.AddNode(Id, Text, true);
            break;
        }

        // Handles the "New Playlist" command.
        case IDM_NEW_PLAYLIST:
        {
/*
            HTREEITEM hTreeItem = (_hDropTarget != NULL) ? _hDropTarget : _TreeView.GetSelectedItem();

            _hDropTarget = NULL;

            const auto Node = (node_t *) _TreeView.GetData(hTreeItem);

            if (Node == nullptr)
                break;

            std::string Name("New Playlist");

            size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == ~0llu)
                break;
*/
            std::string Name("New Playlist");

            size_t NewIndex = _PlaylistManager->create_playlist(Name.c_str(), Name.size(), ~0llu);

            if (NewIndex == ~0llu)
                break;

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
        // Handles a right mouse button click within the control.
        case NM_RCLICK:
        {
            const DWORD Position = ::GetMessagePos();

            const POINT pt = { GET_X_LPARAM(Position), GET_Y_LPARAM(Position) };
/*
            {
                auto hTreeItem = _TreeView.GetItem(pt);

                // Uncomment if the item pointed to should become the selected item.
                //if (hTreeItem != NULL)
                //    _TreeView.SelectItem(hTreeItem);
            }
*/
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

        // Handles a key press within the control.
        case NM_KEYDOWN:
        {
            const auto nmtv = (NMKEY *) nmhd;

            if (nmtv->nVKey == VK_ESCAPE)
                _TreeView.EndDrag(true);

            break;
        }

        // Handles a change of the selected item.
        case TVN_SELCHANGED:
        {
            const auto nmtv = (NMTREEVIEWW *) nmhd;

            const auto Node = (node_t *) nmtv->itemNew.lParam;

            if (Node == nullptr)
                break;

            size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == ~0llu)
                break;

            _PlaylistManager->set_active_playlist(Index);
            break;
        }

        // Handles a deletion of an item.
        case TVN_DELETEITEM:
        {
            break;
        }

        // Handles the initiation of a drag-and-drop operation involving the left mouse button.
        case TVN_BEGINDRAG:
        {
            const auto nmtv = (NMTREEVIEWW *) nmhd;

            _TreeView.BeginDrag(nmtv);
            break;
        }
    }

    return 0;
}

/// <summary>
/// Handles the WM_MOUSEMOVE message.
/// </summary>
void playlists_uielement_t::OnMouseMove(_In_ UINT flags, _In_ CPoint point) noexcept
{
    _TreeView.DragMove(point);
}

/// <summary>
/// Handles the WM_MOUSELEAVE message.
/// </summary>
void playlists_uielement_t::OnMouseLeave() noexcept
{
    _TreeView.RemoveInsertMarker();
}

/// <summary>
/// Handles the WM_LBUTTONUP message.
/// </summary>
void playlists_uielement_t::OnLButtonUp(_In_ UINT flags, _In_ CPoint point) noexcept
{
    _TreeView.EndDrag(false);
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_added(size_t playlistIndex, size_t start, const pfc::list_base_const_t<metadb_handle_ptr> & data, const bit_array & selection) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_reordered(size_t playlistIndex, const size_t * order, size_t count) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_removing(size_t playlistIndex, const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_removed(size_t playlistIndex, const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_selection_change(size_t playlistIndex, const bit_array & p_affected, const bit_array & p_state) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_item_focus_change(size_t playlistIndex, size_t p_from, size_t p_to) noexcept
{}
	
/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_modified(size_t playlistIndex, const bit_array & p_mask) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_modified_fromplayback(size_t playlistIndex, const bit_array & p_mask,play_control::t_display_level p_level) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_items_replaced(size_t playlistIndex, const bit_array & p_mask, const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_item_ensure_visible(size_t playlistIndex, size_t p_idx) noexcept
{}

/// <summary>
/// Handles the activation of a new playlist.
/// </summary>
void playlists_uielement_t::on_playlist_activate(size_t oldIndex, size_t newIndex) noexcept
{
    SelectPlaylist(newIndex);
}

/// <summary>
/// Handles the creation of a new playlist.
/// </summary>
void playlists_uielement_t::on_playlist_created(size_t index, const char * name, size_t size) noexcept
{
    if ((index == ~0llu) || (name == nullptr) || (size == 0))
        return;

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    pfc::string Name;

    HRESULT hResult = title_formatter_t::Evaluate(_State._NameFormat, Id, _TreeView, Name);

    if (!SUCCEEDED(hResult))
        return;

    std::wstring Text = msc::UTF8ToWide(Name.c_str());

    _TreeView.AddNode(Id, Text, false);
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlists_reorder(const size_t * order, size_t count) noexcept
{
}

/// <summary>
/// Handles playlists that are being removed.
/// </summary>
void playlists_uielement_t::on_playlists_removing(const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
    for (size_t index = mask.find_first(true, 0, oldCount); index < oldCount; index = mask.find_next(true, index, oldCount))
    {
        const auto Id = _PlaylistManager->playlist_get_guid(index);

        _TreeView.RemoveNode(Id);
    }
}

/// <summary>
/// Handles a removed playlist.
/// </summary>
void playlists_uielement_t::on_playlists_removed(const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
}

/// <summary>
/// Handles a renamed playlist.
/// </summary>
void playlists_uielement_t::on_playlist_renamed(size_t index, const char * newName, size_t newSize) noexcept
{
    if ((index == ~0llu) || (newName == nullptr) || (newSize == 0))
        return;

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    pfc::string Name;

    HRESULT hResult = title_formatter_t::Evaluate(_State._NameFormat, Id, _TreeView, Name);

    if (!SUCCEEDED(hResult))
        return;

    std::wstring Text = msc::UTF8ToWide(Name.c_str());

    _TreeView.SetText(Id, Text);
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
void playlists_uielement_t::on_playback_order_changed(size_t p_new_index) noexcept
{
}

/// <summary>
/// 
/// </summary>
void playlists_uielement_t::on_playlist_locked(size_t index, bool isLocked) noexcept
{
}

/// <summary>
/// Gets all the playlists and adds them to the treeview.
/// </summary>
void playlists_uielement_t::GetPlaylists() noexcept
{
    const size_t PlaylistCount = _PlaylistManager->get_playlist_count();

    for (size_t PlaylistIndex = 0; PlaylistIndex < PlaylistCount; ++PlaylistIndex)
    {
        const auto Id = _PlaylistManager->playlist_get_guid(PlaylistIndex);

        pfc::string Name;

        HRESULT hResult = title_formatter_t::Evaluate(_State._NameFormat, Id, _TreeView, Name);

        if (!SUCCEEDED(hResult))
            return;

        std::wstring Text = msc::UTF8ToWide(Name.c_str());

        _TreeView.AddNode(Id, Text, false);
    }
}

/// <summary>
/// Selects the specified playlist in the tree view.
/// </summary>
void playlists_uielement_t::SelectPlaylist(size_t index) const noexcept
{
    const auto Id = _PlaylistManager->playlist_get_guid(index);

    _TreeView.Walk([&](HTREEITEM hItem) -> bool
    {
        auto Node = (node_t *) _TreeView.GetData(hItem);

        if ((Node != nullptr) && (Node->Id == Id))
        {
            _TreeView.SelectItem(hItem);

            return false;
        }

        return true; // Continue enumerating
    });
}
