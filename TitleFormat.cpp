
/** $VER: TitleFormat.cpp (2026.07.05) P. Stuer **/

#include "pch.h"

#include "TitleFormat.h"

#pragma hdrstop

/// <summary>
/// Evaluates a foobar2000 Title Format script.
/// </summary>
HRESULT title_formatter_t::Evaluate(_In_ const std::string & script, _In_ size_t playlistIndex, _Out_ pfc::string & result) noexcept
{
    service_ptr_t<titleformat_object> tfo;

    static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(tfo, script.c_str());

    custom_titleformat_hook_t Hook(playlistIndex);

    tfo->run(&Hook, result, nullptr);

    return S_OK;
}

/// <summary>
/// 
/// </summary>
bool custom_titleformat_hook_t::process_field(titleformat_text_out * out, const char * name, t_size size, bool & isFound)
{
    /** Component specific variables **/

    if (::_stricmp(name, "node_name") == 0)
    {
        pfc::string Text;

        _PlaylistManager->playlist_get_name(_PlaylistIndex, Text);

        out->write(titleformat_inputtypes::unknown, Text);

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "count") == 0)
    {
        const size_t ItemCount = _PlaylistManager->playlist_get_item_count(_PlaylistIndex);

        pfc::string Text = pfc::format_int((int64_t) ItemCount).c_str();

        out->write(titleformat_inputtypes::unknown, Text);

        isFound = true;

        return true;

    }

    if (::_stricmp(name, "is_folder") == 0)
    {
        bool IsFolder = false;

        if (IsFolder)
        {
            out->write_int(titleformat_inputtypes::unknown, 1);

            isFound = true;

            return true;
        }

        return false;
    }

    /** Common foobar2000 variables **/

    if (::_stricmp(name, "fb2k_path") == 0)
    {
        pfc::string Path;

        ::uGetModuleFileName(NULL, Path);

        Path = pfc::string_directory(Path);

        out->write(titleformat_inputtypes::unknown, Path);

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "fb2k_component_path") == 0)
    {
        pfc::string Path;

        ::uGetModuleFileName(core_api::get_my_instance(), Path);

        Path = pfc::string_directory(Path);

        out->write(titleformat_inputtypes::unknown, Path);

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "fb2k_profile_path") == 0)
    {
        pfc::string Path = core_api::get_profile_path();

        Path = foobar2000_io::filesystem::g_get_native_path(Path);

        out->write(titleformat_inputtypes::unknown, Path);

        isFound = true;

        return true;
    }

    /** Windows environment variables **/

    {
        std::string Name = "%"; Name += name; Name += "%";

        pfc::string Path = ExpandEnvironmentStrings(Name.c_str()).c_str();

        out->write(titleformat_inputtypes::unknown, Path);

        isFound = true;

        return true;
    }
}

/// <summary>
/// Expands the environment variables in the specified string.
/// </summary>
const std::string custom_titleformat_hook_t::ExpandEnvironmentStrings(_In_ const std::string & src) noexcept
{
    const DWORD Size = ::ExpandEnvironmentStringsA(src.c_str(), nullptr, 0) + 1;

    std::string Dst;

    Dst.resize(Size);

    ::ExpandEnvironmentStringsA(src.c_str(), (LPSTR) Dst.data(), (DWORD) Dst.size());

    return Dst;
}
