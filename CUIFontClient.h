
/** $VER: CUIFontClient.h (2026.07.25) P. Stuer - Columns User Interface support **/

#pragma once

#include <fonts.h>

namespace uie
{
class cui_element_t;

/// <summary>
/// Receives notifications from CUI when the fonts change.
/// </summary>
class cui_font_client_t : public cui::fonts::client
{
public:
    cui_font_client_t() { }

    cui_font_client_t(const cui_font_client_t &) = delete;
    cui_font_client_t & operator=(const cui_font_client_t &) = delete;
    cui_font_client_t(cui_font_client_t &&) = delete;
    cui_font_client_t & operator=(cui_font_client_t &&) = delete;

    virtual ~cui_font_client_t() { }

    #pragma region cui::fonts::client

    const GUID & get_client_guid() const noexcept final;
    void get_name(pfc::string_base & out) const noexcept final;
    cui::fonts::font_type_t get_default_font_type() const noexcept final;
    void on_font_changed() const noexcept final;

    #pragma endregion

public:
    cui_element_t * _Element = nullptr;
};
}
