
/** $VER: Preferences.cpp (2026.07.22) P. Stuer **/

#include "pch.h"

#include <SDK\preferences_page.h>

#include <helpers\atl-misc.h>
#include <helpers\DarkMode.h>

#include "PlaylistUIElement.h"
#include "Tracker.h"
#include "Resources.h"
#include "Preferences.h"
#include "State.h"
#include "IconList.h"
#include "ImageList.h"
#include "Theme.h"
#include "TitleFormat.h"

#pragma hdrstop

/// <summary>
/// Implements the preferences page for the component.
/// </summary>
class preferences_t : public CDialogImpl<preferences_t>, public preferences_page_instance, public ui_config_callback
{
public:
    preferences_t(preferences_page_callback::ptr callback) : m_bMsgHandled(FALSE), _Callback(callback)
    {
        _Theme.Initialize();

        icon_list_t::Register(THIS_HINSTANCE);
    }

    virtual ~preferences_t() { }

    enum
    {
        IDD = IDD_PREFERENCES
    };

    //! Called when user changes configuration of fonts.
    virtual void ui_fonts_changed() {}

    //! Called when user changes configuration of colors (also as a result of toggling dark mode). \n
    virtual void ui_colors_changed()
    {
    }

    #pragma region preferences_page_instance

    /// <summary>
    /// Returns a combination of preferences_state constants.
    /// </summary>
    virtual t_uint32 get_state() final
    {
        t_uint32 State = preferences_state::resettable | preferences_state::dark_mode_supported;

        if (HasChanged())
            State |= preferences_state::changed;

        return State;
    }

    /// <summary>
    /// Applies the changes to the preferences.
    /// </summary>
    virtual void apply() final
    {
        _State = _NewState;

        auto CurrentElement = _UIElementTracker.GetCurrent();

        if (CurrentElement != nullptr)
            CurrentElement->Refresh();

         OnChanged();
    }

    /// <summary>
    /// Resets this page's content to the default values. Does not apply any changes - lets user preview the changes before hitting "apply".
    /// </summary>
    virtual void reset() final
    {
        _NewState.Reset();

        InitializeControls();

        OnChanged();
    }

    #pragma endregion

    // WTL message map
    BEGIN_MSG_MAP_EX(preferences_t)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_HANDLER_EX(IDC_TEXT_FORMAT,         EN_CHANGE,      OnEditChange)
        COMMAND_HANDLER_EX(IDC_FILE_PATH,           EN_CHANGE,      OnEditChange)
        COMMAND_HANDLER_EX(IDC_IMAGE_SIZE,          EN_KILLFOCUS,   OnEditKillFocus)

        COMMAND_HANDLER_EX(IDC_NODE_TYPE,           CBN_SELCHANGE,  OnSelectionChange)

        COMMAND_HANDLER_EX(IDC_FILE_PATH_SELECT,    BN_CLICKED,     OnButtonClick)

        MSG_WM_NOTIFY(OnNotify);
    END_MSG_MAP()

private:
    /// <summary>
    /// Initializes the dialog.
    /// </summary>
    BOOL OnInitDialog(CWindow, LPARAM) noexcept
    {
        _DarkMode.AddDialogWithControls(*this);

        _NewState = _State;

        InitializeControls();

        return FALSE;
    }

    /// <summary>
    /// Initializes the controls.
    /// </summary>
    void InitializeControls()
    {
        auto Scope = toggle_t(_IgnoreNotifications, true);

        // Text Format
        {
            SetDlgItemTextW(IDC_TEXT_FORMAT, msc::UTF8ToWide(_NewState._TextFormat).c_str());
        }

        // Node Type 
        {
            auto w = (CComboBox) GetDlgItem(IDC_NODE_TYPE);

            w.ResetContent();

            const WCHAR * Labels[] = { L"Folder", L"Playlist", L"Playlist (Playing)" };

            for (auto Label : Labels)
                w.AddString(Label);

            _SelectedIndex = 0;

            w.SetCurSel(_SelectedIndex);
        }

        // File Path and Icon Index
        {
            auto & Image = _NewState._Images[(size_t) _SelectedIndex];

            SetDlgItemTextW(IDC_FILE_PATH, msc::UTF8ToWide(Image._FilePath).c_str());

            UpdateIconList();
        }

        // Image Size
        {
            wchar_t Text[32];

            _itow_s(std::clamp((int) _NewState._ImageSize, 16, 256), Text, _countof(Text), 10);

            SetDlgItemTextW(IDC_IMAGE_SIZE, Text);
        }
    }

