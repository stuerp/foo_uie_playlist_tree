#pragma once

#include <guiddef.h>
#include <string>

using namespace msc;

struct node_t
{
    node_t(const std::string & name, const GUID guid, bool isFolder) : Name(name), Id(guid), IsFolder(isFolder) { }
    virtual ~node_t() = default;

    std::string Name;
    GUID Id;
    bool IsFolder;
};
