
/** $VER: TitleFormat.cpp (2026.07.22) P. Stuer **/

#include "pch.h"

#include "TitleFormat.h"

#pragma hdrstop

template<typename... Pairs> auto dispatcher_t(Pairs &&... pairs)
{
    return std::unordered_map<std::string_view, std::function<bool()>>{ { std::forward<Pairs>(pairs)... } };
}

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
/// Returns the value of a Title Format field.
/// </summary>
bool custom_titleformat_hook_t::process_field(titleformat_text_out * out, const char * name, t_size size, bool & isFound)
{
    const size_t Index = _PlaylistManager->find_playlist_by_guid(_Id);

    const bool IsFolder = (Index == SIZE_MAX);

    const auto Dispatcher = dispatcher_t
    (
        /** Component specific variables **/

        std::pair{ "node_name", [&]() -> bool
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
        }},

        std::pair{ "node_item_count", [&]() -> bool
        {
            const size_t ItemCount = _PlaylistManager->playlist_get_item_count(Index);

            std::locale Locale(""); // User default Windows locale.

            const auto Text = std::format(Locale, "{:L}", ItemCount);

            out->write(titleformat_inputtypes::unknown, Text.c_str());

            isFound = true;

            return true;
        }},

        std::pair{ "node_is_folder", [&]() -> bool
        {
            if (!IsFolder)
                return false;

            out->write_int(titleformat_inputtypes::unknown, 1);

            isFound = true;

            return true;
        }},

        std::pair{ "playlist_duration", [&]() -> bool
        {
            auto Duration = GetPlaylistDuration(Index);

            std::locale Locale(""); // User default Windows locale.

            const auto Text = std::format(Locale, "{:L}", Duration);

            out->write(titleformat_inputtypes::unknown, Text.c_str());

            isFound = true;

            return true;
        }},

        std::pair{ "playlist_duration_natural", [&]() -> bool
        {
            auto Duration = GetPlaylistDuration(Index);

            if (Duration > 0.)
            {
                constexpr double SECS_PER_WEEK  = 604'800.;
                constexpr double SECS_PER_DAY   = 86'400.;
                constexpr double SECS_PER_HOUR  = 3'600.;
                constexpr double SECS_PER_MIN   = 60.;

                std::ostringstream oss;

                const auto Weeks = (size_t) (Duration / SECS_PER_WEEK); Duration = std::fmod(Duration, SECS_PER_WEEK);

                if (Weeks > 0)
                    oss << Weeks << 'w';

                if (!oss.str().empty())
                    oss << " ";

                const auto Days = (size_t) (Duration / SECS_PER_DAY);  Duration = std::fmod(Duration, SECS_PER_DAY);

                if (Days > 0)
                    oss << Days << 'd';

                if (!oss.str().empty())
                    oss << " ";

                if (Duration > 0.)
                {
                    const auto Hours   = (size_t) (Duration / SECS_PER_HOUR); Duration = std::fmod(Duration, SECS_PER_HOUR);
                    const auto Minutes = (size_t) (Duration / SECS_PER_MIN);  Duration = std::fmod(Duration, SECS_PER_MIN);
                    const auto Seconds = (size_t) (Duration);                 Duration -= Seconds;

                    oss << std::setfill('0') << std::setw(2) << Hours << ':'
                        << std::setfill('0') << std::setw(2) << Minutes << ':'
                        << std::setfill('0') << std::setw(2) << Seconds << '.'
                        << std::setfill('0') << std::setw(3) << (int) (Duration * 1'000.);
                }

                out->write(titleformat_inputtypes::unknown, oss.str().c_str());
            }
            else
                out->write(titleformat_inputtypes::unknown, "N/A");

            isFound = true;

            return true;
        }},

        std::pair{ "playlist_size", [&]() -> bool
        {
            auto Size = GetPlaylistSize(Index);

            std::string Text;

            if (Size != SIZE_MAX)
            {
                std::locale Locale(""); // User default Windows locale.

                Text = std::format(Locale, "{:L}", Size);
            }
            else
                Text = "N/A";

            out->write(titleformat_inputtypes::unknown, Text.c_str());

            isFound = true;

            return true;
        }},

        std::pair{ "playlist_size_natural", [&]() -> bool
        {
            auto Size = GetPlaylistSize(Index);

            std::string Text;

            if ((long long) Size >= 0)
            {
                std::locale Locale(""); // User default Windows locale.

                if (Size >= 1'099'511'627'776)
                    Text = std::format(Locale, "{:.3Lf} TB", (double) Size / 1'099'511'627'776.);
                else
                if (Size >= 1'073'741'824)
                    Text = std::format(Locale, "{:.3Lf} GB", (double) Size / 1'073'741'824.);
                else
                if (Size >= 1'048'576)
                    Text = std::format(Locale, "{:.3Lf} MB", (double) Size / 1'048'576.);

                else
                if (Size >= 1'024)
                    Text = std::format(Locale, "{:.3Lf} KB", (double) Size / 1'024.);
                else
                    Text = std::format(Locale, "{:L} bytes", Size);
            }
            else
                Text = "N/A";

            out->write(titleformat_inputtypes::unknown, Text.c_str());

            isFound = true;

            return true;
        }},

        /** Common foobar2000 variables **/

        std::pair{ "fb2k_path", [&]() -> bool
        {
            pfc::string Path;

            ::uGetModuleFileName(NULL, Path);

            Path = pfc::string_directory(Path);

            out->write(titleformat_inputtypes::unknown, Path);

            isFound = true;

            return true;
        }},

        std::pair{ "fb2k_component_path", [&]() -> bool
        {
            pfc::string Path;

            ::uGetModuleFileName(core_api::get_my_instance(), Path);

            Path = pfc::string_directory(Path);

            out->write(titleformat_inputtypes::unknown, Path);

            isFound = true;

            return true;
        }},

        std::pair{ "fb2k_profile_path", [&]() -> bool
        {
            pfc::string Path = core_api::get_profile_path();

            Path = foobar2000_io::filesystem::g_get_native_path(Path);

            out->write(titleformat_inputtypes::unknown, Path);

            isFound = true;

            return true;
        }}
    );

    if (auto it = Dispatcher.find(name); it != Dispatcher.end())
        return it->second();

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

/// <summary>
/// Gets tje duration of a playlist (in seconds).
/// </summary>
double custom_titleformat_hook_t::GetPlaylistDuration(size_t index) const noexcept
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

    callback_t Callback;

    _PlaylistManager->playlist_enum_items(index, Callback, bit_array_true());

    return Callback.Duration;
}


/// <summary>
/// Gets the size of a playlist (in bytes).
/// </summary>
t_filesize custom_titleformat_hook_t::GetPlaylistSize(size_t index) const noexcept
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

    callback_t Callback;

    _PlaylistManager->playlist_enum_items(index, Callback, bit_array_true());

    return Callback.Size;
}
