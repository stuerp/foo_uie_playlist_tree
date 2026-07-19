
/** $VER: CUIColorClient.cpp (2026.07.14) P. Stuer **/

#include "pch.h"

#include "CUIElement.h"
#include "CUIColorClient.h"

#pragma hdrstop

namespace uie
{

/// <summary>
/// Called whenever a color changes.
/// </summary>
void cui_color_client_t::on_colour_changed(uint32_t changed_items_mask) const
{
    if (_Element != nullptr)
        _Element->GetColors();
}

/// <summary>
/// Called whenever a supported boolean flag changes. Support for a flag is determined using the get_supported_bools() method.
/// </summary>
void cui_color_client_t::on_bool_changed(uint32_t changed_items_mask) const
{
    if (_Element != nullptr)
        _Element->GetColors();
}

}
