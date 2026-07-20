
/** $VER: FolderManager.h (2026.07.20) P. Stuer **/

#pragma once

#include <SDK\service.h>

using namespace msc;

/// <summary>
/// Declares the folder manager callback.
/// </summary>
class NOVTABLE folder_manager_callback_t
{
public:
    virtual ~folder_manager_callback_t() noexcept { };

	virtual void OnFolderCreated(const GUID & id, const std::string & name) noexcept = 0;
	virtual void OnFolderRemoved(const GUID & id) noexcept = 0;
	virtual void OnFolderRenamed(const GUID & id, const std::string & oldName, const std::string & newName) noexcept = 0;
};

/// <summary>
/// Declares the folder manager service.
/// </summary>
class NOVTABLE folder_manager_t : public service_base
{
public:
    virtual void CreateFolder(const GUID & id, const std::string & name) noexcept = 0;
    virtual void GetFolderName(const GUID & id, std::string & name) const noexcept = 0;
    virtual void SetFolderName(const GUID & id, const std::string & text) noexcept = 0;
    virtual void RemoveFolder(const GUID & id) noexcept = 0;

	virtual void RegisterCallback(folder_manager_callback_t * callback) noexcept = 0;
	virtual void UnregisterCallback(folder_manager_callback_t * callback) noexcept = 0;

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(folder_manager_t);
};

/// <summary>
/// Represents a folder that contains playlists.
/// </summary>
class folder_t
{
public:
    folder_t(const std::string & name) : Name(name)
    {
    }

    std::string Name;
};
