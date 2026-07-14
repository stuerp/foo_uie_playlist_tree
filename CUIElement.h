
/** $VER: CUIElement.h (2026.07.14) P. Stuer - Columns User Interface support **/

#pragma once

#include "PlaylistUIElement.h"

#include <ui_extension.h>

namespace uie
{

class cui_color_client_t;

/// <summary>
/// Implements a Columns UI element.
/// </summary>
class cui_element_t : public playlist_uielement_t, public uie::window
{
public:
    cui_element_t() { }

    cui_element_t(const cui_element_t &) = delete;
    cui_element_t & operator=(const cui_element_t &) = delete;
    cui_element_t(cui_element_t &&) = delete;
    cui_element_t & operator=(cui_element_t &&) = delete;

    virtual ~cui_element_t() { }

    #pragma region uie::window

    /// <summary>
    /// Gets the category of the extension.
    /// </summary>
    void get_category(pfc::string_base & out) const final
    {
        out = "Panels";
    }

    /// <summary>
    /// Gets the type of the extension.
    /// </summary>
    uint32_t get_type() const final
    {
        return uie::type_panel;
    }

    HWND create_or_transfer_window(HWND parent, const window_host_ptr & newHost, const ui_helpers::window_position_t & position);

    virtual void destroy_window();

    /// <summary>
    /// Returns true if the extension is available.
    /// </summary>
    virtual bool is_available(const window_host_ptr & p) const
    {
        return true;
    }

    /// <summary>
    /// Gets the window handle of the extension.
    /// </summary>
    virtual HWND get_wnd() const
    {
        return *this;
    }

    #pragma endregion

    #pragma region extension_base

    /// <summary>
    /// Gets the unique ID of extension.
    /// </summary>
    const GUID & get_extension_guid() const
    {
        return GetGUID();
    }

    /// <summary>
    /// Gets a user-readable name of the extension.
    /// </summary>
    void get_name(pfc::string_base & out) const
    {
        out = STR_COMPONENT_NAME;
    }

    void set_config(stream_reader * stream, size_t size, abort_callback & abortHandler) final;
    void get_config(stream_writer * stream, abort_callback & abortHandler) const final;

    #pragma endregion

    void GetColors() noexcept override;

private:
    window_host_ptr _Host;
    HWND _hParent;
};

}
