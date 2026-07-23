
/** $VER: State.cpp (2026.07.23) P. Stuer **/

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
    _TextFormat = "%node_name%$if(%node_is_folder%,,' ('%node_item_count%')')";
    _ToolTip    = "$if(%playlist_size_natural%,%playlist_size_natural%\n$if2(%playlist_duration_natural%,', N/A'),'N/A')";

    _ImageSize = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

    _Images.clear();

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
    _TextFormat = other._TextFormat;
    _ToolTip    = other._ToolTip;
    _ImageSize  = other._ImageSize;
    _Images     = other._Images;

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

    _TextFormat = Object.value("nameFormat", _TextFormat).c_str();
    _ToolTip    = Object.value("toolTip", _ToolTip).c_str();
    _ImageSize  = Object.value("imageSize", _ImageSize);

    if (_Images.size() == Object["images"].size())
    {
        _Images.clear();

        for (const auto & Image : Object["images"])
        {
            _Images.push_back(
            {
                Image["filePath"],
                Image["iconIndex"]
            });
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

        { "nameFormat", _TextFormat },
        { "toolTip", _ToolTip },
        { "imageSize", _ImageSize },
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
