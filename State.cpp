
/** $VER: State.cpp (2026.07.05) P. Stuer **/

#include "pch.h"

#include "State.h"

#pragma hdrstop

/// <summary>
/// Initializes a new instance.
/// </summary>
state_t::state_t() noexcept
{
    Reset();
}

/// <summary>
/// Resets this instance.
/// </summary>
void state_t::Reset() noexcept
{
    _NameFormat = "%node_name%$if(%is_folder%,'',' ('%count%')')";
}

/// <summary>
/// Implements the = operator.
/// </summary>
state_t & state_t::operator=(const state_t & other) noexcept
{
    return *this;
}

/// <summary>
/// Deserializes this instance from a JSON character array.
/// </summary>
void state_t::FromJSON(const char * data, size_t size) noexcept
{
    if ((data == nullptr) || (size == 0))
        return;

    json Object = json::parse(data, data + size, nullptr, true);

    _NameFormat = Object.value("nameFormat", _NameFormat);
}

/// <summary>
/// Serializes this instance to JSON string.
/// </summary>
json state_t::ToJSON() const noexcept
{
    json Object =
    {
        { "schemaVersion", _SchemaVersion },

        { "nameFormat", _NameFormat },
    };

    return Object;
}
