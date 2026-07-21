
/** $VER: MultiSelectTreeView.h (2026.07.19) P. Stuer - Adaped from foobar2000 SDK **/

#pragma once

#include "Toggle.h"
#include "Log.h"

#include <set>
#include <vector>

class multi_select_tree_view_t : public CMessageMap
{
public:
    multi_select_tree_view_t(unsigned ID) : _Id(ID) { }

    multi_select_tree_view_t(const multi_select_tree_view_t &) = delete;
    multi_select_tree_view_t(const multi_select_tree_view_t &&) = delete;
    multi_select_tree_view_t & operator=(const multi_select_tree_view_t &) = delete;
    multi_select_tree_view_t & operator=(multi_select_tree_view_t &&) = delete;

    virtual ~multi_select_tree_view_t() noexcept { };

    /// <summary>
    /// 
    /// </summary>
    std::set<HTREEITEM> GetSelection() const noexcept
    {
        return _Selection;
    }

    /// <summary>
    /// 
    /// </summary>
    std::set<HTREEITEM> const & GetSelectionRef() const noexcept
    {
        return _Selection;
    }

    /// <summary>
    /// 
    /// </summary>
    HTREEITEM GetSingleSelection() const
    {
        if (_Selection.size() != 1)
            return NULL;

        return *_Selection.begin();
    }

    /// <summary>
    /// 
    /// </summary>
    std::vector<HTREEITEM> GetOrderedSelection(CTreeViewCtrl tree) const
    {
        std::vector<HTREEITEM> Result;

        Result.reserve(_Selection.size());

        for (HTREEITEM Iter = tree.GetRootItem(); Iter != NULL; Iter = tree.GetNextVisibleItem(Iter))
        {
            if (_Selection.count(Iter) > 0)
                Result.push_back(Iter);
        }

        return Result;
    }

    /// <summary>
    /// 
    /// </summary>
    size_t GetSelectionCount() const noexcept
    {
        return _Selection.size();
    }

    /// <summary>
    /// 
    /// </summary>
    bool IsItemSelected(HTREEITEM hItem) const noexcept
    {
        return _Selection.count(hItem) > 0;
    }

    /// <summary>
    /// 
    /// </summary>
    void ClearSelection(CTreeViewCtrl tree)
    {
        if (_Selection.size() == 0)
            return;

        CRgn UpdateRegion;

        if (_Selection.size() <= 100)
        {
            UpdateRegion.CreateRectRgn(0, 0, 0, 0);

            for (auto hIter : _Selection)
            {
                CRect rc;

                if (!tree.GetItemRect(hIter, rc, TRUE))
                    continue;

                CRgn Region;

                Region.CreateRectRgnIndirect(rc);

                UpdateRegion.CombineRgn(Region, RGN_OR);
            }
        }

        _Selection.clear();

        tree.RedrawWindow(NULL, UpdateRegion);
    }

    /// <summary>
    /// 
    /// </summary>
    void SelectSingleItem(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        _SelectionStart = hItem;

        if ((_Selection.size() == 1) && (*_Selection.begin() == hItem))
            return;

        ClearSelection(tree);

        SelectItem(tree, hItem);
    }

    /// <summary>
    /// 
    /// </summary>
    void SelectItem(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        if (IsItemSelected(hItem))
            return;

        _Selection.insert(hItem);

        RedrawItem(tree, hItem);
    }

    /// <summary>
    /// 
    /// </summary>
    void DeselectItem(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        if (!IsItemSelected(hItem))
            return;

        _Selection.erase(hItem);

        RedrawItem(tree, hItem);
    }

    BEGIN_MSG_MAP_EX(multi_select_tree_view_t)
        NOTIFY_HANDLER_EX(_Id, NM_CLICK, OnClick)
        NOTIFY_HANDLER_EX(_Id, NM_SETFOCUS, OnFocus)
        NOTIFY_HANDLER_EX(_Id, NM_KILLFOCUS, OnFocus)
        NOTIFY_HANDLER_EX(_Id, NM_CUSTOMDRAW, OnCustomDraw)

