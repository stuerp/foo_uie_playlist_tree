#pragma once

#include <guiddef.h>
#include <string>

using namespace msc;

struct node_t
{
    node_t(const GUID guid, bool isFolder) : Id(guid), IsFolder(isFolder) { }
    virtual ~node_t() = default;

    GUID Id;
    bool IsFolder;
};
