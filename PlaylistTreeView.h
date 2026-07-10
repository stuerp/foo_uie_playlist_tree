
/** $VER: PlaylistsTreeView.h (2026.07.08) P. Stuer **/

#pragma once

#include "TreeView.h"

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

    std::string GetText(const GUID id) const noexcept;
    void SetName(const GUID id, const std::string & name) const noexcept;

    void AddItem(const GUID & id, const std::string & name, bool isFolder, bool isExpanded, const GUID & parentId) const noexcept;
    void RemoveItem(const GUID id) const noexcept;

    void Clear() const noexcept override;

    HTREEITEM FindItem(const GUID & id) const noexcept;

    /// <summary>
    /// Serializes this instance to JSON.
    /// </summary>
    template<typename Visitor> bool ToJSON(Visitor && visitor, json::array_t * nodes) const noexcept
    {
        return ToJSON_(TreeView_GetRoot(Get()), visitor, nodes);
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