        NOTIFY_HANDLER_EX(_Id, TVN_SELCHANGING, OnSelectionChanging)
        NOTIFY_HANDLER_EX(_Id, TVN_SELCHANGED, OnSelectionChanged)
        NOTIFY_HANDLER_EX(_Id, TVN_ITEMEXPANDED, OnItemExpanded)
        NOTIFY_HANDLER_EX(_Id, TVN_DELETEITEM, OnItemDeleted)
    END_MSG_MAP()

private:
    /// <summary>
    /// 
    /// </summary>
    LRESULT OnClick(NMHDR * nmhd) noexcept
    {
        SetMsgHandled(FALSE);

        CTreeViewCtrl Tree(nmhd->hwndFrom);

        CPoint pt((LPARAM) ::GetMessagePos());

        Tree.ScreenToClient(&pt);

        UINT Flags = 0;

        HTREEITEM hItem = Tree.HitTest(pt, &Flags);

        if (!((hItem != NULL) && ((Flags & TVHT_ONITEM) != 0)))
            return FALSE;
/*
        if (IsKeyPressed(VK_CONTROL))
        {
            ToggleSelection(Tree, hItem);
            UpdateSelection(Tree, hItem);

            return TRUE;
        }
*/
        if ((hItem == Tree.GetSelectedItem()) && !IsItemSelected(hItem))
        {
            ToggleSelection(Tree, hItem);

            return TRUE;
        }

        return FALSE;
    }

