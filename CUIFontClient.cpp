
/** $VER: CUIFontClient.cpp (2026.07.25) P. Stuer **/

#include "pch.h"

#include "CUIElement.h"
#include "CUIFontClient.h"

#pragma hdrstop

namespace uie
{

const GUID & cui_font_client_t::get_client_guid() const noexcept
{
    static const GUID Id = GUID_UI_ELEMENT;

    return Id;
}

void cui_font_client_t::get_name(pfc::string_base & out) const noexcept
{
    out = STR_COMPONENT_NAME;
}

cui::fonts::font_type_t cui_font_client_t::get_default_font_type() const noexcept
{
    return cui::fonts::font_type_t::font_type_items;
}

void cui_font_client_t::on_font_changed() const noexcept
{
    if (_Element != nullptr)
        _Element->OnFontsChanged();
}

}
