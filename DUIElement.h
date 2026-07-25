
/** $VER: DUIElement.h (2026.07.25) P. Stuer - Implements Default User Interface support **/

#pragma once

#include "PlaylistUIElement.h"

/// <summary>
/// Implements a Default UI element.
/// </summary>
class dui_element_t : public playlist_uielement_t, public ui_element_instance
{
public:
    dui_element_t(ui_element_config::ptr data, ui_element_instance_callback::ptr callback);

    dui_element_t(const dui_element_t &) = delete;
    dui_element_t & operator=(const dui_element_t &) = delete;
    dui_element_t(dui_element_t &&) = delete;
    dui_element_t & operator=(dui_element_t &&) = delete;

    virtual ~dui_element_t() { };

    #pragma region ui_element_instance

    static void g_get_name(pfc::string_base & name) noexcept;
    static const char * g_get_description() noexcept;
    static GUID g_get_guid() noexcept;
    static GUID g_get_subclass() noexcept;
    static ui_element_config::ptr g_get_default_configuration() noexcept;

    void initialize_window(HWND hParent) noexcept;
    void set_configuration(ui_element_config::ptr data) noexcept final;
    ui_element_config::ptr get_configuration() noexcept final;
    virtual void notify(const GUID & what, t_size param1, const void * param2, t_size param2Size);

    #pragma endregion

    void GetColors() noexcept final;
    void GetFonts() noexcept final;

protected:
    ui_element_instance_callback::ptr m_callback; // Don't rename this. BumpableElement uses it.
};
