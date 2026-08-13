
/** $VER: Node.h (2026.08.13) P. Stuer **/

#pragma once

#include <guiddef.h>
#include <string>

using namespace msc;

/// <summary>
/// Represents a node of the playlist tree view.
/// </summary>
struct node_t
{
    node_t(const GUID id, const std::string & name, bool isFolder, bool isFrozen) : Name(name), Id(id), IsFolder(isFolder), IsFrozen(isFrozen) { }
    virtual ~node_t() = default;

    std::string Name;
    GUID Id;
    bool IsFolder;
    bool IsFrozen;
};
