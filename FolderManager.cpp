
/** $VER: FolderManager.cpp (2026.07.20) P. Stuer **/

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
    virtual HRESULT CreateFolder() noexcept override
    {
        // Create the new Folder item.
        std::string Name("New Folder");

        GUID Id;

        HRESULT hResult = ::CoCreateGuid(&Id);

        if (!SUCCEEDED(hResult))
            return hResult;

        return CreateFolder(Id, Name);
    }

    /// <summary>
    /// Creates a new folder.
    /// </summary>
    virtual HRESULT CreateFolder(const GUID & id, const std::string & name) noexcept override
    {
        _Items.try_emplace(id, folder_t(name));

        for (auto Callback : _Callbacks)
            Callback->OnFolderCreated(id, name);

        return S_OK;
    }

    /// <summary>
    /// Gets the name of the specified folder.
    /// </summary>
    virtual HRESULT GetFolderName(const GUID & id, std::string & text) const noexcept override
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
        {
            text.clear();

            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }

        text = Iter->second.Name;

        return S_OK;
    }

    /// <summary>
    /// Sets the name of the specified folder.
    /// </summary>
    virtual HRESULT SetFolderName(const GUID & id, const std::string & text) noexcept override
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        Iter->second.Name = text;

        return S_OK;
    }

    /// <summary>
    /// Removes the specified folder.
    /// </summary>
    virtual HRESULT RemoveFolder(const GUID & id) noexcept override
    {
        for (auto Callback : _Callbacks)
            Callback->OnFolderRemoving(id);

        _Items.erase(id);

        return S_OK;
    }

    /// <summary>
    /// Registers a callback.
    /// </summary>
    virtual HRESULT RegisterCallback(folder_manager_callback_t * callback) noexcept override
    {
        _Callbacks.insert(callback);

        return S_OK;
    }

    /// <summary>
    /// Unregisters a callback.
    /// </summary>
    virtual HRESULT UnregisterCallback(folder_manager_callback_t * callback) noexcept override
    {
        _Callbacks.erase(callback);

        return S_OK;
    }

private:
    std::map<GUID, folder_t, GUIDLess> _Items;
    std::unordered_set<folder_manager_callback_t *> _Callbacks;
};

static service_factory_single_t<folder_manager_impl> _FolderManagerFactory;
