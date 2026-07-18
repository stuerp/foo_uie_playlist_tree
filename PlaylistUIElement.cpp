
/** $VER: PlaylistsUIElement.cpp (2026.07.18) P. Stuer **/

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

        _TreeViewSubclass.Attach(_TreeView.Get());

        _DarkMode.AddCtrlAuto(_TreeView.Get());

        {
            HRESULT hResult = InitImageList();

            if (!SUCCEEDED(hResult))
                Log.Write(STR_COMPONENT_BASENAME " failed to initialize image lists: 0x%08X", hResult);
        }
    }

    // Deserialize the state.
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

        _DropTarget = new drop_target_t(_TreeView.Get(), this);

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

        if (_DropTarget != nullptr)
        {
            _DropTarget->Release();
            _DropTarget = nullptr;
        }

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
            const auto Parent = (node_t *) _TreeView.GetData(_hPopupItem);

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

            // Redraw the tree view. Note: Only required when using custom draw.
            ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
            ::UpdateWindow(_TreeView.Get());
            break;
        }

        // Handles the "New Playlist" command.
        case IDM_NEW_PLAYLIST:
        {
            size_t NewIndex = _PlaylistManager->create_playlist_autoname();

            if (NewIndex == SIZE_MAX)
                break;
            break;
        }

        // Handles the "Rename" command.
        case IDM_RENAME:
        {
            _TreeView.EditItem(_hPopupItem);
            break;
        }

        // Handles the "Remove" command.
        case IDM_REMOVE:
        {
            _IsUser = true;

            _TreeView.RemoveItem(_hPopupItem);

            _IsUser = false;
            break;
        }

        // Handles the "Sort" command.
        case IDM_SORT:
        {
            _TreeView.Sort(_hPopupItem);
            break;
        }

        // Handles the "Save all playlists..." command.
        case IDM_SAVE_ALL:
        {
            standard_commands::main_save_all_playlists();
            break;
        }

        // Handles the "Save playlist..." command.
        case IDM_SAVE:
        {
            const auto Node = (node_t *) _TreeView.GetData(_hPopupItem);

            if (Node == nullptr)
                break;

            auto Index = _PlaylistManager->find_playlist_by_guid(Node->Id);

            if (Index == SIZE_MAX)
                break;

            pfc::string FileName;

            if (!_PlaylistManager->playlist_get_name(Index, FileName))
                break;

            pfc::list_t<metadb_handle_ptr> Items;

            _PlaylistManager->playlist_get_all_items(Index, Items);

            pfc::string_formatter Extensions;
            uint32_t DefaultExtension = 0;

            {
                service_enum_t<playlist_loader> LoaderEnumerator;
                service_ptr_t<playlist_loader> Loader;

                uint32_t i = 0;

                while (LoaderEnumerator.next(Loader))
                {
                    if (Loader->can_write())
                    {
                        Extensions << Loader->get_extension() << " files|*." << Loader->get_extension() << "|";

                        if (!::stricmp_utf8(Loader->get_extension(), "fpl"))
                            DefaultExtension = i;
                        i++;
                    }
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
                    popup_message::g_show(e.what(), "Error writing playlist", popup_message::icon_error);
                }
            }
            break;
        }

        // Handles the "Load playlist..." command.
        case IDM_LOAD:
        {
            standard_commands::main_load_playlist();
            break;
        }

        // Handles the "Clear history" command.
        case IDM_CLEAR_HISTORY:
        {
            int Result = popup_message_v3::get()->messageBox(m_hWnd, "Delete removed playlists permanently?", STR_COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

            if (Result == IDYES)
                _PlaylistManager->recycler_purge(bit_array_true());
            break;
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
/// Handles the WM_NOTIFY message.
/// </summary>
LRESULT playlist_uielement_t::OnNotify(int id, NMHDR * nmhd) noexcept
{
    if (nmhd->idFrom != IDC_TREEVIEW)
        return 0;

    switch (nmhd->code)
    {
        // Handles a right mouse button click within the control.
        case NM_CUSTOMDRAW:
        {
        #ifndef FullCustomDraw
            const auto tvcd = (NMTVCUSTOMDRAW *) nmhd;

            const auto hTreeView = tvcd->nmcd.hdr.hwndFrom;
            const auto hDC       = tvcd->nmcd.hdc;

            switch (tvcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                {
        /*
                    // Draw the control background.
                    {
                        RECT rc;

                        ::GetClientRect(hTreeView, &rc);

                        ::FillRect(hDC, &rc, _Theme.GetWindowBrush());
                    }
        */
                    SetMsgHandled(TRUE);

                    return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.
                }

                case CDDS_ITEMPREPAINT:
                {
                    const RECT & rcItem = tvcd->nmcd.rc;

                    if ((rcItem.right - rcItem.left) == 0)
                        return CDRF_DODEFAULT;

                    const auto hItem = (HTREEITEM) tvcd->nmcd.dwItemSpec;

                    // Get information about the item.
                    wchar_t Text[512];

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
                    const auto IsSelected    = ((tvi.state & TVIS_SELECTED) != 0);
                    const auto IsHighlighted = ((tvi.state & TVIS_DROPHILITED) != 0);
                    const auto IsHot         = ((tvcd->nmcd.uItemState & CDIS_HOT) != 0);

                    // Get bounding rectangle of the item text.
                    RECT rcText;

                    TreeView_GetItemRect(hTreeView, hItem, &rcText, TRUE);

                    const LONG ImageSize = rcText.bottom - rcText.top;

                    // Clear the background of the full item.
                    {
                        HBRUSH hBrush = _Theme.GetWindowBrush();

                        ::FillRect(hDC, &rcItem, hBrush);
                    }

                    RECT rc = rcItem;

                    rc.left += ImageSize * tvcd->iLevel;

                    // Draw a chevron for a Folder node.
                    {
                        if (HasChildren)
                        {
                            RECT rcChev = rc;

                            rcChev.right = rcChev.left + ImageSize;

                            const auto hOldFont = (HFONT) ::SelectObject(hDC, _Theme.GetFont());

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

                            ::DrawThemeTextEx(_Theme.GetTextStyle(), hDC, 0, 0, (tvi.state & TVIS_EXPANDED) ? ChevronDown : ChevronRight, -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE, &rcChev, &Options);

                            ::SelectObject(hDC, hOldFont);
                        }

                        rc.left += ImageSize;
                    }

                    // Draw background.
                    {
                        rc.right = rc.left + 1 + ImageSize + 1 + 3 + (rcText.right - rcText.left);

                        if (IsSelected)
                        {
                            HBRUSH hBrush = HasFocus ? _Theme.GetSelectionBrush() : _Theme.GetInactiveSelectionBrush();

                            ::FillRect(hDC, &rc, hBrush);

                            // Draw the focus rectangle.
                            HPEN hPen = _Theme.GetWindowTextPen();

                            auto hOldBrush = ::SelectObject(hDC, hBrush);
                            auto hOldPen = ::SelectObject(hDC, hPen);

                            ::RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, 1, 1);

                            ::SelectObject(hDC, hOldPen);
                            ::SelectObject(hDC, hOldBrush);
                        }
                        else
                        if (IsHot || IsHighlighted)
                        {
                            HBRUSH hBrush = _Theme.GetHighlightBrush();

                            ::FillRect(hDC, &rc, hBrush);
                        }
                    }

                    // Draw the image.
                    {
                        const LONG dx = ((1 + ImageSize + 1) - 16) / 2;
                        const LONG dy = (ImageSize           - 16) / 2;

                        ::ImageList_Draw(_hImageList, tvi.iImage, hDC, rc.left + dx, rc.top + dy, ILD_NORMAL);

                        rc.left += 1 + ImageSize + 1 + 3;
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

                        ::DrawThemeTextEx(_Theme.GetTextStyle(), hDC, 0, 0, Text, -1, DT_LEFT | DT_SINGLELINE, &rc, &Options);
                    }

                    SetMsgHandled(TRUE);

                    return CDRF_SKIPDEFAULT; // Skip all other stages because we've drawn the complete item.
                }

                default:
                {
                    SetMsgHandled(FALSE);

                    return CDRF_DODEFAULT;
                }
            }
        #endif

        #ifdef SimpleCustomDraw
            if (_IsDUI)
                break;

            auto tvcd = (NMTVCUSTOMDRAW *) nmhd;

            const auto hTreeView = tvcd->nmcd.hdr.hwndFrom;
            const auto hDC        = tvcd->nmcd.hdc;

            switch (tvcd->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                {
                    // Draw the control background.
                    RECT rc;

                    ::GetClientRect(hTreeView, &rc);

                    HBRUSH hBrush = ::CreateSolidBrush(_Theme.GetWindowColor());

                    ::FillRect(hDC, &rc, hBrush);

                    ::DeleteObject(hBrush);

                    return CDRF_NOTIFYITEMDRAW; // Request item-specific notifications.
                }

                case CDDS_ITEMPREPAINT:
                {
                    const auto hItem = (HTREEITEM) tvcd->nmcd.dwItemSpec;

                    const TVITEMEX tvi
                    {
                        .mask       = TVIF_STATE,
                        .hItem      = hItem,
                        .stateMask = 0xFF,
                    };

                    TreeView_GetItem(hTreeView, &tvi);

                    const auto HasFocus      = (::GetFocus() == hTreeView);
                    const auto IsSelected    = ((tvi.state & TVIS_SELECTED) != 0);
                    const auto IsHighlighted = ((tvi.state & TVIS_DROPHILITED) != 0);
                    const auto IsHot         = ((tvcd->nmcd.uItemState & CDIS_HOT) != 0);

                    if (IsSelected || IsHighlighted)
                    {
                        tvcd->clrText   = HasFocus ? _Theme.GetSelectionTextColor() : _Theme.GetInactiveSelectionTextColor();
                        tvcd->clrTextBk = HasFocus ? _Theme.GetSelectionColor()     : _Theme.GetInactiveSelectionColor();
                    }
                    else
                    if (IsHot)
                    {
                        tvcd->clrText   = _Theme.GetHighlightTextColor();
                        tvcd->clrTextBk = _Theme.GetHighlightColor();
                    }
                    else
                    {
                        tvcd->clrText   = _Theme.GetWindowTextColor();
                        tvcd->clrTextBk = _Theme.GetWindowColor();
                    }

                    return CDRF_NEWFONT; // Tell the control we've changed colors.
                }
            }
        #endif
            break;
        }

        // Handles a right mouse button click within the control.
        case NM_RCLICK:
        {
            const DWORD Position = ::GetMessagePos();

            const POINT pt = { GET_X_LPARAM(Position), GET_Y_LPARAM(Position) };

            // Remember the item we're hovering over, if any.
            _hPopupItem = _TreeView.GetItem(pt);

            const auto Node = (node_t *) _TreeView.GetData(_hPopupItem);

            const bool OnPlaylistNode = (Node != nullptr) && (_PlaylistManager->find_playlist_by_guid(Node->Id) != SIZE_MAX);

            const HMENU hMenu = ::LoadMenuW(THIS_HINSTANCE, MAKEINTRESOURCE(IDM_CONTEXT_MENU));

            if (hMenu == NULL)
                break;

            const HMENU hPopup = ::GetSubMenu(hMenu, 0);

            if (hPopup != NULL)
            {
                {
                    const UINT State = (_hPopupItem != NULL) ? MF_ENABLED : MF_DISABLED | MF_GRAYED;

                    ::EnableMenuItem(hPopup, IDM_RENAME, State);
                    ::EnableMenuItem(hPopup, IDM_REMOVE, State);
                }

                {
                    ::EnableMenuItem(hPopup, IDM_SAVE,     OnPlaylistNode ? MF_ENABLED : MF_DISABLED | MF_GRAYED);

                    ::EnableMenuItem(hPopup, IDM_SAVE_ALL, (_hPopupItem == NULL) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                    ::EnableMenuItem(hPopup, IDM_LOAD,     (_hPopupItem == NULL) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
                }

                // Create and append the Restore submenu.
                {
                    const size_t RecycleCount = _PlaylistManager->recycler_get_count();

                    if (RecycleCount != 0)
                    {
                        HMENU hRestore = ::CreatePopupMenu();

                        for (size_t Index = 0; Index < RecycleCount; ++Index)
                        {
                            pfc::string Name;

                            _PlaylistManager->recycler_get_name(Index, Name);

                            ::AppendMenuW(hRestore, MF_STRING, IDM_HISTORY + Index, msc::UTF8ToWide(Name.c_str()).c_str());
                        }

                        ::AppendMenuW(hRestore, MF_SEPARATOR, 0, NULL);
                        ::AppendMenuW(hRestore, MF_STRING, IDM_CLEAR_HISTORY, L"Clear history");

                        // Append the Restore menu to the popup menu.
                        ::AppendMenuW(hPopup, MF_SEPARATOR, 0, NULL);

                        MENUITEMINFOW mii =
                        {
                            .cbSize     = sizeof(mii),
                            .fMask      = MIIM_STRING | MIIM_SUBMENU,
                            .hSubMenu   = hRestore,
                            .dwTypeData = (LPWSTR) L"Restore",
                        };

                        ::InsertMenuItemW(hPopup, (UINT) ::GetMenuItemCount(hPopup), TRUE, &mii);
                    }
                }

                ::TrackPopupMenu(hPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, nullptr);

                ::PostMessageW(m_hWnd, WM_NULL, 0, 0);
            }

            ::DestroyMenu(hMenu);

            SetMsgHandled(FALSE);

            return FALSE;
        }

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

                    if (_IsPlaying)
                    {
                        const size_t Index = _PlaylistManager->get_playing_playlist();

                        if (Index != SIZE_MAX)
                        {
                            const auto Id = _PlaylistManager->playlist_get_guid(Index);

                            if (Id == Node->Id)
                                Image = ItemImage::PlaylistPlaying;
                        }
                    }

                    tvi.iImage = tvi.iSelectedImage = Image;
                }
            }

            SetMsgHandled(FALSE);

            return FALSE;
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

            SetMsgHandled(FALSE);
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

            SetMsgHandled(FALSE);
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

            SetMsgHandled(FALSE);

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

            // Recalculate the item rectangle.
            _TreeView.RefreshItem(Node->Id);

            // Redraw the tree view. Note: Only required when using custom draw.
            ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
            ::UpdateWindow(_TreeView.Get());

            SetMsgHandled(FALSE);

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

            SetMsgHandled(FALSE);

            return FALSE;
        }

        // Handles the initiation of a drag-and-drop operation involving the left mouse button.
        case TVN_BEGINDRAG:
        {
            const auto nmtv = (NMTREEVIEWW *) nmhd;

            _TreeView.BeginDrag(nmtv);

            SetMsgHandled(FALSE);

            return FALSE;
        }
/*
        // Handles a parent item's list of child itemss is about to expand or collapse.
        case TVN_ITEMEXPANDING:
        {
            const auto nmtv = (NMTREEVIEWW *) nmhd;

            // Redraw the tree view. Note: Only required when using custom draw.
            ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
            ::UpdateWindow(_TreeView.Get());

            return FALSE; // Allow expand.
        }
*/
        // Handles a parent item's list of child items has expanded or collapsed.
        case TVN_ITEMEXPANDED:
        {
            // Redraw the tree view. Note: Only required when using custom draw.
            ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
            ::UpdateWindow(_TreeView.Get());

            SetMsgHandled(FALSE);

            return TRUE;
        }

        // Handles a middle button up (custom) notification.
        case TVN_MBUTTONUP:
        {
            _IsUser = true;

            const auto nmtv = (NMTREEVIEWW *) nmhd;

            _TreeView.RemoveItem(nmtv->itemOld.hItem);

            _IsUser = false;
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

#pragma region playlist_callback

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_added(size_t playlistIndex, size_t start, const pfc::list_base_const_t<metadb_handle_ptr> & handles, const bit_array & selection) noexcept
{
    auto Id = _PlaylistManager->playlist_get_guid(playlistIndex);

    _TreeView.RefreshItem(Id);

    // Redraw the tree view. Note: Only required when using custom draw.
    ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
    ::UpdateWindow(_TreeView.Get());
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

    // Redraw the tree view. Note: Only required when using custom draw.
    ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
    ::UpdateWindow(_TreeView.Get());
}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_selection_change(size_t playlistIndex, const bit_array & affected, const bit_array & state) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_item_focus_change(size_t playlistIndex, size_t from, size_t to) noexcept
{}
    
/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_modified(size_t playlistIndex, const bit_array & mask) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_modified_fromplayback(size_t playlistIndex, const bit_array & mask, play_control::t_display_level level) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_items_replaced(size_t playlistIndex, const bit_array & mask, const pfc::list_base_const_t<t_on_items_replaced_entry> & data) noexcept
{}

/// <summary>
/// 
/// </summary>
void playlist_uielement_t::on_item_ensure_visible(size_t playlistIndex, size_t itemIndex) noexcept
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

    // Redraw the tree view. Note: Only required when using custom draw.
    ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
    ::UpdateWindow(_TreeView.Get());
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
    for (size_t Index = mask.find_first(true, 0, oldCount); Index < oldCount; Index = mask.find_next(true, Index, oldCount))
    {
        auto Id = _PlaylistManager->playlist_get_guid(Index);

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

/// <summary>
/// Deserializes this instance from a JSON object.
/// </summary>
void playlist_uielement_t::FromJSON(json object) noexcept
{
    FromJSON(object, { });

    // Add all playlists that are missing in the loaded configuration to the root.
    const size_t PlaylistCount = _PlaylistManager->get_playlist_count();

    for (size_t PlaylistIndex = 0; PlaylistIndex < PlaylistCount; ++PlaylistIndex)
    {
        const auto Id = _PlaylistManager->playlist_get_guid(PlaylistIndex);

        if (_TreeView.FindItem(Id) != NULL)
            continue;

        pfc::string Name;

        _PlaylistManager->playlist_get_name(PlaylistIndex, Name);

        _TreeView.AddItem({ }, { }, Id, Name.c_str(), false, false);
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

        bool IsFolder   = Node.value("isFolder", IsFolder);
        bool IsExpanded = Node.value("isExpanded", IsExpanded);

        const GUID Id = msc::UTF8ToGUID(IdText);
//      Id = pfc::GUID_from_text(IdText.c_str());

        if (Id == GUID())
            continue; // Should not happen...

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
    auto hItem = _TreeView.GetItem(pt);

    // Highlight the drop target.
    if (hItem != _hDropTarget)
    {
        _TreeView.SetState(_hDropTarget, 0, TVIS_DROPHILITED);

        _hDropTarget = hItem;

        _TreeView.SetState(_hDropTarget, TVIS_DROPHILITED);
    }

    if (keyState & MK_CONTROL)
        return DROPEFFECT_MOVE;

    return DROPEFFECT_COPY;
}

/// <summary>
/// Drops the specified files an the tree view.
/// </summary>
void playlist_uielement_t::DropFiles(IDataObject * dataObject) noexcept
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

    #pragma warning(disable: 4302) // 'type cast': truncation from 'LPWSTR' to 'WORD'
    ::SetCursor(::LoadCursorW(NULL, MAKEINTRESOURCE(IDC_APPSTARTING)));

    static_api_ptr_t<playlist_incoming_item_filter_v2>()->process_dropped_files_async
    (
        dataObject,
        playlist_incoming_item_filter_v2::op_flag_delay_ui,
        core_api::get_main_window(),
        new service_impl_t<drop_notification_handler_t>(Index, true)
    );

    _PlaylistManager->set_active_playlist(Index);

    ::SetCursor(::LoadCursorW(NULL, MAKEINTRESOURCE(IDC_ARROW)));
    #pragma warning(default: 4302)

    // Redraw the tree view. Note: Only required when using custom draw.
    ::InvalidateRect(_TreeView.Get(), nullptr, FALSE);
    ::UpdateWindow(_TreeView.Get());
}

/// <summary>
/// Refreshes this instance.
/// </summary>
void playlist_uielement_t::Refresh() noexcept
{
    _Theme.Initialize();

    HRESULT hResult = InitImageList();

    if (!SUCCEEDED(hResult))
        Log.Write(STR_COMPONENT_BASENAME " failed to initialize image list: 0x%08", hResult);

    _TreeView.RefreshAllItems();
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
            auto Node = (const node_t *) _TreeView.GetData(hItem);

            if ((Node == nullptr) || ((Node != nullptr) && (Node->Id == GUID())))
                return true; // Continue enumerating. Should not occur.

//          (*node)["id"]       = pfc::print_guid(Node->Id).c_str();
            (*node)["id"]       = msc::GUIDToUTF8(Node->Id);
            (*node)["name"]     = Node->Name;
            (*node)["isFolder"] = Node->IsFolder;

            if (Node->IsFolder)
                (*node)["isExpanded"] = _TreeView.IsExpanded(Node->Id);

            return true; // Continue enumerating.
        }, &Nodes);

        Object["nodes"] = Nodes;

        #ifdef _DEBUG
        console::print(Object.dump(4).c_str());
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
    _hImageList.Reset();

    const auto IconSize = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

    _hImageList = ::ImageList_Create((int) IconSize, (int) IconSize, ILC_COLOR32 | ILC_MASK, (int) _State._Images.size(), 0);

    if (!_hImageList)
        return HRESULT_FROM_WIN32(::GetLastError());

    for (const auto & Image : _State._Images)
    {
        himagelist_t hSrcImageList = image_list_factory_t::Create(Image._FilePath, IconSize);

        if (!hSrcImageList)
            return HRESULT_FROM_WIN32(::GetLastError());

        hicon_t hIcon = ::ImageList_GetIcon(hSrcImageList, (int) Image._IconIndex, ILD_TRANSPARENT);

        if (!hIcon)
            return HRESULT_FROM_WIN32(::GetLastError());

        ::ImageList_ReplaceIcon(_hImageList, -1, hIcon);
    }

    _TreeView.SetNormalImageList(_hImageList);
    _TreeView.SetStateImageList(_hImageList);

    return S_OK;
}

tracker_t<playlist_uielement_t> _UIElementTracker;
