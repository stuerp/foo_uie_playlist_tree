
/** $VER: PlaylistLockManager.cpp (2026.07.25) P. Stuer **/

#include "pch.h"

#include "PlaylistLockManager.h"

#include <unordered_map>

#pragma hdrstop

const GUID playlist_lock_manager_t::class_guid = { 0xa4e1acc9, 0xded6, 0x47c6, { 0x89, 0x69, 0xfb, 0x78, 0xad, 0xd6, 0x30, 0x26 } };

namespace std
{

/// <summary>
/// Hash function for GUIDs (used by unordered map).
/// </summary>
template<> struct hash<GUID>
{
    size_t operator()(const GUID & id) const noexcept
    {
        const uint64_t * p = (const uint64_t *) &id;

        std::hash<uint64_t> Hash;

        return Hash(p[0]) ^ Hash(p[1]);
    }
};

}

/// <summary>
/// Implements the playlist lock manager service.
/// </summary>
class playlist_lock_manager_impl : public playlist_lock_manager_t
{
public:
    playlist_lock_manager_impl() = default;

    playlist_lock_manager_impl(const playlist_lock_manager_impl &) = delete;
    playlist_lock_manager_impl & operator=(const playlist_lock_manager_impl &) = delete;
    playlist_lock_manager_impl(playlist_lock_manager_impl &&) = delete;
    playlist_lock_manager_impl & operator=(playlist_lock_manager_impl &&) = delete;

    virtual ~playlist_lock_manager_impl() = default;

    /// <summary>
    /// Adds a lock to the specified playlist.
    /// </summary>
    HRESULT Add(const GUID & id, uint32_t mask) noexcept final
    {
        if ((id == GUID_NULL) || (mask == 0u))
            return E_FAIL;

        static_api_ptr_t<playlist_manager_v5> pm;

        const auto Index = pm->find_playlist_by_guid(id);

        if (pm->playlist_lock_is_present(Index))
            return E_FAIL;

        auto Lock = fb2k::service_new<playlist_lock_t>(mask);

        if (!pm->playlist_lock_install(Index, Lock))
            return E_FAIL;

        _Locks.try_emplace(id, Lock);

        return S_OK;
    }

    /// <summary>
    /// Removes a lock from the specified playlist.
    /// </summary>
    HRESULT Remove(const GUID & id) noexcept final
    {
        if (id == GUID_NULL)
            return E_FAIL;

        const auto Iter = _Locks.find(id);

        if (Iter == _Locks.end())
            return E_FAIL;

        static_api_ptr_t<playlist_manager_v5> pm;

        const auto Index = pm->find_playlist_by_guid(id);

        if (Index == SIZE_MAX)
            return E_FAIL;

        const bool Success = pm->playlist_lock_uninstall(Index, Iter->second);

        _Locks.erase(id);

        return Success ? S_OK : E_FAIL;
    }

    /// <summary>
    /// Returns true if the playlist has been locked by this component.
    /// </summary>
    bool IsLocked(const GUID & id) noexcept final
    {
        return _Locks.contains(id);
    }

private:
    std::unordered_map<GUID, playlist_lock::ptr> _Locks;
};

static service_factory_single_t<playlist_lock_manager_impl> _PlaylistLockManagerFactory;
