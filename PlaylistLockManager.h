
/** $VER: PlaylistLockManager.h (2026.07.25) P. Stuer **/

#pragma once

#include "Resources.h"

#include <SDK\playlist.h>

/// <summary>
/// Implements our lock.
/// </summary>
class playlist_lock_t : public playlist_lock
{
public:
    playlist_lock_t(uint32_t mask) noexcept : _Mask(mask) { };

    playlist_lock_t(const playlist_lock_t &&) = delete;
    playlist_lock_t & operator=(playlist_lock_t &&) = delete;

    virtual ~playlist_lock_t() noexcept { };

    #pragma region playlist_lock

    /// <summary>
    /// Queries whether specified item insertion operation is allowed in the locked playlist.
    /// </summary>
    /// <param name="sourceIndex">Index from which the items are being inserted</param>
    /// <param name="items">Items being inserted</param>
    /// <param name="selection">Caller-requested selection state of items being inserted</param>
    /// <returns>True to allow the operation, false to block it</returns>
    bool query_items_add(t_size sourceIndex, const pfc::list_base_const_t<metadb_handle_ptr> & items, const bit_array & selection) noexcept final
    {
        return ((_Mask & playlist_lock::filter_add) != 0);
    }

    /// <summary>
    /// Queries whether specified item reorder operation is allowed in the locked playlist.
    /// </summary>
    /// <param name="order">Pointer to array containing permutation defining requested reorder operation</param>
    /// <param name="count">Number of items in array pointed to by 'order'. This should always be equal to number of items on the locked playlist</param>
    /// <returns>True to allow the operation, false to block it</returns>
    bool query_items_reorder(const t_size * p_order, t_size p_count) noexcept final
    {
        return ((_Mask & playlist_lock::filter_reorder) != 0);
    }

    /// <summary>
    /// Queries whether specified item removal operation is allowed in the locked playlist.
    /// </summary>
    /// <param name="mask">Specifies which items from locked playlist are being removed</param>
    /// <param name="force">If set to true, the call is made only for notification purpose and items are getting removed regardless (after e.g. they have been physically removed)</param>
    /// <returns>True to allow the operation, false to block it. Note that return value is ignored if p_force is set to true</returns>
    bool query_items_remove(const bit_array & mask, bool force) noexcept final
    {
        return ((_Mask & playlist_lock::filter_remove) != 0);
    }

    /// <summary>
    /// Queries whether specified item replacement operation is allowed in the locked playlist.
    /// </summary>
    /// <param name="itemIndex">Index of the item being replaced</param>
    /// <param name="oldHandle">Old value of the item being replaced</param>
    /// <param name="newHandle">New value of the item being replaced</param>
    /// <returns>True to allow the operation, false to block it</returns>
    bool query_item_replace(t_size itemIndex, const metadb_handle_ptr & oldHandle, const metadb_handle_ptr & newHandle) noexcept final
    {
        return ((_Mask & playlist_lock::filter_replace) != 0);
    }

    /// <summary>
    /// Queries whether renaming the locked playlist is allowed.
    /// </summary>
    /// <param name="newName">Requested new name of the playlist; a UTF-8 encoded string</param>
    /// <param name="newNameSize">Length limit of the name string, in bytes (actual string may be shorter if null terminator is encountered before). Set this to infinite to use plain null-terminated strings</param>
    /// <returns>True to allow the operation, false to block it</returns>
    bool query_playlist_rename(const char * p_new_name, t_size p_new_name_len) noexcept final
    {
        return ((_Mask & playlist_lock::filter_rename) != 0);
    }

    /// <summary>
    /// Queries whether removal of the locked playlist is allowed. Note that the lock will be released when the playlist is removed.
    /// </summary>
    /// <returns>True to allow the operation, false to block it</returns>
    bool query_playlist_remove() noexcept final
    {
        return ((_Mask & playlist_lock::filter_remove_playlist) != 0);
    }

    /// <summary>
    /// Executes "default action" (doubleclick etc) for specified playlist item. When the playlist is not locked, default action starts playback of the item.
    /// </summary>
    /// <returns>True if custom default action was executed, false to fall-through to default one for non-locked playlists (start playback).</returns>
    bool execute_default_action(t_size p_item) noexcept final
    {
        return false;
    }

    /// <summary>
    /// Notifies lock about changed index of the playlist, in result of user reordering playlists or removing other playlists.
    /// </summary>
    void on_playlist_index_change(t_size newIndex) noexcept final
    {
    }

    /// <summary>
    /// Notifies lock about the locked playlist getting removed.
    /// </summary>
    void on_playlist_remove() noexcept final
    {
    }

    /// <summary>
    /// Retrieves human-readable name of playlist lock to display.
    /// </summary>
    void get_lock_name(pfc::string_base & name) noexcept final
    {
        name = STR_COMPONENT_NAME;
    }

    /// <summary>
    /// Requests user interface of component controlling the playlist lock to be shown.
    /// </summary>
    void show_ui() noexcept final
    {
    }

    /// <summary>
    /// Queries which actions the lock filters. The return value must not change while the lock is registered with playlist_manager.
    /// The return value is a combination of one or more filter_* constants.
    /// </summary>
    t_uint32 get_filter_mask() noexcept final
    {
        return _Mask;
    }

    #pragma endregion

private:
    uint32_t _Mask;
};

/// <summary>
/// Declares the playlist lock manager service.
/// </summary>
class NOVTABLE  playlist_lock_manager_t : public service_base
{
public:
    virtual HRESULT Add(const GUID & id, uint32_t mask) noexcept = 0;
    virtual HRESULT Remove(const GUID & id) noexcept = 0;
    virtual bool IsLocked(const GUID & id) noexcept = 0;

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(playlist_lock_manager_t);
};
