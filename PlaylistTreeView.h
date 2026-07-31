
/** $VER: PlaylistsTreeView.h (2026.07.30) P. Stuer **/

#pragma once

#include "TreeView.h"
#include "Node.h"
#include "Log.h"

#pragma warning(push)
#pragma warning(disable: 4868) // compiler may not enforce left-to-right evaluation order in braced initializer list

#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

#pragma warning(pop)

/// <summary>
/// Implements a tree view that understands nodes.
/// </summary>
class playlist_tree_view_t : public tree_view_t
{
public:
    playlist_tree_view_t() : tree_view_t() { };

    playlist_tree_view_t(const playlist_tree_view_t &) = delete;
    playlist_tree_view_t(const playlist_tree_view_t &&) = delete;
    playlist_tree_view_t & operator=(const playlist_tree_view_t &) = delete;
    playlist_tree_view_t & operator=(playlist_tree_view_t &&) = delete;

    virtual ~playlist_tree_view_t() noexcept { };

    bool GetText(const GUID & id, std::string & text) const noexcept;

    using tree_view_t::GetText;     // Adds the base class overload.

    bool SetName(const GUID & id, const std::string & name) const noexcept;

    node_t * AddItem(const GUID & parentId, const GUID & insertAfterId, const GUID & id, const std::string & name, bool isFolder, bool isExpanded) const noexcept;
    bool RemoveItem(const GUID & id) const noexcept;
    bool SelectItem(const GUID & id) const noexcept;
    bool SelectItem(const std::string & name) const noexcept;

    using tree_view_t::SelectItem;  // Adds the base class overload.

    HTREEITEM FindItem(const GUID & id) const noexcept;

    HTREEITEM GetHighlightedItem(const POINT & pt) const noexcept;

    node_t * GetSelectedItem() const noexcept;

    bool RefreshItem(const GUID & id) const noexcept;

    bool IsExpanded(const GUID & id) const noexcept;

    bool RemoveItem(HTREEITEM hItem) const noexcept
    {
        return __super::RemoveItem(hItem);
    }

    size_t GetChildCount(const GUID & id) const noexcept;

    /// <summary>
    /// Serializes this instance to JSON.
    /// </summary>
    template<typename Visitor> bool ToJSON(Visitor && visitor, json::array_t * nodes) const noexcept
    {
        return ToJSON_(TreeView_GetRoot(Get()), visitor, nodes);
    }

    /// <summary>
    /// Walks the tree view.
    /// </summary>
    template<typename Visitor> bool Walk(Visitor && visitor) const noexcept
    {
        const auto hItem = TreeView_GetRoot(Get());

        return Walk_(hItem, NULL, visitor); // Always walk the complete tree.
    }

    /// <summary>
    /// Walks the tree view starting at the specified node.
    /// </summary>
    template<typename Visitor> bool Walk(Visitor && visitor, const node_t * startNode) const noexcept
    {
        if (startNode == nullptr)
            return false;

        auto hItem = FindItem(startNode->Id);

        if (hItem == NULL)
            return false;

        return Walk_(hItem, hItem, visitor);
    }

protected:
    virtual bool AllowDrop(DropZone dropZone) noexcept override;

private:
    /// <summary>
    /// Serializes this instance to JSON.
    /// </summary>
    template<typename Visitor> bool ToJSON_(HTREEITEM hItem, Visitor && visitor, json::array_t * nodes) const noexcept
    {
        while (hItem != NULL)
        {
            json::object_t Node;

            if (!visitor(hItem, &Node))
                return false;

            json::array_t Nodes;

            // Walk the sub branch if the current item has children.
            const auto hChild = TreeView_GetChild(Get(), hItem);

            if ((hChild != NULL) && !ToJSON_(hChild, visitor, &Nodes))
                return false;

            if (Nodes.size() != 0)
                Node["nodes"] = Nodes;

            (*nodes).push_back(Node);

            hItem = TreeView_GetNextSibling(Get(), hItem);
        }

        return true;
    }

    /// <summary>
    /// Walks this instance.
    /// </summary>
    template<typename Visitor> bool Walk_(HTREEITEM hItem, HTREEITEM hStartItem, Visitor && visitor) const noexcept
    {
        auto hCurrentItem = hItem;

        while (hCurrentItem != NULL)
        {
            const auto Node = (node_t *) GetData(hCurrentItem);

            if (!visitor(Node))
                return false;

            // Walk the sub branch if the current item has children.
            const auto hChild = TreeView_GetChild(Get(), hCurrentItem);

            if ((hChild != NULL) && !Walk_(hChild, hStartItem, visitor))
                return false;

            hCurrentItem = TreeView_GetNextSibling(Get(), hCurrentItem);
        }

        // Stop walking if we've walked the complete branch downwards. Don't go up.
        if (TreeView_GetParent(Get(), hItem) == hStartItem)
            return false;

        return true;
    }
};
