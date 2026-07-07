
/** $VER: PlaylistsTreeView.h (2026.07.07) P. Stuer **/

#pragma once

#include "TreeView.h"

#include <nlohmann\json.hpp>

using json = nlohmann::ordered_json;

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

    std::string GetText(_In_ const GUID id) const noexcept;
    void SetText(_In_ const GUID id, _In_ const std::string & text) const noexcept;

    void AddItem(_In_ const GUID id, _In_ const std::string & name, _In_ const std::string & text, _In_ bool isFolder) const noexcept;
    void RemoveItem(_In_ const GUID id) const noexcept;

    void Clear() const noexcept override;

    HTREEITEM FindItem(_In_ const GUID id) const noexcept;

    /// <summary>
    /// Serializes this intance to JSON.
    /// </summary>
    template<typename Visitor> bool ToJSON(_In_ Visitor && visitor, json::array_t * nodes) const noexcept
    {
        return ToJSON_(NULL, visitor, nodes);
    }

private:
    /// <summary>
    /// Serializes this intance to JSON.
    /// </summary>
    template<typename Visitor> bool ToJSON_(HTREEITEM hParent, _In_ Visitor && visitor, json::array_t * nodes) const noexcept
    {
        HTREEITEM hItem = (hParent == NULL) ? TreeView_GetRoot(Get()) : TreeView_GetChild(Get(), hParent);

        while (hItem != NULL)
        {
            json::object_t * Object = new json::object_t;

            if (!visitor(hItem, Object))
                return false;

            json::array_t Nodes;

            if (!ToJSON_(hItem, visitor, &Nodes))
                return false;

            if (Nodes.size() != 0)
                (*Object)["nodes"] = Nodes;

            (*nodes).push_back(*Object);

            hItem = TreeView_GetNextSibling(Get(), hItem);
        }

        return true;
    }

public:
/*

    // shell32.dll icons
    enum Icon : int
    {
        File    = 0,
        Folder  = 3
    };
*/
    // imageres.dll icons
    enum Icon : int
    {
        File    = 126,
        Folder  = 4
    };
};
