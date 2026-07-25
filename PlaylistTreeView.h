
/** $VER: PlaylistsTreeView.h (2026.07.24) P. Stuer **/

#pragma once

#include "TreeView.h"
#include "Node.h"

#pragma warning(push)
#pragma warning(disable: 4868) // compiler may not enforce left-to-right evaluation order in braced initializer list

#include <nlohmann\json.hpp>

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
    bool SetName(const GUID & id, const std::string & name) const noexcept;

    using tree_view_t::SelectItem; // Adds the base class overload.

    node_t * AddItem(const GUID & parentId, const GUID & insertAfterId, const GUID & id, const std::string & name, bool isFolder, bool isExpanded) const noexcept;
    bool RemoveItem(const GUID & id) const noexcept;
    bool SelectItem(const GUID & id) const noexcept;

    HTREEITEM FindItem(const GUID & id) const noexcept;

    HTREEITEM GetHighlightedItem(const POINT & pt) const noexcept;

    node_t * GetSelectedItem() const noexcept;

    bool RefreshItem(const GUID & id) const noexcept;

    bool IsExpanded(const GUID & id) const noexcept;

    bool RemoveItem(HTREEITEM hItem) const noexcept
    {
        return __super::RemoveItem(hItem);
    }

    /// <summary>
    /// Serializes this instance to JSON.
    /// </summary>
    template<typename Visitor> bool ToJSON(Visitor && visitor, json::array_t * nodes) const noexcept
    {
        return ToJSON_(TreeView_GetRoot(Get()), visitor, nodes);
    }

protected:
    /// <summary>
    /// Returns true if a drop is allowed on the target.
    /// </summary>
    virtual bool AllowDrop(DropZone dropZone) noexcept override
    {
        if (_hDropTarget == NULL)
            return false;

        auto Node = (const node_t *) GetData(_hDropTarget);

        if (Node == nullptr)
            return false;

        return Node->IsFolder || (!Node->IsFolder && (dropZone != DropZone::Middle));
    }

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

            if (!ToJSON_(TreeView_GetChild(Get(), hItem), visitor, &Nodes))
                return false;

            if (Nodes.size() != 0)
                Node["nodes"] = Nodes;

            (*nodes).push_back(Node);

            hItem = TreeView_GetNextSibling(Get(), hItem);
        }

        return true;
    }

public:
};
