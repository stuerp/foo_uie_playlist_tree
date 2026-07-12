
/** $VER: FolderManager.cpp (2026.07.12) P. Stuer **/

#include "pch.h"

#include "FolderManager.h"

#include <map>

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
    /// Creates a new folder.
    /// </summary>
    virtual void CreateFolder(const GUID & id, const std::string & name) noexcept
    {
        _Items.emplace(id, folder_t(name));
    }

    /// <summary>
    /// Gets the name of the specified folder.
    /// </summary>
    virtual void GetFolderName(const GUID & id, std::string & text) const noexcept
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
        {
            text.clear();

            return;
        }

        text = Iter->second.Name;
    }

    /// <summary>
    /// Sets the name of the specified folder.
    /// </summary>
    virtual void SetFolderName(const GUID & id, const std::string & text) noexcept
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
            return;

        Iter->second.Name = text;
    }

    /// <summary>
    /// Removes the specified folder.
    /// </summary>
    virtual void RemoveFolder(const GUID & id) noexcept
    {
        _Items.erase(id);
    }

private:
    std::map<GUID, folder_t, GUIDLess> _Items;
};

static service_factory_single_t<folder_manager_impl> _FolderManagerFactory;
