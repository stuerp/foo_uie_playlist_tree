
/** $VER: DUIElement.cpp (2026.07.17) P. Stuer - Implements Default User Interface support **/

#include "pch.h"

#include "DUIElement.h"
//#include "State.h"
#include "Theme.h"

#include <helpers\BumpableElem.h>

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
dui_element_t::dui_element_t(ui_element_config::ptr data, ui_element_instance_callback::ptr callback) : m_callback(callback)
{
    _Theme._IsDUI = true;

    set_configuration(data);

    // Call here before the colors have been initialized.
    _Theme.Initialize();

    GetColors();
}

#pragma region ui_element_instance interface

/// <summary>
/// Retrieves the name of the element.
/// </summary>
void dui_element_t::g_get_name(pfc::string_base & name)
{
    name = STR_COMPONENT_NAME;
}

/// <summary>
/// Retrieves the description of the element.
/// </summary>
const char * dui_element_t::g_get_description()
{
    return STR_COMPONENT_DESCRIPTION;
}

/// <summary>
/// Retrieves the GUID of the element.
/// </summary>
GUID dui_element_t::g_get_guid()
{
    return uielement_t::GetGUID();
}

/// <summary>
/// Retrieves the subclass GUID of the element.
/// </summary>
GUID dui_element_t::g_get_subclass()
{
    return ui_element_subclass_utility;
}

/// <summary>
/// Retrieves the default configuration of the element.
/// </summary>
ui_element_config::ptr dui_element_t::g_get_default_configuration()
{
    const auto Config = GetDefaultConfiguration();

    return ui_element_config::g_create(g_get_guid(), Config.c_str(), Config.size());
}

/// <summary>
/// Initializes the element's windows.
/// </summary>
void dui_element_t::initialize_window(HWND hWndParent)
{
    const DWORD Style = 0;
    const DWORD ExStyle = 0;

    this->Create(hWndParent, nullptr, nullptr, Style, ExStyle);
}

/// <summary>
/// Sets the instance configuration data.
/// </summary>
void dui_element_t::set_configuration(ui_element_config::ptr data)
{
    ui_element_config_parser Parser(data);

    SetConfiguration((const char *) data->get_data(), data->get_data_size());
}

/// <summary>
/// Gets the instance configuration data.
/// </summary>
ui_element_config::ptr dui_element_t::get_configuration()
{
    const auto Config = GetConfiguration();

    return ui_element_config::g_create(g_get_guid(), Config.c_str(), Config.size());
}

/// <summary>
/// Used by the host to notify the element about various events.
/// See ui_element_notify_* GUIDs for possible "what" parameter; meaning of other parameters depends on the "what" value.
/// Container classes should dispatch all notifications to their children.
/// </summary>
void dui_element_t::notify(const GUID & what, t_size param1, const void * param2, t_size param2Size)
{
    if (what == ui_element_notify_colors_changed)
        OnColorsChanged();
}

/// <summary>
/// Gets the colors.
/// </summary>
void dui_element_t::GetColors() noexcept
{
    _Theme.SetWindowColor               ((COLORREF) m_callback->query_std_color(ui_color_background));
    _Theme.SetWindowTextColor           ((COLORREF) m_callback->query_std_color(ui_color_text));

    _Theme.SetSelectionColor            ((COLORREF) m_callback->query_std_color(ui_color_selection));
    _Theme.SetSelectionTextColor        ((COLORREF) m_callback->query_std_color(ui_color_text));

    _Theme.SetInactiveSelectionColor    ((COLORREF) m_callback->query_std_color(ui_color_selection));
    _Theme.SetInactiveSelectionTextColor((COLORREF) m_callback->query_std_color(ui_color_text));

    _Theme.SetHighlightColor            ((COLORREF) m_callback->query_std_color(ui_color_highlight));
    _Theme.SetHighlightTextColor        ((COLORREF) m_callback->query_std_color(ui_color_text));
/*
    _Theme.SetWindowColor               ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_background): ::GetSysColor(COLOR_WINDOW));
    _Theme.SetWindowTextColor           ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_text)      : ::GetSysColor(COLOR_WINDOWTEXT));

    _Theme.SetSelectionColor            ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_selection) : ::GetSysColor(COLOR_HIGHLIGHT));
    _Theme.SetSelectionTextColor        ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_text)      : ::GetSysColor(COLOR_HIGHLIGHTTEXT));

    _Theme.SetInactiveSelectionColor    ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_selection) : ::GetSysColor(COLOR_BTNFACE));
    _Theme.SetInactiveSelectionTextColor((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_text)      : ::GetSysColor(COLOR_BTNTEXT));

    _Theme.SetHighlightColor            ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_highlight) : ::GetSysColor(COLOR_HIGHLIGHT));
    _Theme.SetHighlightTextColor        ((COLORREF) _DarkMode ? m_callback->query_std_color(ui_color_text)      : ::GetSysColor(COLOR_HIGHLIGHTTEXT));
*/
    TreeView_SetBkColor  (_TreeView.Get(), _Theme.GetWindowColor());
    TreeView_SetTextColor(_TreeView.Get(), _Theme.GetWindowTextColor());

    ::InvalidateRect(_TreeView.Get(), nullptr, TRUE);
}

static service_factory_single_t<ui_element_impl_withpopup<dui_element_t>> _Factory;

#pragma endregion
