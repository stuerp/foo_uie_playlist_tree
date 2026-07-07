
/** $VER: FolderManager.h (2026.07.06) P. Stuer **/

#pragma once

#include <SDK\service.h>

using namespace msc;

/// <summary>
/// Declares the folder manager service.
/// </summary>
class NOVTABLE folder_manager_t : public service_base
{
public:
    virtual void CreateFolder(_In_ const pfc::string & text, _In_ GUID id) = 0;
    virtual void GetFolderName(_In_ GUID id, _Out_ pfc::string & text) const = 0;

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(folder_manager_t);
};

/// <summary>
/// Represents a folder that contains playlists.
/// </summary>
class folder_t
{
public:
    folder_t(const pfc::string & name) : Name(name)
    {
    }

    pfc::string Name;
};
