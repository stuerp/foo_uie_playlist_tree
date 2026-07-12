
/** $VER: TitleFormat.cpp (2026.07.06) P. Stuer **/

#include "pch.h"

#include "TitleFormat.h"

#pragma hdrstop

/// <summary>
/// Evaluates a foobar2000 Title Format script.
/// </summary>
HRESULT title_formatter_t::Evaluate(const std::string & script, const GUID id, pfc::string & result) noexcept
{
    service_ptr_t<titleformat_object> tfo;

    static_api_ptr_t<titleformat_compiler>()->compile_safe_ex(tfo, script.c_str());

    custom_titleformat_hook_t Hook(id);

    tfo->run(&Hook, result, nullptr);

    return S_OK;
}

/// <summary>
/// 
/// </summary>
bool custom_titleformat_hook_t::process_field(titleformat_text_out * out, const char * name, t_size size, bool & isFound)
{
    const size_t Index = _PlaylistManager->find_playlist_by_guid(_Id);

    const bool IsFolder = (Index == ~0llu);

    /** Component specific variables **/

    if (::_stricmp(name, "node_name") == 0)
    {
        pfc::string Text;

        if (IsFolder)
        {
            std::string Name;

            _FolderManager->GetFolderName(_Id, Name);

            Text = Name.c_str();
        }
        else
            _PlaylistManager->playlist_get_name(Index, Text);

        out->write(titleformat_inputtypes::unknown, Text);

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "node_item_count") == 0)
    {
        const size_t ItemCount = _PlaylistManager->playlist_get_item_count(Index);

        out->write(titleformat_inputtypes::unknown, std::format("{}", ItemCount).c_str());

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "node_is_folder") == 0)
    {
        if (!IsFolder)
            return false;

        out->write_int(titleformat_inputtypes::unknown, 1);

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "playlist_duration") == 0)
    {
        class callback_t : public playlist_manager::enum_items_callback
        {
        public:
            virtual ~callback_t() { }

            bool on_item(t_size itemIndex, const metadb_handle_ptr & item, bool isSelected) override
            {
                if (item.is_valid())
                {
                    const double Length = item->get_length(); // in seconds

                    // Ignore unknown lengths (-1.0 or similar)
                    if (Length > 0.)
                        Duration += Length;
                }

                return true; // Continue enumerating
            }

        public:
            double Duration = 0.; // in seconds
        };

        static_api_ptr_t<playlist_manager> pm;

        callback_t Callback;

        pm->playlist_enum_items(Index, Callback, bit_array_true());

        out->write(titleformat_inputtypes::unknown, std::format("{}", Callback.Duration).c_str());

//      auto NaturalDuration = pfc::format_time(Callback.Duration);

        isFound = true;

        return true;
    }

    if (::_stricmp(name, "playlist_size") == 0)
    {
        class callback_t : public playlist_manager::enum_items_callback
        {
        public:
            virtual ~callback_t() { }

            bool on_item(t_size itemIndex, const metadb_handle_ptr & item, bool isSelected) override
            {
                if (item.is_valid())
                    Size += item->get_filesize();

                return true; // Continue enumerating
            }

        public:
            t_filesize Size = 0; // in bytes
        };

        static_api_ptr_t<playlist_manager> pm;

        callback_t Callback;

        pm->playlist_enum_items(Index, Callback, bit_array_true());

        out->write(titleformat_inputtypes::unknown, std::format("{}", Callback.Size).c_str());

//      auto NaturalSize = pfc::format_file_size_short(Callback.Size);

        isFound = true;

        return true;
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
const std::string custom_titleformat_hook_t::ExpandEnvironmentStrings(const std::string & src) noexcept
{
    const DWORD Size = ::ExpandEnvironmentStringsA(src.c_str(), nullptr, 0) + 1;

    std::string Dst;

    Dst.resize(Size);

    ::ExpandEnvironmentStringsA(src.c_str(), (LPSTR) Dst.data(), (DWORD) Dst.size());

    return Dst;
}
