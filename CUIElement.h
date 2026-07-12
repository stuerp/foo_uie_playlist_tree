
/** $VER: cuielement_t.h (2026.07.12) P. Stuer - Columns User Interface support **/

#pragma once

#include "PlaylistUIElement.h"

#include <ui_extension.h>

namespace uie
{
class cui_color_client_t;

/// <summary>
/// Implements a Columns UI element.
/// </summary>
class cuielement_t :  public playlist_uielement_t, public uie::window
{
public:
    cuielement_t();

    cuielement_t(const cuielement_t &) = delete;
    cuielement_t & operator=(const cuielement_t &) = delete;
    cuielement_t(cuielement_t &&) = delete;
    cuielement_t & operator=(cuielement_t &&) = delete;

    virtual ~cuielement_t() { }

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

static std::vector<cuielement_t *> _Elements; // Very ugly but necessary because of the weird CUI notification mechanism.

/// <summary>
/// Receives notifications from CUI when the colors change.
/// </summary>
class cui_color_client_t : public cui::colours::client
{
public:
    cui_color_client_t() { }

    cui_color_client_t(const cui_color_client_t &) = delete;
    cui_color_client_t & operator=(const cui_color_client_t &) = delete;
    cui_color_client_t(cui_color_client_t &&) = delete;
    cui_color_client_t & operator=(cui_color_client_t &&) = delete;

    virtual ~cui_color_client_t() { }

    #pragma region cui::colours::client

    virtual const GUID & get_client_guid() const
    {
        static const GUID guid = GUID_UI_ELEMENT;

        return guid;
   }

    virtual void get_name(pfc::string_base & out) const
    {
        out = STR_COMPONENT_NAME;
    }

    /// <summary>
    /// Return a combination of bool_flag_t to indicate which boolean flags are supported. 
    /// </summary>
    virtual uint32_t get_supported_bools() const override
    {
        return cui::colours::bool_dark_mode_enabled;
    }

    /// <summary>
    /// Indicates whether the Theme API is supported.
    /// </summary>
    virtual bool get_themes_supported() const override
    {
        return false;
    }

    virtual void on_colour_changed(uint32_t changed_items_mask) const override;

    /// <summary>
    /// Called whenever a supported boolean flag changes. Support for a flag is determined using the get_supported_bools() method.
    /// </summary>
    virtual void on_bool_changed(uint32_t changed_items_mask) const override;

    #pragma endregion

    /// <summary>
    /// Registers a cuielement_t with this client.
    /// </summary>
    static void Register(cuielement_t * element)
    {
        if (element == nullptr)
            return;

        _Elements.push_back(element);
    }

    /// <summary>
    /// Unregisters a cuielement_t from this client.
    /// </summary>
    static void Unregister(cuielement_t * element)
    {
        if (auto Element = std::find(_Elements.begin(), _Elements.end(), element); Element != _Elements.end())
            _Elements.erase(Element);
    }
};

}
