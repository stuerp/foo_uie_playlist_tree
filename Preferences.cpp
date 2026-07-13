
/** $VER: Preferences.cpp (2026.07.13) P. Stuer **/

#include "pch.h"

#include <SDK\preferences_page.h>

#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "PlaylistUIElement.h"
#include "Tracker.h"
#include "Resources.h"
#include "Preferences.h"
#include "State.h"
#include "IconList.h"
#include "ImageList.h"
#include "Theme.h"
#include "TitleFormat.h"
#include "Log.h"

#pragma hdrstop

/// <summary>
/// Implements the preferences page for the component.
/// </summary>
class preferences_t : public CDialogImpl<preferences_t>, public preferences_page_instance
{
public:
    preferences_t(preferences_page_callback::ptr callback) : m_bMsgHandled(FALSE), _Callback(callback)
    {
        _Theme.Initialize(_DarkMode);

        icon_list_t::Register(THIS_HINSTANCE);
    }

    virtual ~preferences_t() { }

    enum
    {
        IDD = IDD_PREFERENCES
    };

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
        std::wstring Text;

        Text.resize(4096);

        {
            GetDlgItemTextW(IDC_TEXT_FORMAT, Text.data(), (int) Text.size());

            _State._NameFormat = msc::WideToUTF8(Text.c_str());
        }

        {
            if (_SelectedIndex != -1)
            {
                auto & Image = _State._Images[(size_t) _SelectedIndex];

                GetDlgItemTextW(IDC_FILE_PATH, Text.data(), (int) Text.size());

                Image._FilePath  = msc::WideToUTF8(Text.c_str());
                Image._IconIndex = (uint32_t) SendDlgItemMessageW(IDC_IMAGE_LIST, ILM_GETSELECTEDITEM);
            }
        }

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
        _State.Reset();

        InitializeControls();

        OnChanged();
    }

    #pragma endregion

    // WTL message map
    BEGIN_MSG_MAP_EX(preferences_t)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_HANDLER_EX(IDC_TEXT_FORMAT, EN_CHANGE,      OnEditChange)
        COMMAND_HANDLER_EX(IDC_FILE_PATH,   EN_CHANGE,      OnEditChange)

        COMMAND_HANDLER_EX(IDC_NODE_TYPE,  CBN_SELCHANGE,  OnSelectionChange)

        MSG_WM_NOTIFY(OnNotify);
    END_MSG_MAP()

private:
    /// <summary>
    /// Initializes the dialog.
    /// </summary>
    BOOL OnInitDialog(CWindow, LPARAM) noexcept
    {
        _DarkMode.AddDialogWithControls(*this);

        InitializeControls();

        return FALSE;
    }

    /// <summary>
    /// Initializes the controls.
    /// </summary>
    void InitializeControls()
    {
        // Initialize the Name Format edit box.
        SetDlgItemTextW(IDC_TEXT_FORMAT, msc::UTF8ToWide(_State._NameFormat).c_str());

        // Initialize the Image Type combobox.
        {
            auto w = (CComboBox) GetDlgItem(IDC_NODE_TYPE);

            w.ResetContent();

            const WCHAR * Labels[] = { L"Folder", L"Playlist", L"Playlist (Playing)" };

            for (auto Label : Labels)
                w.AddString(Label);

            _SelectedIndex = 0;

            w.SetCurSel(_SelectedIndex);

            OnImageTypeChange();
        }
    }

    /// <summary>
    /// Handles an update of the selected item of a combo box.
    /// </summary>
    void OnSelectionChange(UINT, int id, CWindow w) noexcept
    {
        if (id != IDC_NODE_TYPE)
            return;

        auto cb = (CComboBox) w;

        _SelectedIndex = cb.GetCurSel();

        OnImageTypeChange();

        OnChanged();
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditChange(UINT, int id, CWindow) noexcept
    {
        if (_IgnoreNotifications)
            return;

        // Update the icon list when the user changes the file path.
        if (id == IDC_FILE_PATH)
        {
            std::wstring Text;

            Text.resize(4096);

            GetDlgItemTextW(IDC_FILE_PATH, Text.data(), (int) Text.size());

            const auto Index = (uint32_t) SendDlgItemMessageW(IDC_IMAGE_LIST, ILM_GETSELECTEDITEM);

            UpdateIconList(msc::WideToUTF8(Text.c_str()), Index);
        }

        OnChanged();
    }

    /// <summary>
    /// Handles a click on a button.
    /// </summary>
    void OnButtonClick(UINT, int id, CWindow w) noexcept
    {
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

        {
            GetDlgItemTextW(IDC_TEXT_FORMAT, Text.data(), (int) Text.size());

            if (_State._NameFormat != msc::WideToUTF8(Text.c_str()))
                return true;
        }

        auto & Image = _State._Images[(size_t) _SelectedIndex];

        {
            GetDlgItemTextW(IDC_FILE_PATH, Text.data(), (int) Text.size());

            if (Image._FilePath != msc::WideToUTF8(Text.c_str()))
                return true;

            const auto Index = (uint32_t) SendDlgItemMessageW(IDC_IMAGE_LIST, ILM_GETSELECTEDITEM);

            if (Image._IconIndex != Index)
                return true;
        }

        return false;
    }

    /// <summary>
    /// Handles a change to the selected image type.
    /// </summary>
    void OnImageTypeChange() noexcept
    {
        _IgnoreNotifications = true;

        auto & Image = _State._Images[(size_t) _SelectedIndex];

        {
            // Set the image file path.
            SetDlgItemTextW(IDC_FILE_PATH, msc::UTF8ToWide(Image._FilePath).c_str());

            // Set the image index.
            UpdateIconList(Image._FilePath, Image._IconIndex);
        }

        _IgnoreNotifications = false;
    }

    /// <summary>
    /// Updates the icon list.
    /// </summary>
    void UpdateIconList(const std::string & filePath, uint32_t iconIndex) noexcept
    {
        pfc::string Text;

        HRESULT hResult = title_formatter_t::Evaluate(filePath, { }, Text);

        auto FilePath = (SUCCEEDED(hResult)) ? Text.c_str() : filePath;

        const auto IconSize = (uint32_t) ::GetSystemMetrics(SM_CXSMICON);

        HIMAGELIST hImageList = image_list_factory_t::Create(filePath, IconSize);

        if (hImageList == NULL)
        {
            _IgnoreNotifications = false;

            return;
        }

        HWND hIconList = GetDlgItem(IDC_IMAGE_LIST);

        ::SendMessageW(hIconList, ILM_SETIMAGELIST, 0, (LPARAM) hImageList);

        ::SendMessageW(hIconList, ILM_SELECTITEM, (WPARAM) iconIndex, 0);
    }

private:
    const preferences_page_callback::ptr _Callback;

    uielement_t * _CurrentElement;

    int _SelectedIndex;
    bool _IgnoreNotifications;

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
