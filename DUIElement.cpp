
/** $VER: DUIElement.cpp (2026.07.10) P. Stuer - Implements Default User Interface support **/

#include "pch.h"

#include "DUIElement.h"
#include "Node.h"
#include "State.h"
#include "Theme.h"

#include <helpers\BumpableElem.h>

#pragma hdrstop

#pragma region ui_element_instance interface

/// <summary>
/// Initializes a new instance.
/// </summary>
duielement_t::duielement_t(ui_element_config::ptr data, ui_element_instance_callback::ptr callback) : m_callback(callback)
{
    set_configuration(data);

    GetColors();
}

/// <summary>
/// Retrieves the name of the element.
/// </summary>
void duielement_t::g_get_name(pfc::string_base & name)
{
    name = STR_COMPONENT_NAME;
}

/// <summary>
/// Retrieves the description of the element.
/// </summary>
const char * duielement_t::g_get_description()
{
    return STR_COMPONENT_DESCRIPTION;
}

/// <summary>
/// Retrieves the GUID of the element.
/// </summary>
GUID duielement_t::g_get_guid()
{
    return uielement_t::GetGUID();
}

/// <summary>
/// Retrieves the subclass GUID of the element.
/// </summary>
GUID duielement_t::g_get_subclass()
{
    return ui_element_subclass_utility;
}

/// <summary>
/// Retrieves the default configuration of the element.
/// </summary>
ui_element_config::ptr duielement_t::g_get_default_configuration()
{
    state_t DefaultState;

    std::string Config = DefaultState.ToJSON().dump(-1);

    return ui_element_config::g_create(g_get_guid(), Config.c_str(), Config.size());
}

/// <summary>
/// Initializes the element's windows.
/// </summary>
void duielement_t::initialize_window(HWND hWndParent)
{
    const DWORD Style = 0;
    const DWORD ExStyle = 0;

    this->Create(hWndParent, nullptr, nullptr, Style, ExStyle);
}

/// <summary>
/// Sets the element configuration.
/// </summary>
void duielement_t::set_configuration(ui_element_config::ptr data)
{
    ui_element_config_parser Parser(data);

    _State.FromJSON((const char *) data->get_data(), data->get_data_size());
}

/// <summary>
/// Gets the element configuration.
/// </summary>
ui_element_config::ptr duielement_t::get_configuration()
{
    auto Object = _State.ToJSON();

    json::array_t Nodes;

    _TreeView.ToJSON([&](HTREEITEM hItem, json::object_t * node) -> bool
    {
        auto Node = (const node_t *) _TreeView.GetData(hItem);

        if (Node == nullptr)
            return true; // Continue enumerating. Should not occur.

        (*node)["id"]       = msc::GUIDToUTF8(Node->Id);
        (*node)["name"]     = Node->Name;
/*
        (*node)["image"]    =
        {
            { "filePath", "test" },
            { "index", 42 }
        };
*/
        (*node)["isFolder"] = Node->IsFolder;

        if (Node->IsFolder)
            (*node)["isExpanded"] = Node->IsExpanded;

        return true; // Continue enumerating.
    }, &Nodes);

    _TreeView.Clear();

    Object["nodes"] = Nodes;

    std::string Config = Object.dump(-1);

::OutputDebugStringA(Object.dump(4).c_str()); // FIXME

    return ui_element_config::g_create(g_get_guid(), Config.c_str(), Config.size());
}

/// <summary>
/// Used by the host to notify the element about various events.
/// See ui_element_notify_* GUIDs for possible "what" parameter; meaning of other parameters depends on the "what" value.
/// Container classes should dispatch all notifications to their children.
/// </summary>
void duielement_t::notify(const GUID & what, t_size param1, const void * param2, t_size param2Size)
{
    if (what == ui_element_notify_colors_changed)
        GetColors();
}

/// <summary>
/// Gets the colors.
/// </summary>
void duielement_t::GetColors() noexcept
{
    _Theme.SetColor(COLOR_WINDOW,    (COLORREF) m_callback->query_std_color(ui_color_background));
    _Theme.SetColor(COLOR_WINDOWTEXT,(COLORREF) m_callback->query_std_color(ui_color_text));
    _Theme.SetColor(COLOR_HIGHLIGHT, (COLORREF) m_callback->query_std_color(ui_color_selection));
    _Theme.SetColor(COLOR_HOTLIGHT,  (COLORREF) m_callback->query_std_color(ui_color_highlight));
}

static service_factory_single_t<ui_element_impl_withpopup<duielement_t>> _Factory;

#pragma endregion
