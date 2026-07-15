
/** $VER: CUIColorClient.h (2026.07.14) P. Stuer - Columns User Interface support **/

#pragma once

#include "PlaylistUIElement.h"
#include "Theme.h"

#include <ui_extension.h>

namespace uie
{
class cui_element_t;

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
        static const GUID Id = GUID_UI_ELEMENT;

        return Id;
    }

    virtual void get_name(pfc::string_base & out) const
    {
        out = STR_COMPONENT_NAME;
    }

    /// <summary>
    /// Return a combination of colours to indicate which colors are supported. 
    /// </summary>
    uint32_t get_supported_colours() const override
    {
        return cui::colours::colour_flag_text                    | cui::colours::colour_flag_background |
               cui::colours::colour_flag_selection_text          | cui::colours::colour_flag_selection_background |
               cui::colours::colour_flag_inactive_selection_text | cui::colours::colour_flag_inactive_selection_background;
//      return cui::colours::colour_flag_all;
    /*
         = 1 << colour_inactive_selection_text,
         = 1 << colour_inactive_selection_background,

        colour_flag_active_item_frame = 1 << colour_active_item_frame,

        colour_flag_group_foreground = 1 << colour_group_foreground,
        colour_flag_group_background = 1 << colour_group_background,
    */
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

public:
    cui_element_t * _Element = nullptr;
};
}
