
/** $VER: State.h (2026.07.12) P. Stuer **/

#pragma once

#pragma warning(push)
#pragma warning(disable: 4868) // compiler may not enforce left-to-right evaluation order in braced initializer list

#include <nlohmann\json.hpp>

using json = nlohmann::ordered_json;

#pragma warning(pop)

enum ItemImage : int
{
    Folder          = 0,
    Playlist        = 1,
    PlaylistPlaying = 2,
};

struct image_t
{
    std::string _FilePath;
    uint32_t _IconIndex;
};

/// <summary>
/// Represents the state of the component.
/// </summary>
class state_t
{
public:
    state_t() noexcept;

    state_t & operator=(const state_t & other) noexcept;

    virtual ~state_t() noexcept { }

    void Reset() noexcept;

    void FromJSON(const char * data, size_t size) noexcept;
    json ToJSON() const noexcept;

public:
    std::string _NameFormat;

    std::vector<image_t> _Images;

    const uint32_t _SchemaVersion = 1; // v0.1.0.0

    /* Not serialized */
    json _Object;
};

extern state_t _State;
