
/** $VER: PlaylistsTreeView.cpp (2026.08.04) P. Stuer **/

#include "pch.h"

#include "PlaylistTreeView.h"

#include "Theme.h"

#pragma hdrstop

/// <summary>
/// Gets the text of the specified item.
/// </summary>
bool playlist_tree_view_t::GetText(const GUID & id, std::string & text) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    return __super::GetText(hItem, text);
}

/// <summary>
/// Sets the name of the specified item.
/// </summary>
bool playlist_tree_view_t::SetName(const GUID & id, const std::string & name) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    auto Node = (node_t *) GetData(hItem);

    if (Node == nullptr)
        return false;

    Node->Name = name;

    RedrawItem(hItem);

    return true;
}

/// <summary>
/// Adds an item.
/// </summary>
node_t * playlist_tree_view_t::AddItem(const GUID & parentId, const GUID & insertAfterId, const GUID & id, const std::string & name, bool isFolder, bool isExpanded) const noexcept
{
    auto Node = new node_t(id, name, isFolder);

    HTREEITEM hParent = FindItem(parentId);

    if (hParent == NULL)
        hParent = TVI_ROOT;

    HTREEITEM hInsertAfter = FindItem(insertAfterId);

    if (hInsertAfter == NULL)
        hInsertAfter = TVI_LAST;

    const UINT State = isExpanded ? TVIS_EXPANDED : 0;

    auto hNewItem = __super::AddItem(hParent, hInsertAfter, State, Node);

    if (hNewItem == NULL)
    {
        delete Node;

        return nullptr;
    }

    return Node;
}

/// <summary>
/// Removes the specified item.
/// </summary>
bool playlist_tree_view_t::RemoveItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    return __super::RemoveItem(hItem);
}

/// <summary>
/// Selects the specified id.
/// </summary>
bool playlist_tree_view_t::SelectItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    return SelectItem(hItem);
}

/// <summary>
/// Selects the item with the specified name.
/// </summary>
bool playlist_tree_view_t::SelectItem(const std::string & name) const noexcept
{
    bool IsSelected = false;

    __super::Walk([&](HTREEITEM hItem, void * context) -> bool
    {
        auto Node = (const node_t *) GetData(hItem);

        if ((Node != nullptr) && (Node->Name == name))
        {
            IsSelected = SelectItem(hItem);

            return false;
        }

        return true; // Continue walking.
    });

    return IsSelected;
}

/// <summary>
/// Finds the item with the specified id.
/// </summary>
HTREEITEM playlist_tree_view_t::FindItem(const GUID & id) const noexcept
{
    if (id == GUID_NULL)
        return NULL;

    HTREEITEM hFoundItem = NULL;

    __super::Walk([&](HTREEITEM hItem, void * context) -> bool
    {
        auto Node = (const node_t *) GetData(hItem);

        if ((Node != nullptr) && (Node->Id == id))
        {
            hFoundItem = hItem;

            return false;
        }

        return true; // Continue walking
    });

    return hFoundItem;
}

/// <summary>
/// Gets the item at the specified point.
/// </summary>
HTREEITEM playlist_tree_view_t::GetHighlightedItem(const POINT & pt) const noexcept
{
    auto hItem = __super::GetHighlightedItem(pt);

    return hItem;
}

/// <summary>
/// Gets the selected item.
/// </summary>
node_t * playlist_tree_view_t::GetSelectedItem() const noexcept
{
    auto hItem = __super::GetSelectedItem();

    if (hItem == NULL)
        return nullptr;

    auto Node = (node_t *) GetData(hItem);

    return Node;
}

/// <summary>
/// Refreshes the specified item.
/// </summary>
bool playlist_tree_view_t::RefreshItem(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    __super::RefreshItem(hItem);

    return true;
}

/// <summary>
/// Returns true if the specified item is expanded.
/// </summary>
bool playlist_tree_view_t::IsExpanded(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return false;

    UINT State = 0;

    if (!GetState(hItem, State))
        return false;

    return ((State & TVIS_EXPANDED) == TVIS_EXPANDED);
}

/// <summary>
/// Returns the number of direct children of a node.
/// </summary>
size_t playlist_tree_view_t::GetChildCount(const GUID & id) const noexcept
{
    HTREEITEM hItem = FindItem(id);

    if (hItem == NULL)
        return SIZE_MAX;

    return __super::GetChildCount(hItem);
}

/// <summary>
/// Returns true if a drop is allowed on the target.
/// </summary>
bool playlist_tree_view_t::AllowDrop(DropZone dropZone) noexcept
{
    if (_hDropTarget == NULL)
        return false;

    auto Node = (const node_t *) GetData(_hDropTarget);

    if (Node == nullptr)
        return false;

    return Node->IsFolder || (!Node->IsFolder && (dropZone != DropZone::Middle));
}

