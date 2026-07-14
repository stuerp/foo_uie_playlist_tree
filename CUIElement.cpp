
/** $VER: CUIElement.cpp (2026.07.14) P. Stuer **/

#include "pch.h"

#include "CUIElement.h"
#include "CUIColorClient.h"
#include "Resources.h"
#include "State.h"
#include "Theme.h"

#pragma hdrstop

namespace uie
{

static uie::window_factory<cui_element_t> _CUIElementFactory;

static cui::colours::client::factory<cui_color_client_t> _CUIColorClientFactory;

/// <summary>
/// Creates or transfers the window.
/// </summary>
HWND cui_element_t::create_or_transfer_window(HWND hParent, const window_host_ptr & newHost, const ui_helpers::window_position_t & position)
{
    _hParent = hParent;

    if (*this == nullptr)
    {
        _Host = newHost;

        CRect r;

        position.convert_to_rect(r);

        Create(hParent, r, 0, WS_CHILD, 0);
    }
    else
    {
        ShowWindow(SW_HIDE);
        SetParent(hParent);

        _Host->relinquish_ownership(*this);
        _Host = newHost;

        SetWindowPos(NULL, position.x, position.y, (int) position.cx, (int) position.cy, SWP_NOZORDER);
    }

    auto Client = &_CUIColorClientFactory.get_static_instance();

    if (Client != nullptr)
        Client->_Element = this;

    GetColors();

    return *this;
}

/// <summary>
/// Destroys the window.
/// </summary>
void cui_element_t::destroy_window()
{
    ::DestroyWindow(*this);

    _Host.release();
}

/// <summary>
/// Sets the instance configuration data.
/// </summary>
void cui_element_t::set_config(stream_reader * stream, size_t size, abort_callback & abortHandler)
{
    std::vector<uint8_t> Config(size);

    stream->read(Config.data(), size, abortHandler);

    SetConfiguration((const char *) Config.data(), Config.size());
}

/// <summary>
/// Gets the instance configuration data.
/// </summary>
void cui_element_t::get_config(stream_writer * stream, abort_callback & abortHandler) const
{
    const auto Config = GetConfiguration();

    stream->write_string_raw(Config.c_str(), abortHandler);
}

/// <summary>
/// Gets the colors.
/// </summary>
void cui_element_t::GetColors() noexcept
{
    cui::colours::helper Helper(GUID_UI_ELEMENT); // Use pfc::guid_null for Global

    _Theme.SetColor(COLOR_WINDOW,        Helper.get_colour(cui::colours::colour_background));
    _Theme.SetColor(COLOR_WINDOWTEXT,    Helper.get_colour(cui::colours::colour_text));

    _Theme.SetColor(COLOR_HIGHLIGHT,     Helper.get_colour(cui::colours::colour_selection_background));
    _Theme.SetColor(COLOR_HIGHLIGHTTEXT, Helper.get_colour(cui::colours::colour_selection_text));

    _Theme.SetColor(COLOR_MENUHILIGHT,   Helper.get_colour(cui::colours::colour_inactive_selection_background));
    _Theme.SetColor(COLOR_GRAYTEXT,      Helper.get_colour(cui::colours::colour_inactive_selection_text));

    _Theme.SetColor(COLOR_HOTLIGHT,      Helper.get_colour(cui::colours::colour_active_item_frame));


    TreeView_SetBkColor(_TreeView.Get(), _Theme.GetColor(COLOR_WINDOW));
    TreeView_SetTextColor(_TreeView.Get(), _Theme.GetColor(COLOR_WINDOWTEXT));

    ::InvalidateRect(m_hWnd, nullptr, TRUE);
}

}
