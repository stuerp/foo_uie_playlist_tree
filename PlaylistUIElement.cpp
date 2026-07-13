
/** $VER: PlaylistsUIElement.cpp (2026.07.13) P. Stuer **/

#include "pch.h"

#include "PlaylistUIElement.h"
#include "DropTarget.h"
#include "ImageList.h"
#include "TitleFormat.h"
#include "Node.h"
#include "State.h"
#include "Theme.h"
#include "Log.h"

#include <SDK\playlist.h>
#include <SDK\metadb.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
playlist_uielement_t::playlist_uielement_t()
{
    _PlaylistManager->register_callback(this, (t_uint32) flag_all);
}

/// <summary>
/// Deletes this instance.
/// </summary>
playlist_uielement_t::~playlist_uielement_t()
{
    _PlaylistManager->unregister_callback(this);

    OnDestroy();
}

/// <summary>
/// Creates the window.
/// </summary>
LRESULT playlist_uielement_t::OnCreate(CREATESTRUCT * cs) noexcept
{
    auto Result = uielement_t::OnCreate(cs);

    if (Result != 0)
        return Result;

    // Create the tree view.
    {
        if (!_TreeView.Create(m_hWnd, IDC_TREEVIEW))
            return -1;

        _DarkMode.AddCtrlAuto(_TreeView.Get());

        if (!InitImageList())
            return -1;
    }

//  _State._Object.clear(); // Uncomment to reset the state.

    FromJSON(_State._Object);

    // Select the active playlist.
    {
        size_t Index = _PlaylistManager->get_active_playlist();

        if (Index != SIZE_MAX)
            SelectPlaylist(Index);
    }

    // Create the drop target.
    {
        HRESULT hResult = ::OleInitialize(nullptr);

        _DropTarget = new drop_target_t(m_hWnd, this);

        hResult = ::RegisterDragDrop(m_hWnd, _DropTarget);

        if (!SUCCEEDED(hResult))
            Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to register drop target: 0x08X", hResult);
    }

   _UIElementTracker.Add(this);

    return 0;
}

/// <summary>
/// Destroys the window.
/// </summary>
void playlist_uielement_t::OnDestroy() noexcept
{
    _UIElementTracker.Remove(this);

    // Destroy the drop target.
    {
        ::RevokeDragDrop(m_hWnd);

        _DropTarget->Release();

        ::OleUninitialize();
    }

    // Destroy the tree view.
    {
        _hImageList.Reset();

        _TreeView.Destroy();
    }

    uielement_t::OnDestroy();
}

/// <summary>
/// Handles the WM_SIZE message.
/// </summary>
void playlist_uielement_t::OnSize(UINT type, CSize size) noexcept
{
    uielement_t::OnSize(type, size);

    ::MoveWindow(_TreeView.Get(), 0, 0, size.cx, size.cy, TRUE);
}

/// <summary>
/// Handles the WM_COMMAND message.
/// </summary>
void playlist_uielement_t::OnCommand(UINT notifyCode, int id, CWindow wnd) noexcept
{
    switch (id)
    {
        // Handles the "New Folder" command.
        case IDM_NEW_FOLDER:
        {
            // Create the new Folder item.
            std::string Name("New Folder");

            GUID Id;

            HRESULT hResult = ::CoCreateGuid(&Id);

            if (!SUCCEEDED(hResult))
                return;

            _FolderManager->CreateFolder(Id, Name);

            // Get the data of the item we were hovering over, if any.
            auto Parent = (const node_t *) _TreeView.GetData(_hPopupItem);

            // Add the item.
            auto ParentId = GUID();
            auto InsertAfterId = GUID();

            if (Parent != nullptr)
            {
                if (Parent->IsFolder)
                    ParentId = Parent->Id;
                else
                    InsertAfterId = Parent->Id;
            }

            _TreeView.AddItem(ParentId, InsertAfterId, Id, Name, true, false);

            _hPopupItem = NULL;
            break;
        }

        // Handles the "New Playlist" command.
        case IDM_NEW_PLAYLIST:
        {
/*
            pfc::string Name("New Playlist");

            size_t NewIndex = _PlaylistManager->create_playlist(Name, Name.length(), SIZE_MAX);
*/
            size_t NewIndex = _PlaylistManager->create_playlist_autoname();

            if (NewIndex == SIZE_MAX)
                break;
            break;
        }

        // Handles the "Rename" command.
        case IDM_RENAME:
        {
            _TreeView.EditSelectedItem();
            break;
        }

        // Handles the "Remove" command.
        case IDM_REMOVE:
        {
            _IsUser = true;

            _TreeView.RemoveSelectedItem();

            _IsUser = false;
            break;
        }

        // Handles the "Sort" command.
        case IDM_SORT:
        {
            _TreeView.Sort(_hPopupItem);
            break;
        }
    }
}