    /// <summary>
    /// 
    /// </summary>
    LRESULT OnFocus(NMHDR * nmhd) noexcept
    {
        SetMsgHandled(FALSE);

        CTreeViewCtrl Tree(nmhd->hwndFrom);

        if (_Selection.size() > 100)
        {
            Tree.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
        else
        if (_Selection.size() > 0)
        {
            CRgn UpdateRegion;

            UpdateRegion.CreateRectRgn(0, 0, 0, 0);

            for (auto hIter : _Selection)
            {
                CRect rc;

                if (!Tree.GetItemRect(hIter, rc, TRUE))
                    continue;

                CRgn Region;

                Region.CreateRectRgnIndirect(rc);

                UpdateRegion.CombineRgn(Region, RGN_OR);
            }

            Tree.RedrawWindow(NULL, UpdateRegion, RDW_INVALIDATE | RDW_ERASE);
        }

        return 0;
    }

    /// <summary>
    /// 
    /// </summary>
    LRESULT OnCustomDraw(NMHDR * nmhd) noexcept
    {
        SetMsgHandled(FALSE);

        auto tvcd = (NMTVCUSTOMDRAW *) nmhd;

        switch (tvcd->nmcd.dwDrawStage)
        {
            case CDDS_ITEMPREPAINT:
            {
                const auto hItem = (HTREEITEM) tvcd->nmcd.dwItemSpec;

                UINT State = 0;

                if (IsItemSelected(hItem))
                {
                    tvcd->nmcd.uItemState |= CDIS_SELECTED;

                    State = TVIS_SELECTED;
                }
                else
                    tvcd->nmcd.uItemState &= ~(CDIS_FOCUS | CDIS_SELECTED);

                CTreeViewCtrl Tree(nmhd->hwndFrom);

                Tree.SetItemState(hItem, State, TVIS_SELECTED);

                return CDRF_DODEFAULT;
            }

            case CDDS_PREPAINT:
                return CDRF_NOTIFYITEMDRAW;

            default:
                return CDRF_DODEFAULT;
        }
    }

    /// <summary>
    /// 
    /// </summary>
    LRESULT OnSelectionChanging(NMHDR * nmhd) noexcept
    {
        SetMsgHandled(FALSE);

        if (_OwnSelectionChange)
            return 0;

        CTreeViewCtrl Tree(nmhd->hwndFrom);

        auto nmtv = (NMTREEVIEW *) nmhd;

        const auto hItem = nmtv->itemNew.hItem;

        if (IsTypingInProgress())
            SelectSingleItem(Tree, hItem);
        else
        if (IsKeyPressed(VK_SHIFT))
            SelectRange(Tree, hItem);
        else
        if (IsKeyPressed(VK_CONTROL))
            ToggleSelection(Tree, hItem);
        else
            SelectSingleItem(Tree, hItem);

        return 0;
    }

    /// <summary>
    /// 
    /// </summary>
    LRESULT OnSelectionChanged(NMHDR *) noexcept
    {
        SetMsgHandled(FALSE);

        if (_OwnSelectionChangeNotification)
            SetMsgHandled(FALSE);

        return 0;
    }

    /// <summary>
    /// 
    /// </summary>
    LRESULT OnItemExpanded(NMHDR * nmhd) noexcept
    {
        SetMsgHandled(FALSE);

        auto nmtv = (NMTREEVIEW *) nmhd;

        CTreeViewCtrl Tree(nmhd->hwndFrom);

        if ((nmtv->itemNew.state & TVIS_EXPANDED) == 0)
        {
            if (DeselectChildren(Tree, nmtv->itemNew.hItem))
                SendSelectionChangedNotification(Tree, nmtv->itemNew.hItem);
        }

        SetMsgHandled(FALSE);

        return 0;
    }

    /// <summary>
    /// 
    /// </summary>
    LRESULT OnItemDeleted(NMHDR * nmhd) noexcept
    {
        SetMsgHandled(FALSE);

        const auto hItem = ((NMTREEVIEW *) nmhd)->itemOld.hItem;

        _Selection.erase(hItem);

        if (_SelectionStart == hItem)
            _SelectionStart = NULL;

        return 0;
    }

private:
    /// <summary>
    /// 
    /// </summary>
    bool DeselectChildren(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        bool State = false;

        for (auto hIter = tree.GetChildItem(hItem); hIter != NULL; hIter = tree.GetNextSiblingItem(hIter))
        {
            if (_Selection.erase(hIter) > 0)
                State = true;

            if (_SelectionStart == hIter)
                _SelectionStart = NULL;

            if (tree.GetItemState(hIter, TVIS_EXPANDED))
            {
                if (DeselectChildren(tree, hIter))
                    State = true;
            }
        }

        return State;
    }

    /// <summary>
    /// 
    /// </summary>
    void RedrawItem(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        CRect rc;

        if (tree.GetItemRect(hItem, rc, TRUE))
            tree.RedrawWindow(rc);

        SendSelectionChangedNotification(tree, hItem);
    }

    /// <summary>
    /// Sends an TVN_SELCHANGED notification.
    /// </summary>
    void SendSelectionChangedNotification(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        NMTREEVIEWW nmtv
        {
            .hdr
            {
                .hwndFrom = tree,
                .idFrom   = _Id,
                .code     = TVN_SELCHANGED
            },
            .action = TVN_SELCHANGED,
            .itemOld =
            {
                .mask = 0,
                .hItem = hItem,
/*
                UINT      state;
                UINT      stateMask;
                LPWSTR    pszText;
                int       cchTextMax;
                int       iImage;
                int       iSelectedImage;
                int       cChildren;
                LPARAM    lParam;
*/
            },
//          .itemNew,
//          .ptDrag
        };

        auto Scope = toggle_t(_OwnSelectionChangeNotification, true);
        {
            tree.GetParent().SendMessage(WM_NOTIFY, _Id, (LPARAM) &nmtv);
        }
    }

    /// <summary>
    /// 
    /// </summary>
    void UpdateSelection(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        if (IsItemSelected(hItem) || (hItem != tree.GetSelectedItem()))
            return;

        auto Scope = toggle_t(_OwnSelectionChange, true);

        for (;;)
        {
            if ((hItem == TVI_ROOT) || (hItem == NULL) || IsItemSelected(hItem))
            {
                tree.SelectItem(hItem);

                return;
            }

            for (auto hIter = tree.GetPrevSiblingItem(hItem); hIter != NULL; hIter = tree.GetPrevSiblingItem(hIter))
            {
                if (IsItemSelected(hIter))
                {
                    tree.SelectItem(hIter);

                    return;
                }
            }

            for (auto hIter = tree.GetNextSiblingItem(hItem); hIter != NULL; hIter = tree.GetNextSiblingItem(hIter))
            {
                if (IsItemSelected(hIter))
                {
                    tree.SelectItem(hIter);

                    return;
                }
            }

            hItem = tree.GetParentItem(hItem);
        }
    }

    /// <summary>
    /// 
    /// </summary>
    void ToggleSelection(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        _SelectionStart = hItem;

        if (IsItemSelected(hItem))
            _Selection.erase(hItem);
        else
            _Selection.insert(hItem);

        RedrawItem(tree, hItem);
    }

    /// <summary>
    /// 
    /// </summary>
    void SelectRange(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        if ((_SelectionStart == NULL) || (_SelectionStart == hItem))
        {
            SelectSingleItem(tree, hItem);

            return;
        }

        auto NewSelection = GrabRange(tree, _SelectionStart, hItem);

        ApplySelection(tree, std::move(NewSelection));
    }

    /// <summary>
    /// 
    /// </summary>
    static std::set<HTREEITEM> GrabRange(CTreeViewCtrl tree, HTREEITEM hFromItem, HTREEITEM hToItem)
    {
        std::set<HTREEITEM> Range1, Range2;

        HTREEITEM hFromIter = hFromItem, hToIter = hToItem;

        for (;;)
        {
            if (hFromIter != NULL)
            {
                Range1.insert(hFromIter);

                if (hFromIter == hToItem)
                    return Range1;

                hFromIter = tree.GetNextVisibleItem(hFromIter);
            }

            if (hToIter != NULL)
            {
                Range2.insert(hToIter);

                if (hToIter == hFromItem)
                    return Range2;

                hToIter = tree.GetNextVisibleItem(hToIter);
            }

            if ((hFromIter == NULL) && (hToIter == NULL))
                return std::set<HTREEITEM>();
        }
    }

    /// <summary>
    /// 
    /// </summary>
    void ApplySelection(CTreeViewCtrl tree, std::set<HTREEITEM> && newSelection)
    {
        CRgn UpdateRegion;

        bool IsChanged = false;

        if ((newSelection.size() != _Selection.size()) && ((newSelection.size() + _Selection.size()) > 100))
        {
            IsChanged = true;
        }
        else
        {
            UpdateRegion.CreateRectRgn(0, 0, 0, 0);

            // Add the regions of the newly selected items.
            for (auto hIter : _Selection)
            {
                if (newSelection.count(hIter) == 0)
                {
                    IsChanged = true;

                    CRect rc;

                    if (tree.GetItemRect(hIter, rc, TRUE))
                        continue;

                    CRgn Region;

                    Region.CreateRectRgnIndirect(rc);

                    UpdateRegion.CombineRgn(Region, RGN_OR);
                }
            }

            // Add the regions of the existing selected items.
            for (auto hIter : newSelection)
            {
                if (_Selection.count(hIter) == 0)
                {
                    IsChanged = true;

                    CRect rc;

                    if (tree.GetItemRect(hIter, rc, TRUE))
                        continue;

                    CRgn Region;

                    Region.CreateRectRgnIndirect(rc);

                    UpdateRegion.CombineRgn(Region, RGN_OR);
                }
            }
        }

        if (IsChanged)
        {
            _Selection = std::move(newSelection);

            tree.RedrawWindow(NULL, UpdateRegion);

            SendSelectionChangedNotification(tree, NULL);
        }
    }

    /// <summary>
    /// Returns true if the user is typing.
    /// </summary>
    bool IsTypingInProgress() const
    {
        return _IsLastTypingTimeValid && (::GetTickCount64() - _LastTypingTime < 500);
    }

    bool _IsLastTypingTimeValid = false;
    ULONGLONG _LastTypingTime = 0;

    /// <summary>
    /// Returns true if the specified key is a navigation key.
    /// </summary>
    static bool IsNavigationKey(UINT vKey) noexcept
    {
        switch (vKey)
        {
            case VK_UP:
            case VK_DOWN:
            case VK_RIGHT:
            case VK_LEFT:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_HOME:
            case VK_END:
                return true;

            default:
                return false;
        }
    }

#ifdef Unused
private:
    void OnContextMenu_FixSelection(CTreeViewCtrl tree, CPoint pt)
    {
        if (pt == CPoint(-1, -1))
            return;

        tree.ScreenToClient(&pt);

        UINT Flags = 0;

        const HTREEITEM hItem = tree.HitTest(pt, &Flags);

        if (hItem != NULL && (Flags & TVHT_ONITEM) != 0)
        {
            if (!IsItemSelected(hItem))
                SelectSingleItem(tree, hItem);

            SelectItemInternal(tree, hItem);
        }
    }

    void OnLButtonDown(CTreeViewCtrl tree, WPARAM wParam, LPARAM lParam)
    {
        if (IsKeyPressed(VK_CONTROL))
            return;

        UINT Flags = 0;

        HTREEITEM hItem = tree.HitTest(CPoint(lParam), &Flags);

        if ((hItem != NULL) && ((Flags & TVHT_ONITEM) != 0) && !IsItemSelected(hItem))
            tree.SelectItem(hItem);
    }

    BOOL OnChar(CTreeViewCtrl tree, WPARAM code)
    {
        switch (code)
        {
            case ' ':
            {
                if (IsKeyPressed(VK_CONTROL) || !IsTypingInProgress())
                {
                    HTREEITEM item = tree.GetSelectedItem();

                    if (item != NULL)
                        ToggleSelection(tree, item);

                    return TRUE;
                }
                break;
            }
        }

        _LastTypingTime = ::GetTickCount64();
        _IsLastTypingTimeValid = true;

        return FALSE;
    }

    BOOL OnKeyDown(CTreeViewCtrl tree, UINT vKey)
    {
        if (IsNavigationKey(vKey))
            _IsLastTypingTimeValid = false;

        switch (vKey)
        {
            case VK_UP:
            {
                if (IsKeyPressed(VK_CONTROL))
                {
                    auto hItem = tree.GetSelectedItem();

                    if (hItem != NULL)
                    {
                        HTREEITEM prev = tree.GetPrevVisibleItem(hItem);

                        if (prev != NULL)
                        {
                            SelectItemInternal(tree, prev);

                            if (IsKeyPressed(VK_SHIFT))
                            {
                                if (_SelectionStart == NULL)
                                    _SelectionStart = hItem;

                                SelectRange(tree, prev);
                            }
                        }
                    }

                    return TRUE;
                }
                break;
            }

            case VK_DOWN:
            {
                if (IsKeyPressed(VK_CONTROL))
                {
                    auto hItem = tree.GetSelectedItem();

                    if (hItem != NULL)
                    {
                        HTREEITEM next = tree.GetNextVisibleItem(hItem);

                        if (next != NULL)
                        {
                            SelectItemInternal(tree, next);

                            if (IsKeyPressed(VK_SHIFT))
                            {
                                if (_SelectionStart == NULL)
                                    _SelectionStart = hItem;

                                SelectRange(tree, next);
                            }
                        }
                    }
                    return TRUE;
                }
                break;
            }
/*
            case VK_LEFT:
            {
                if (IsKeyPressed(VK_CONTROL))
                {
                    tree.SendMessage(WM_HSCROLL, SB_LINEUP, 0);
                }
                break;
            }

            case VK_RIGHT:
            {
                if (IsKeyPressed(VK_CONTROL))
                {
                    tree.SendMessage(WM_HSCROLL, SB_LINEDOWN, 0);
                }
                break;
            }
*/
        }

        return FALSE;
    }

    void SelectItemInternal(CTreeViewCtrl tree, HTREEITEM hItem)
    {
        auto Scope = toggle_t(_OwnSelectionChange, true);

        tree.SelectItem(hItem);
    }
#endif

private:
    const UINT _Id;

    std::set<HTREEITEM> _Selection;
    HTREEITEM _SelectionStart = NULL;

    bool _OwnSelectionChangeNotification = false;
    bool _OwnSelectionChange = false;
};
