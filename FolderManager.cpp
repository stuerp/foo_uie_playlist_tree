
/** $VER: FolderManager.cpp (2026.07.06) P. Stuer **/

#include "pch.h"

#include "FolderManager.h"

#include <map>

#pragma hdrstop

const GUID folder_manager_t::class_guid = { 0x5746f1d2, 0xec43, 0x4cc6, { 0x8e, 0x58, 0x92, 0xcf, 0x2b, 0xf4, 0xa0, 0x49 } };

struct GuidLess
{
    bool operator()(const GUID & a, const GUID & b) const
    {
        return std::memcmp(&a, &b, sizeof(GUID)) < 0;
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
    virtual void CreateFolder(_In_ const pfc::string & text, _In_ const GUID & id)
    {
        _Items.emplace(id, folder_t(text));
    }

    /// <summary>
    /// Gets the name of the specified folder.
    /// </summary>
    virtual void GetFolderName(_In_ const GUID & id, _Out_ pfc::string & text) const
    {
        auto Iter = _Items.find(id);

        if (Iter == _Items.end())
        {
            text.reset();

            return;
        }

        text = Iter->second.Name;
    }

private:
    std::map<GUID, folder_t, GuidLess> _Items;
};

static service_factory_single_t<folder_manager_impl> _FolderManagerFactory;
