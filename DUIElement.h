
/** $VER: DUIElement.h (2026.07.04) P. Stuer - Implements Default User Interface support **/

#pragma once

#include "PlaylistUIElement.h"

/// <summary>
/// Implements a Default UI element.
/// </summary>
class duielement_t : public playlist_uielement_t, public ui_element_instance
{
public:
    duielement_t(ui_element_config::ptr data, ui_element_instance_callback::ptr callback);

    duielement_t(const duielement_t &) = delete;
    duielement_t & operator=(const duielement_t &) = delete;
    duielement_t(duielement_t &&) = delete;
    duielement_t & operator=(duielement_t &&) = delete;

    virtual ~duielement_t() { };

    #pragma region ui_element_instance interface

    static void g_get_name(pfc::string_base & p_out);
    static const char * g_get_description();
    static GUID g_get_guid();
    static GUID g_get_subclass();
    static ui_element_config::ptr g_get_default_configuration();

    void initialize_window(HWND p_parent);
    virtual void set_configuration(ui_element_config::ptr p_data);
    virtual ui_element_config::ptr get_configuration();
    virtual void notify(const GUID & what, t_size param1, const void * param2, t_size param2Size);

    #pragma endregion

    void GetColors() noexcept override;

protected:
    ui_element_instance_callback::ptr m_callback; // Don't rename this. BumpableElement uses it.
};
