
/** $VER: TitleFormat.h (2026.07.06) P. Stuer - Implements a custom hook to expand title formatting **/

#pragma once

#include <SDKDDKVer.h>

#define NOMINMAX

#include <Windows.h>

#undef ExpandEnvironmentStrings

#include <string>

#include <SDK\foobar2000-lite.h>
#include <SDK\titleformat.h>

#include "PlaylistsTreeView.h"

/// <summary>
/// Implements a title formatter.
/// </summary>
class title_formatter_t
{
private:
    title_formatter_t();

public:
    static HRESULT Evaluate(_In_ const std::string & script, _In_ const GUID & id, _In_ playlists_tree_view_t & treeView, _Out_ pfc::string & result) noexcept;
};

/// <summary>
/// Implements a title format hook.
/// </summary>
class custom_titleformat_hook_t : public titleformat_hook
{
public:
    custom_titleformat_hook_t(_In_ const GUID & id, _In_ playlists_tree_view_t & treeView) : _Id(id), _TreeView(treeView) { }

    virtual ~custom_titleformat_hook_t() noexcept { }

    bool process_field(titleformat_text_out * out, const char * name, t_size size, bool & isFound) override final;

    bool process_function(titleformat_text_out * out, const char * name, t_size nameSize, titleformat_hook_function_params * parameters, bool & isFound) override final
    {
        return false;
    }

private:
    static const std::string ExpandEnvironmentStrings(_In_ const std::string & src) noexcept;

private:
    static_api_ptr_t<playlist_manager_v5> _PlaylistManager;

    const GUID & _Id;
    playlists_tree_view_t & _TreeView;
};
