
/** $VER: Node.h (2026.07.07) P. Stuer **/

#pragma once

#include <guiddef.h>
#include <string>

using namespace msc;

/// <summary>
/// Represents a node of the playlist tree view.
/// </summary>
struct node_t
{
    node_t(const GUID id, const std::string & name, bool isFolder, bool isExpanded) : Name(name), Id(id), IsFolder(isFolder), IsExpanded(isExpanded) { }
    virtual ~node_t() = default;

    std::string Name;
    GUID Id;
    bool IsFolder;
    bool IsExpanded;
};