/// <summary>
/// Draws a treeview item.
/// </summary>
void playlist_tree_view_t::DrawItem(HDC hDC, HTREEITEM hItem, int level, int scrollX, const RECT & rcItem, HIMAGELIST imageList, uint32_t imageWidth, bool hasFolders, bool isHot, bool isFocused, bool isDragImage) const noexcept
{
    const auto HasFocus = (::GetFocus() == Get());

    // Get information about the item.
    wchar_t Text[512] = { };

    TVITEMEX tvi
    {
        .mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE | TVIF_CHILDREN,
        .hItem      = hItem,
        .stateMask  = 0xFF,
        .pszText    = Text,
        .cchTextMax = _countof(Text),
    };

    TreeView_GetItem(Get(), &tvi);

    const auto IsSelected    = ((tvi.state & TVIS_SELECTED)    != 0) & !isDragImage;
    const auto IsHighlighted = ((tvi.state & TVIS_DROPHILITED) != 0) & !isDragImage;

    // Calculate the start position of the item content.
    RECT rc = rcItem;

    rc.left += (imageWidth * level) - scrollX;

    // Calculate the image width including horizontal padding.
    const LONG ImageWidth = (1 + (LONG) imageWidth + 1) + 3;

    // Get bounding rectangle of the item text.
    RECT rcText;

    TreeView_GetItemRect(Get(), hItem, &rcText, TRUE);

    const LONG TextWidth  = rcText.right  - rcText.left;
    const LONG TextHeight = rcText.bottom - rcText.top;

    // Draw a chevron if folders are being used.
    if (hasFolders)
    {
        // Draw a chevron if the folder has children.
        if (tvi.cChildren != 0)
        {
            RECT rcChevron = { rc.left, rc.top, rcChevron.left + TextHeight, rc.bottom };

            const auto hOldFont = ::SelectObject(hDC, _Theme.GetIconFont());

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

            ::DrawThemeTextEx(_Theme.GetTextStyle(), hDC, 0, 0, (tvi.state & TVIS_EXPANDED) ? ChevronDown : ChevronRight, -1, Flags, &rcChevron, &Options);

            ::SelectObject(hDC, hOldFont);
        }

        rc.left += TextHeight;
    }

    // Draw the background.
    if (!isDragImage)
    {
        rc.right = rc.left + ImageWidth + TextWidth;

        if (IsSelected)
        {
            const auto & hBrush = HasFocus ? _Theme.GetSelectionBrush() : _Theme.GetInactiveSelectionBrush();

            ::FillRect(hDC, &rc, hBrush);

            // Draw the focus rectangle.
            if (isFocused)
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
        if (isHot || IsHighlighted)
        {
            const auto & hBrush = _Theme.GetHighlightBrush();

            ::FillRect(hDC, &rc, hBrush);
        }
    }

    // Draw the image.
    {
        const LONG dx = 1;
        const LONG dy = (TextHeight - (LONG) imageWidth) / 2;

        ::ImageList_Draw(imageList, tvi.iImage, hDC, rc.left + dx, rc.top + dy, ILD_TRANSPARENT);

        rc.left += ImageWidth;
    }

    // Draw the text.
    {
        const COLORREF Color = IsSelected ? (HasFocus ? _Theme.GetSelectionTextColor() : _Theme.GetInactiveSelectionTextColor()) : ((isHot || IsHighlighted) ? _Theme.GetHighlightTextColor() : _Theme.GetWindowTextColor());

        const DTTOPTS Options =
        {
            .dwSize = sizeof(Options),
            .dwFlags = DTT_TEXTCOLOR,
            .crText = Color
        };

        rc.right = rc.left + TextWidth;

        const auto hOldFont = ::SelectObject(hDC, _Theme.GetPlaylistFont());

        constexpr DWORD Flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX;

        ::DrawThemeTextEx(_Theme.GetTextStyle(), hDC, 0, 0, Text, -1, Flags, &rc, &Options);

        ::SelectObject(hDC, hOldFont);
    }
}

/// <summary>
/// Calculates the dimensions of a drag image.
/// </summary>
void playlist_tree_view_t::MeasureDragImage(HTREEITEM hItem, RECT & rect) const noexcept
{
    // Get the rectangle of the text part of the item.
    TreeView_GetItemRect(Get(), hItem, &rect, TRUE);

    // Add room for the icon.
    HIMAGELIST hImageList = GetNormalImageList();

    int IconWidth, IconHeight;

    ::ImageList_GetIconSize(hImageList, &IconWidth, &IconHeight);

    rect.left -= 1 + IconWidth + 1 + 3;
}

/// <summary>
/// Draws a treeview item.
/// </summary>
void playlist_tree_view_t::DrawDragImage(HDC hDC, HTREEITEM hItem, const RECT & rc) const noexcept
{
    RECT rcItem = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };

    HIMAGELIST hImageList = GetNormalImageList();

    int IconWidth, IconHeight;

    ::ImageList_GetIconSize(hImageList, &IconWidth, &IconHeight);

    ::FillRect(hDC, &rcItem, _Theme.GetWindowBrush());

    DrawItem(hDC, hItem, 0, 0, rcItem, hImageList, (uint32_t) IconWidth, false, false, false, true);
}

