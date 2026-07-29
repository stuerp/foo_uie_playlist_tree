
/** $VER: PlaylistLockManager.cpp (2026.07.29) P. Stuer **/

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
    /// Locks the specified playlist. If a lock already exists, modify it with the new filter mask.
    /// </summary>
    HRESULT LockPlaylist(const GUID & id, uint32_t filterMask) noexcept final
    {
        if ((id == GUID_NULL) || (filterMask == 0u))
            return E_INVALIDARG;

        static_api_ptr_t<playlist_manager_v5> pm;

        const auto Index = pm->find_playlist_by_guid(id);

        if (Index == SIZE_MAX)
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        if (pm->playlist_lock_is_present(Index))
        {
            HRESULT hResult = UnlockPlaylist(id);

            if (!SUCCEEDED(hResult))
                return hResult; // Already locked but not by me.
        }

        auto Lock = fb2k::service_new<playlist_lock_t>(filterMask);

        if (!pm->playlist_lock_install(Index, Lock))
            return HRESULT_FROM_WIN32(ERROR_LOCK_FAILED);

        _Locks.try_emplace(id, Lock);

        return S_OK;
    }

    /// <summary>
    /// Unlocks the specified playlist.
    /// </summary>
    HRESULT UnlockPlaylist(const GUID & id) noexcept final
    {
        if (id == GUID_NULL)
            return E_INVALIDARG;

        const auto Iter = _Locks.find(id);

        if (Iter == _Locks.end())
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        static_api_ptr_t<playlist_manager_v5> pm;

        const auto Index = pm->find_playlist_by_guid(id);

        if (Index == SIZE_MAX)
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        const bool Success = pm->playlist_lock_uninstall(Index, Iter->second);

        _Locks.erase(id);

        return Success ? S_OK : HRESULT_FROM_WIN32(ERROR_WAS_UNLOCKED);
    }

    /// <summary>
    /// Returns true if the playlist has been locked by this component.
    /// </summary>
    bool IsLockedByMe(const GUID & id) const noexcept final
    {
        return _Locks.contains(id);
    }

    /// <summary>
    /// Returns the filter mask of a lock if present.
    /// </summary>
    HRESULT GetFilterMask(const GUID & id, uint32_t & filterMask) const noexcept final
    {
        if (id == GUID_NULL)
            return E_INVALIDARG;

        const auto Iter = _Locks.find(id);

        if (Iter == _Locks.end())
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        filterMask = Iter->second->get_filter_mask();

        return S_OK;
    }

private:
    std::unordered_map<GUID, playlist_lock::ptr> _Locks;
};

static service_factory_single_t<playlist_lock_manager_impl> _PlaylistLockManagerFactory;
