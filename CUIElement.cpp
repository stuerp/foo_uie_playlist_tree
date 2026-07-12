
/** $VER: cuielement_t.cpp (2026.07.12) P. Stuer **/

#include "pch.h"

#include "CUIElement.h"
#include "State.h"
#include "Theme.h"

#pragma hdrstop

namespace uie
{
#pragma region cuielement_t

/// <summary>
/// Initializes a new instance.
/// </summary>
cuielement_t::cuielement_t()
{
    GetColors();
}

/// <summary>
/// Creates or transfers the window.
/// </summary>
HWND cuielement_t::create_or_transfer_window(HWND hParent, const window_host_ptr & newHost, const ui_helpers::window_position_t & position)
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

    cui_color_client_t::Register(this);

    return *this;
}

/// <summary>
/// Destroys the window.
/// </summary>
void cuielement_t::destroy_window()
{
    cui_color_client_t::Unregister(this);

    ::DestroyWindow(*this);

    _Host.release();
}

/// <summary>
/// Sets the instance configuration data.
/// </summary>
void cuielement_t::set_config(stream_reader * stream, size_t size, abort_callback & abortHandler)
{
    std::vector<uint8_t> Config(size);

    stream->read(Config.data(), size, abortHandler);

    SetConfiguration((const char *) Config.data(), Config.size());
}

/// <summary>
/// Gets the instance configuration data.
/// </summary>
void cuielement_t::get_config(stream_writer * stream, abort_callback & abortHandler) const
{
    const auto Config = GetConfiguration();

    stream->write_string_raw(Config.c_str(), abortHandler);
}

/// <summary>
/// Gets the colors.
/// </summary>
void cuielement_t::GetColors() noexcept
{
    cui::colours::helper Helper(pfc::guid_null);

    _Theme.SetColor(COLOR_WINDOW,     Helper.get_colour(cui::colours::colour_background));
    _Theme.SetColor(COLOR_WINDOWTEXT, Helper.get_colour(cui::colours::colour_text));
    _Theme.SetColor(COLOR_HIGHLIGHT,  Helper.get_colour(cui::colours::colour_selection_background));
    _Theme.SetColor(COLOR_HOTLIGHT,   Helper.get_colour(cui::colours::colour_active_item_frame));
}

static uie::window_factory<cuielement_t> _CUIElementFactory;

#pragma endregion

#pragma region cui_color_client_t

void cui_color_client_t::on_colour_changed(uint32_t changed_items_mask) const
{
    for (auto Iter : _Elements)
        Iter->OnColorsChanged();
}

void cui_color_client_t::on_bool_changed(uint32_t changed_items_mask) const
{
    for (auto Iter : _Elements)
        Iter->OnColorsChanged();
}

static cui::colours::client::factory<cui_color_client_t> _CUIColorClientFactory;

#pragma endregion
}
