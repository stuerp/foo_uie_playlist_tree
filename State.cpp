
/** $VER: State.cpp (2026.07.24) P. Stuer **/

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
    _TextFormat    = "%node_name%$if(%node_is_folder%,,' ('%node_item_count%')')";
    _ToolTipFormat = "$if(%playlist_size_natural%,%playlist_size_natural%\n$if2(%playlist_duration_natural%,', N/A'),'N/A')";
    _ImageSize     = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

    _Images.clear();

    _Images.push_back({ "imageres.dll",   4 }); // Folder
    _Images.push_back({ "imageres.dll", 126 }); // Playlist
    _Images.push_back({ "imageres.dll", 125 }); // Playlist (Playing)
    _Images.push_back({ "shell32.dll",   47 }); // Playlist (Locked)

    _Object.clear();
}

/// <summary>
/// Implements the = operator.
/// </summary>
state_t & state_t::operator=(const state_t & other) noexcept
{
    _TextFormat    = other._TextFormat;
    _ToolTipFormat = other._ToolTipFormat;
    _ImageSize     = other._ImageSize;

    _Images        = other._Images;

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

    _TextFormat    = Object.value("nameFormat", _TextFormat).c_str();
    _ToolTipFormat = Object.value("toolTip",    _ToolTipFormat).c_str();
    _ImageSize     = Object.value("imageSize",  _ImageSize);

    size_t Index = 0;

    for (const auto & Image : Object["images"])
    {
        if (Index < _Images.size())
        {
            _Images[Index] =
            {
                Image["filePath"],
                Image["iconIndex"]
            };
        }
        else
        {
            _Images.push_back
            ({
                Image["filePath"],
                Image["iconIndex"]
            });
        }

        ++Index;
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
        { "toolTip", _ToolTipFormat },
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
