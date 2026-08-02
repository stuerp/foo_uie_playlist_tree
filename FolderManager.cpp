
/** $VER: FolderManager.cpp (2026.08.02) P. Stuer **/

#include "pch.h"

#include "FolderManager.h"

#include <map>
#include <unordered_set>

#pragma hdrstop

const GUID folder_manager_t::class_guid = { 0x5746f1d2, 0xec43, 0x4cc6, { 0x8e, 0x58, 0x92, 0xcf, 0x2b, 0xf4, 0xa0, 0x49 } };

struct GUIDLess
{
    bool operator()(const GUID & l, const GUID & r) const
    {
        return std::memcmp(&l, &r, sizeof(GUID)) < 0;
    }
};

/// <summary>
/// Implements the folder manager service.
/// </summary>
class folder_manager_impl : public folder_manager_t
{
public:
    folder_manager_impl() = default;

    folder_manager_impl(const folder_manager_impl &) = delete;
    folder_manager_impl & operator=(const folder_manager_impl &) = delete;
    folder_manager_impl(folder_manager_impl &&) = delete;
    folder_manager_impl & operator=(folder_manager_impl &&) = delete;

    virtual ~folder_manager_impl() = default;

    /// <summary>
    /// Creates a new folder with a default name.
    /// </summary>
    HRESULT CreateFolder() noexcept final
    {
        // Create the new Folder item.
        std::string Name("New Folder");

        GUID Id;

        HRESULT hr = ::CoCreateGuid(&Id);

        if (!SUCCEEDED(hr))
            return hr;

        return CreateFolder(Id, Name);
    }

    /// <summary>
    /// Creates a new folder.
    /// </summary>
    HRESULT CreateFolder(const GUID & id, const std::string & name) noexcept final
    {
        _Items.try_emplace(id, folder_t(name));

        for (auto Callback : _Callbacks)
            Callback->OnFolderCreated(id, name);

        return S_OK;
    }

    /// <summary>
    /// Gets the name of the specified folder.
    /// </summary>
    HRESULT GetFolderName(const GUID & id, std::string & name) const noexcept final
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
        {
            name.clear();

            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }

        name = Iter->second.Name;

        return S_OK;
    }

    /// <summary>
    /// Sets the name of the specified folder.
    /// </summary>
    HRESULT SetFolderName(const GUID & id, const std::string & newName) noexcept final
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        const std::string OldName = Iter->second.Name;

        Iter->second.Name = newName;

        for (auto Callback : _Callbacks)
            Callback->OnFolderRenamed(id, OldName, newName);

        return S_OK;
    }

    /// <summary>
    /// Removes the specified folder.
    /// </summary>
    HRESULT RemoveFolder(const GUID & id) noexcept final
    {
        for (auto Callback : _Callbacks)
            Callback->OnFolderRemoving(id);

        _Items.erase(id);

        for (auto Callback : _Callbacks)
            Callback->OnFolderRemoved(id);

        return S_OK;
    }

    /// <summary>
    /// Returns the number of folders.
    /// </summary>
    HRESULT GetFolderCount(uint32_t & count) const noexcept final
    {
        count = (uint32_t) _Items.size();

        return S_OK;
    }

    /// <summary>
    /// Registers a callback.
    /// </summary>
    HRESULT RegisterCallback(folder_manager_callback_t * callback) noexcept final
    {
        _Callbacks.insert(callback);

        return S_OK;
    }

    /// <summary>
    /// Unregisters a callback.
    /// </summary>
    HRESULT UnregisterCallback(folder_manager_callback_t * callback) noexcept final
    {
        _Callbacks.erase(callback);

        return S_OK;
    }

private:
    std::map<GUID, folder_t, GUIDLess> _Items;
    std::unordered_set<folder_manager_callback_t *> _Callbacks;
};

static service_factory_single_t<folder_manager_impl> _FolderManagerFactory;