/// <summary>
/// Handles the WM_NOTIFY message.
/// </summary>
LRESULT playlist_uielement_t::OnNotify(int id, NMHDR * nmhd) noexcept
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

            {
                // Remember the item we're hovering over, if any.
                _hPopupItem = _TreeView.GetItem(pt);

                // Uncomment if the item pointed to should become the selected item.
                //if (hTreeItem != NULL)
                //    _TreeView.SelectItem(hTreeItem);
            }

            {
                const HMENU hMenu = ::LoadMenuW(THIS_HINSTANCE, MAKEINTRESOURCE(IDM_CONTEXT_MENU));

                if (hMenu == NULL)
                    break;

                const HMENU hPopup = ::GetSubMenu(hMenu, 0);

                if (hPopup != NULL)
                {
                    const UINT State = (_hPopupItem != NULL) ? MF_ENABLED : MF_DISABLED | MF_GRAYED;

                    ::EnableMenuItem(hPopup, IDM_RENAME, State);
                    ::EnableMenuItem(hPopup, IDM_REMOVE, State);

                    ::TrackPopupMenu(hPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);

                    ::PostMessageW(m_hWnd, WM_NULL, 0, 0);
                }

                ::DestroyMenu(hMenu);
            }
            break;
        }

#ifdef CustomDraw
        case NM_CUSTOMDRAW:
        {
            auto tvcd = (const NMTVCUSTOMDRAW *) nmhd;

            const HWND hTreeView = tvcd->nmcd.hdr.hwndFrom;
            const HDC hDC        = tvcd->nmcd.hdc;

            switch (tvcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                {
                    return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.
                }

                case CDDS_ITEMPREPAINT:
                {
                    const auto hItem = (HTREEITEM) tvcd->nmcd.dwItemSpec;

                    wchar_t Text[512];

                    const TVITEMEX tvi
                    {
                        .mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE,
                        .hItem      = hItem,
                        .pszText    = Text,
                        .cchTextMax = _countof(Text),
                    };

                    TreeView_GetItem(hTreeView, &tvi);

                    const RECT & rcItem = tvcd->nmcd.rc;

                    // Draw the background.
                    if (tvi.state & TVIS_SELECTED)
                    {
                        HBRUSH hBrush = ::CreateSolidBrush(_Theme.GetColor(COLOR_HIGHLIGHT));

                        ::FillRect(hDC, &rcItem, hBrush);

                        ::DeleteObject(hBrush);
                    }
                    else
                    if (tvi.state & TVIS_DROPHILITED)
                    {
                        auto c1 = (int32_t) _Theme.GetColor(COLOR_WINDOW);
                        auto c2 = (int32_t) _Theme.GetColor(COLOR_HIGHLIGHT);

                        auto v1 = c1 & 0xFF;
                        auto v2 = c2 & 0xFF;

                        auto r = v1 + ::MulDiv(v2 - v1, 50, 100); c1 >>= 8; c2 >>= 8;

                        v1 = c1 & 0xFF;
                        v2 = c2 & 0xFF;

                        auto g = v1 + ::MulDiv(v2 - v1, 50, 100); c1 >>= 8; c2 >>= 8;

                        v1 = c1 & 0xFF;
                        v2 = c2 & 0xFF;

                        auto b = v1 + ::MulDiv(v2 - v1, 50, 100);

                        HBRUSH hBrush = ::CreateSolidBrush(RGB(r, g, b));

                        ::FillRect(hDC, &rcItem, hBrush);

                        ::DeleteObject(hBrush);
                    }

                    {
                        RECT rc = rcItem;

                        rc.left += 16 * tvcd->iLevel;

                        if (tvi.iImage == NodeType::Folder)
                        {
                            RECT rcChev = rc;

                            rcChev.right = rcChev.left + 16;

                            // Get the font height.
                            NONCLIENTMETRICSW ncm { sizeof(ncm) };

                            ::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

                            const LOGFONTW & lf = ncm.lfMessageFont;

                            // Create the font.
                            const HFONT hFont = ::CreateFontW(lf.lfHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe Fluent Icons");

                            const auto hOldFont = (HFONT) ::SelectObject(hDC, hFont);

                        //  const wchar_t * ChevronLeft  = L"\uE76B";
                            const wchar_t * ChevronRight = L"\uE76C";
                            const wchar_t * ChevronDown  = L"\uE70D";
                        //  const wchar_t * ChevronUp    = L"\uE70E";

                            const HTHEME hTheme = ::OpenThemeData(nullptr, L"TEXTSTYLE");

                            const DTTOPTS Options =
                            {
                                .dwSize = sizeof(Options),
                                .dwFlags = DTT_TEXTCOLOR,
                                .crText = _Theme.GetColor(COLOR_WINDOWTEXT)
                            };

                            ::DrawThemeTextEx(hTheme, hDC, 0, 0, (tvi.state & TVIS_EXPANDED) ? ChevronDown : ChevronRight, -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE, &rcChev, &Options);

                            ::CloseThemeData(hTheme);

                            ::SelectObject(hDC, hOldFont);

                            ::DeleteObject(hFont);
                        }

                        rc.left += 16;          // Skip past the chevron.

                        // Draw the image.
                        ::ImageList_Draw(_hImageList, tvi.iImage, hDC, rc.left, rcItem.top, ILD_NORMAL);

                        rc.left += 1 + 16 + 1;  // Skip past the icon.

                        // Draw the text.
                        const HTHEME hTheme = ::OpenThemeData(nullptr, L"TEXTSTYLE");

                        const DTTOPTS Options =
                        {
                            .dwSize = sizeof(Options),
                            .dwFlags = DTT_TEXTCOLOR,
                            .crText = (tvi.state & TVIS_SELECTED) ? _Theme.GetColor(COLOR_HIGHLIGHTTEXT) : _Theme.GetColor(COLOR_WINDOWTEXT)
                        };

                        ::DrawThemeTextEx(hTheme, hDC, 0, 0, Text, -1, DT_LEFT | DT_SINGLELINE, &rc, &Options);

                        ::CloseThemeData(hTheme);
                    }

                    // Draw the focus rectangle.
                    if (tvcd->nmcd.uItemState & CDIS_FOCUS)
                        ::DrawFocusRect(hDC, &rcItem);

                    return CDRF_SKIPDEFAULT; // Skip all other stages because we've drawn the complete item.
                }
            }

            break;
        }
#endif
        // Handles a need for display or sort info.
        case TVN_GETDISPINFO:
        {
            auto & tvi = ((NMTVDISPINFOW *) nmhd)->item;

            auto Node = (const node_t *) tvi.lParam;

            if (Node == nullptr)
                break;

            if (tvi.mask & TVIF_TEXT)
            {
                pfc::string Text;

                HRESULT hResult = title_formatter_t::Evaluate(_State._NameFormat, Node->Id, Text);

                if (!SUCCEEDED(hResult))
                    break;

                ::wcscpy_s(tvi.pszText, (size_t) tvi.cchTextMax, msc::UTF8ToWide(Text.c_str()).c_str());
            }

            if (tvi.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
            {
                if (Node->IsFolder)
                    tvi.iImage = tvi.iSelectedImage = ItemImage::Folder;
                else
                {
                    auto Image = ItemImage::Playlist;

                    size_t Index = _PlaylistManager->get_playing_playlist();

                    if (Index != SIZE_MAX)
                    {
                        const auto Id = _PlaylistManager->playlist_get_guid(Index);

                        if (Id == Node->Id)
                            Image = ItemImage::PlaylistPlaying;
                    }

                    tvi.iImage = tvi.iSelectedImage = Image;
                }
            }
            break;
        }

        // Handles a change of the selected item.
        case TVN_SELCHANGED:
        {
            const auto nmtv = (NMTREEVIEWW *) nmhd;

            const auto Node = (node_t *) nmtv->itemNew.lParam;

            if (Node == nullptr)
                break;

            const size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == SIZE_MAX)
                break;

            _PlaylistManager->set_active_playlist(Index);
            break;
        }

        // Handles a deletion of an item.
        case TVN_DELETEITEM:
        {
            // Don't respond to Delete notifications caused by moving items after a drop.
            if (_TreeView.IsDragging())
                break;

            const auto nmtv = (NMTREEVIEWW *) nmhd;

            auto Node = (node_t *) nmtv->itemOld.lParam;

            if (Node == nullptr)
                break;

            if (Node->IsFolder)
            {
                _FolderManager->RemoveFolder(Node->Id);
            }
            else
            {
                // Remove ths playlist only by a user action.
                if (_IsUser)
                {
                    const size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

                    if (Index == SIZE_MAX)
                        break;

                    _IsNotification = true;

                    _PlaylistManager->remove_playlist(Index);

                    _IsNotification = false;
                }
            }

            delete Node;
            break;
        }

        // Handles the beginning of label editing.
        case TVN_BEGINLABELEDIT:
        {
            const auto nmdi = (NMTVDISPINFOW *) nmhd;

            auto Node = (const node_t *) nmdi->item.lParam;

            if (Node == nullptr)
                return TRUE;

            auto hEdit = _TreeView.GetEditControl();

            if (hEdit == NULL)
                return TRUE;

            _EditSubclass.Attach(hEdit);

            ::SetWindowTextW(hEdit, (LPCWSTR) msc::UTF8ToWide(Node->Name).c_str());

            ::SetFocus(hEdit);

            return FALSE;
        }

        // Handles the completion or cancellation of label editing.
        case TVN_ENDLABELEDIT:
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
                size_t Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

                if (Index == SIZE_MAX)
                    return FALSE;

                _PlaylistManager->playlist_rename(Index, Node->Name.c_str(), Node->Name.size());
            }

            return TRUE; // Keep the text.
        }

        // Handles the notification that the user pressed a key and the tree-view control has the input focus. 
        case TVN_KEYDOWN:
        {
            const auto nmkd = (NMTVKEYDOWN *) nmhd;

            switch (nmkd->wVKey)
            {
                case VK_F2:
                {
                    _TreeView.EditSelectedItem();
                    break;
                }

                case VK_DELETE:
                {
                    _IsUser = true;

                    _TreeView.RemoveSelectedItem();

                    _IsUser = false;
                    break;
                }
            }

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

    return FALSE;
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

void playlist_uielement_t::OnDropFiles(HDROP hDropInfo) noexcept
{
    Log.Write("Drop files");
}

#pragma region playlist_callback

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_added(size_t playlistIndex, size_t start, const pfc::list_base_const_t<metadb_handle_ptr> & data, const bit_array & selection) noexcept
{
    auto Id = _PlaylistManager->playlist_get_guid(playlistIndex);

    _TreeView.RefreshItem(Id);
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_reordered(size_t playlistIndex, const size_t * order, size_t count) noexcept
{}

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
    auto Id = _PlaylistManager->playlist_get_guid(playlistIndex);

    _TreeView.RefreshItem(Id);
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_selection_change(size_t playlistIndex, const bit_array & p_affected, const bit_array & p_state) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_item_focus_change(size_t playlistIndex, size_t p_from, size_t p_to) noexcept
{}
	
/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_modified(size_t playlistIndex, const bit_array & p_mask) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_modified_fromplayback(size_t playlistIndex, const bit_array & p_mask,play_control::t_display_level p_level) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_replaced(size_t playlistIndex, const bit_array & p_mask, const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_item_ensure_visible(size_t playlistIndex, size_t p_idx) noexcept
{}

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

    pfc::string Name;

    _PlaylistManager->playlist_get_name(index, Name);

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    // Get the data of the item we were hovering over, if any.
    auto Parent = (const node_t *) _TreeView.GetData(_hPopupItem);

    // Add the item.
    auto ParentId = GUID();
    auto InsertAfterId = GUID();

    if (Parent != nullptr)
    {
        if (Parent->IsFolder)
            ParentId = Parent->Id;
        else
            InsertAfterId = Parent->Id;
    }

    _TreeView.AddItem(ParentId, InsertAfterId, Id, Name.c_str(), false, false);

    _hPopupItem = NULL;
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_playlists_reorder(const size_t * order, size_t count) noexcept
{
}

/// <summary>
/// Handles playlists that are being removed.
/// </summary>
void playlist_uielement_t::on_playlists_removing(const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
    for (size_t index = mask.find_first(true, 0, oldCount); index < oldCount; index = mask.find_next(true, index, oldCount))
    {
        const auto Id = _PlaylistManager->playlist_get_guid(index);

        if (!_IsNotification)
            _TreeView.RemoveItem(Id);
    }
}

/// <summary>
/// Handles a removed playlist.
/// </summary>
void playlist_uielement_t::on_playlists_removed(const bit_array & mask, size_t oldCount, size_t newCount) noexcept
{
}

/// <summary>
/// Handles a renamed playlist.
/// </summary>
void playlist_uielement_t::on_playlist_renamed(size_t index, const char * newName, size_t newSize) noexcept
{
    if ((index == SIZE_MAX) || (newName == nullptr) || (newSize == 0))
        return;

    const auto Id = _PlaylistManager->playlist_get_guid(index);

    pfc::string Name;

    _PlaylistManager->playlist_get_name(index, Name);

    _TreeView.SetName(Id, Name.c_str());
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
}

#pragma endregion

#pragma region play_callback

/// <summary>
/// Playback advanced to new track.
/// </summary>
void playlist_uielement_t::on_playback_new_track(metadb_handle_ptr track)
{
    _TreeView.RefreshAllItems();
}

/// <summary>
/// Playback stopped.
/// </summary>
void playlist_uielement_t::on_playback_stop(play_control::t_stop_reason reason)
{
    _TreeView.RefreshAllItems();
}

/// <summary>
/// Playback paused/resumed.
/// </summary>
void playlist_uielement_t::on_playback_pause(bool state)
{
    _TreeView.RefreshAllItems();
}

#pragma endregion

/// <summary>
/// Deserializes this instance from a JSON object.
/// </summary>
void playlist_uielement_t::FromJSON(json object) noexcept
{
    FromJSON(object, { });

    // Add all playlists that are missing to the root.
    const size_t PlaylistCount = _PlaylistManager->get_playlist_count();

    for (size_t PlaylistIndex = 0; PlaylistIndex < PlaylistCount; ++PlaylistIndex)
    {
        const auto Id = _PlaylistManager->playlist_get_guid(PlaylistIndex);

        if (_TreeView.FindItem(Id) != NULL)
            continue;

        pfc::string Name;

        _PlaylistManager->playlist_get_name(PlaylistIndex, Name);

        _TreeView.AddItem({ }, { }, Id, Name.c_str(), false, false);

        Log.Write("P %s", Name.c_str());
    }
}

/// <summary>
/// Deserializes this instance from a JSON object.
/// </summary>
void playlist_uielement_t::FromJSON(json object, const GUID & parentId) noexcept
{
    // Add all nodes from the JSON object.
    const auto & Nodes = object["nodes"];

    for (auto Node : Nodes)
    {
        std::string IdText = Node.value("id", IdText);
        std::string Name   = Node.value("name", Name);
/*
        const auto & Image = Node["image"];

        if (!Image.is_null())
        {
            std::string ImageFilePath = Image.value("filePath", ImageFilePath);
            int32_t ImageIndex        = Node.value("index", ImageIndex);
        }
*/
        bool IsFolder   = Node.value("isFolder", IsFolder);
        bool IsExpanded = Node.value("isExpanded", IsExpanded);

        GUID Id = msc::GUIDFromUTF8(IdText);

        if (IsFolder)
        {
            _FolderManager->CreateFolder(Id, Name);

            _TreeView.AddItem(parentId, { }, Id, Name, IsFolder, IsExpanded);

            const auto & Children = Node["nodes"];

            if (!Children.is_null())
                FromJSON(Node, Id);
        }
        else
        {
            const size_t Index = _PlaylistManager->find_playlist_by_guid(Id);

            if (Index == SIZE_MAX)
                continue; // TODO: Use a grayed out image to indicate this playlist is missing and add a command to restore it.

            _TreeView.AddItem(parentId, { }, Id, Name, IsFolder, IsExpanded);

            Log.Write("T %s", Name.c_str());
        }
    }
}

/// <summary>
/// Selects the specified playlist in the tree view.
/// </summary>
void playlist_uielement_t::SelectPlaylist(size_t index) const noexcept
{
    const auto Id = _PlaylistManager->playlist_get_guid(index);

    _TreeView.Walk([&](HTREEITEM hItem, void * context) -> bool
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

/// <summary>
/// Determines the drop effect.
/// </summary>
DWORD playlist_uielement_t::GetDropEffect(DWORD keyState, const POINT & pt) noexcept
{
    _hDropTarget = _TreeView.GetItem(pt);

    if (keyState & MK_CONTROL)
        return DROPEFFECT_MOVE;

    return DROPEFFECT_COPY;
}

/// <summary>
/// Drops the specified files an the tree view.
/// </summary>
void playlist_uielement_t::DropFiles(const std::vector<std::wstring> & filePaths) noexcept
{
    auto Node = (const node_t *) _TreeView.GetData(_hDropTarget);

    size_t Index = SIZE_MAX;

    {
        if ((Node == nullptr) || Node->IsFolder)
            Index = _PlaylistManager->create_playlist_autoname();
        else
            Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

        if (Index == SIZE_MAX)
            return;
    }

    {
        metadb_handle_list Handles;

        static_api_ptr_t<metadb_io> IO;

        for (const auto & FilePath : filePaths)
        {
            metadb_handle_list Temp;

            // Properly resolve paths (including archive subsongs, canonicalization, etc.)
            IO->path_to_handles_simple(msc::WideToUTF8(FilePath).c_str(), Temp);

            Handles.add_items(Temp);
        }

        // Add the handles to the playlist and select them.
        _PlaylistManager->playlist_add_items(Index, Handles, bit_array_true());
    }
}

/// <summary>
/// Refreshes this instance.
/// </summary>
void playlist_uielement_t::Refresh() noexcept
{
    InitImageList();

    _TreeView.RefreshAllItems();
}

/// <summary>
/// Sets the configuration.
/// </summary>
void playlist_uielement_t::SetConfiguration(const char * data, size_t size) noexcept
{
    _State.FromJSON(data, size);
}

/// <summary>
/// Gets the configuration.
/// </summary>
std::string playlist_uielement_t::GetConfiguration() const noexcept
{
    // Save the state to a JSON object.
    auto Object = _State.ToJSON();

    json::array_t Nodes;

    _TreeView.ToJSON([&](HTREEITEM hItem, json::object_t * node) -> bool
    {
        auto Node = (const node_t *) _TreeView.GetData(hItem);

        if (Node == nullptr)
            return true; // Continue enumerating. Should not occur.

        (*node)["id"]       = msc::GUIDToUTF8(Node->Id);
        (*node)["name"]     = Node->Name;
/*
        (*node)["image"]    =
        {
            { "filePath", "test" },
            { "index", 42 }
        };
*/
        (*node)["isFolder"] = Node->IsFolder;

        if (Node->IsFolder)
            (*node)["isExpanded"] = _TreeView.IsExpanded(Node->Id);

        return true; // Continue enumerating.
    }, &Nodes);

    Object["nodes"] = Nodes;

    #ifdef _DEBUG
    ::OutputDebugStringA(Object.dump(4).c_str());
    #endif

    const auto Config = Object.dump(-1);

    return Config;
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
bool playlist_uielement_t::InitImageList() noexcept
{
    _hImageList.Reset();

    const auto IconSize = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

    _hImageList = ::ImageList_Create((int) IconSize, (int) IconSize, ILC_COLOR32 | ILC_MASK, (int) _State._Images.size(), 0);

    if (!_hImageList)
        return false;

    for (const auto & Image : _State._Images)
    {
        himagelist_t hSrcImageList = image_list_factory_t::Create(Image._FilePath, IconSize);

        hicon_t hIcon = ::ImageList_GetIcon(hSrcImageList, (int) Image._IconIndex, ILD_TRANSPARENT);

        if (!hIcon)
            return false;

        ::ImageList_ReplaceIcon(_hImageList, -1, hIcon);
    }

    _TreeView.SetImageList(_hImageList);

    return true;
}

tracker_t<playlist_uielement_t> _UIElementTracker;
