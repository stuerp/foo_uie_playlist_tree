
/** $VER: State.cpp (2026.07.09) P. Stuer **/

#include "pch.h"

#include "State.h"

#pragma hdrstop

/*
// shell32.dll icons
enum Icon : int
{
    File    = 0,
    Folder  = 3
};
*/
// imageres.dll icons
enum Icon : int
{
    File    = 126,
    Folder  = 4
};

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
    _NameFormat = "%node_name%$if(%is_folder%,,' ('%count%')')";
//  _NameFormat = "%node_name%$if(%is_folder%,,' ('%count%') '%playlist_duration% %playlist_size%)";

    _Images.push_back({ "imageres.dll", Icon::Folder });
    _Images.push_back({ "imageres.dll", Icon::File });

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
