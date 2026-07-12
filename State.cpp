
/** $VER: State.cpp (2026.07.12) P. Stuer **/

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
    _NameFormat = "%node_name%$if(%node_is_folder%,,' ('%node_item_count%')')";

    _Images.push_back({ "imageres.dll",   4 }); // Folder
    _Images.push_back({ "imageres.dll", 126 }); // Playlist
    _Images.push_back({ "imageres.dll", 125 }); // Playlist Playing

    _Object.clear();
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

    const json Object = json::parse(data, data + size, nullptr, true);

    _NameFormat = Object.value("nameFormat", _NameFormat).c_str();

    if (_Images.size() == Object["images"].size())
    {
        _Images.clear();

        for (const auto & Image : Object["images"])
        {
            _Images.push_back({ Image["filePath"], Image["iconIndex"] });
        }
    }

    _Object = Object;
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

    json::array_t Images;

    for (auto & Image : _Images)
    {
        Images.push_back(json::object_t
        ({
            { "filePath",  Image._FilePath },
            { "iconIndex", Image._IconIndex }
        }));
    };

    Object["images"] = Images;

    return Object;
}

state_t _State;