    /// <summary>
    /// Handles an update of the selected item of a combo box.
    /// </summary>
    void OnSelectionChange(UINT, int id, CWindow w) noexcept
    {
        if (id == IDC_NODE_TYPE)
        {
            auto cb = (CComboBox) w;

            _SelectedIndex = cb.GetCurSel();

            {
                auto & Image = _NewState._Images[(size_t) _SelectedIndex];

                {
                    auto Scope = toggle_t(_IgnoreNotifications, true);

                    SetDlgItemTextW(IDC_FILE_PATH, msc::UTF8ToWide(Image._FilePath).c_str());
                }

                UpdateIconList();
            }
        }

        OnChanged();
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditChange(UINT, int id, CWindow wnd) noexcept
    {
        if (_IgnoreNotifications)
            return;

        if (id == IDC_FILE_PATH)
        {
            auto Edit = (CEdit) wnd;

            auto & Image = _NewState._Images[(size_t) _SelectedIndex];

            {
                std::wstring Text;

                Text.resize(MAX_PATH);

                Edit.GetWindowTextW(Text.data(), (int) Text.size());

                Image._FilePath  = msc::WideToUTF8(Text).c_str();
                Image._IconIndex = 0;
            }

            UpdateIconList();
        }

        OnChanged();
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditKillFocus(UINT, int id, CWindow wnd) noexcept
    {
        if (_IgnoreNotifications)
            return;

        if (id == IDC_IMAGE_SIZE)
        {
            auto Edit = (CEdit) wnd;

            {
                wchar_t Text[32];

                Edit.GetWindowTextW(Text, (int) _countof(Text));

                _NewState._ImageSize = (uint32_t) std::clamp(_wtoi(Text), 16, 256);

                // Convert the number back to text in case it got clamped.
                _itow_s((int) _NewState._ImageSize, Text, _countof(Text), 10);

                const auto CaretIndex = Edit.GetCaretIndex();

                {
                    auto Scope = toggle_t(_IgnoreNotifications, true);

                    Edit.SetWindowTextW(Text);
                }

                Edit.SetCaretIndex(CaretIndex);
            }

            UpdateIconList();
        }

        OnChanged();
    }

    /// <summary>
    /// Handles a click on a button.
    /// </summary>
    void OnButtonClick(UINT, int id, CWindow w) noexcept
    {
        if (id == IDC_FILE_PATH_SELECT)
        {
            std::wstring FilePath;

            FilePath.resize(MAX_PATH);

            GetDlgItemTextW(IDC_FILE_PATH, FilePath.data(), (int) FilePath.size());

            if (msc::OpenFileDialog(m_hWnd, { { L"All files (*.*)", L"*.*" }}, 0, L"dll", L"Choose the icon file...", msc::GetFullPath(FilePath).c_str(), FilePath))
                SetDlgItemTextW(IDC_FILE_PATH, FilePath.c_str());
        }

        OnChanged();
    }

    /// <summary>
    /// Handles notifications from custom controls.
    /// </summary>
    LRESULT OnNotify(int id, NMHDR * nmhd) noexcept
    {
        if (_IgnoreNotifications)
            return FALSE;

        if (nmhd->idFrom != IDC_IMAGE_LIST)
            return FALSE;

        if (nmhd->code != ILN_SELCHANGE)
            return FALSE;

        auto & Image = _NewState._Images[(size_t) _SelectedIndex];

        Image._IconIndex = (uint32_t) SendDlgItemMessageW(IDC_IMAGE_LIST, ILM_GETSELECTEDITEM);

        OnChanged();

        return TRUE;
    }

    /// <summary>
    /// Tells the host that our state has changed to enable/disable the apply button appropriately.
    /// </summary>
    void OnChanged() noexcept
    {
        _Callback->on_state_changed();
    }

    /// <summary>
    /// Returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)
    /// </summary>
    bool HasChanged() noexcept
    {
        std::wstring Text;

        Text.resize(4096);

        // Text Format
        {
            if (_NewState._TextFormat != _State._TextFormat)
                return true;
        }

        auto & Image = _NewState._Images[(size_t) _SelectedIndex];

        // File Path and Icon Index
        {
            if (Image._FilePath != _State._Images[(size_t) _SelectedIndex]._FilePath)
                return true;

            if (Image._IconIndex != _State._Images[(size_t) _SelectedIndex]._IconIndex)
                return true;
        }

        // Image Size
        {
            if (_NewState._ImageSize != _State._ImageSize)
                return true;
        }

        return false;
    }

    /// <summary>
    /// Updates the icon list.
    /// </summary>
    void UpdateIconList() noexcept
    {
        const auto & Image = _NewState._Images[(size_t) _SelectedIndex];

        pfc::string Text;

        HRESULT hResult = title_formatter_t::Evaluate(Image._FilePath, { }, Text);

        auto FilePath = (SUCCEEDED(hResult)) ? Text.c_str() : Image._FilePath;

        HIMAGELIST hImageList = image_list_factory_t::Create(FilePath, _NewState._ImageSize);

        if (hImageList == NULL)
            Log.AtWarn().Write("Failed to create image list from \"%s\": 0x%08X", FilePath.c_str(), ::GetLastError());

        HWND hIconList = GetDlgItem(IDC_IMAGE_LIST);

        ::SendMessageW(hIconList, ILM_SETIMAGELIST, 0, (LPARAM) hImageList);

        ::SendMessageW(hIconList, ILM_SELECTITEM, (WPARAM) Image._IconIndex, 0);
    }

private:
    const preferences_page_callback::ptr _Callback;

    int _SelectedIndex;
    bool _IgnoreNotifications;

    state_t _NewState;

    fb2k::CDarkModeHooks _DarkMode;
};

#pragma region PreferencesPage

/// <summary>
/// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
/// </summary>
class PreferencesPage : public preferences_page_impl<preferences_t>
{
public:
    virtual ~PreferencesPage() { }

    const char * get_name()
    {
        return STR_COMPONENT_NAME;
    }

    GUID get_guid()
    {
        return GUID_PREFERENCES;
    }

    GUID get_parent_guid()
    {
        return guid_display;
    }
};

static preferences_page_factory_t<PreferencesPage> _Factory;

#pragma endregion
