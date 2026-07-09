
/** $VER: State.cpp (2026.07.08) P. Stuer **/

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

    _FolderImageFilePath = "imageres.dll";
    _FolderImageIconIndex = Icon::Folder;

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

    _NameFormat = Object.value("nameFormat", _NameFormat);

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
        { "folder", json::object_t
            ({
                { "filePath", _FolderImageFilePath },
                { "iconIndex", _FolderImageIconIndex },
            })
        },
        { "playlist", json::object_t
            ({
                { "filePath", _PlaylistImageFilePath },
                { "iconIndex", _PlaylistImageIconIndex },
            })
        },
    };

    return Object;
}

state_t _State;
