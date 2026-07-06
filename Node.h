#pragma once

#include <guiddef.h>
#include <string>

struct node_t
{
    node_t(std::string name, GUID guid, bool isFolder) : Name(std::move(name)), Id(guid), IsFolder(isFolder) { }
    virtual ~node_t() = default;

    std::string Name;
    GUID Id;
    bool IsFolder;
};
