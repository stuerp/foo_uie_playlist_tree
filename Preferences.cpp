
/** $VER: Preferences.cpp (2026.07.08) P. Stuer **/

#include "pch.h"

#include <SDK/cfg_var.h>
#include <SDK/preferences_page.h>

#include <pfc/string-conv-lite.h>

#include <helpers/advconfig_impl.h>
#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "Resources.h"
#include "Preferences.h"
#include "State.h"

#pragma hdrstop

/// <summary>
/// Implements the preferences page for the component.
/// </summary>
class Preferences : public CDialogImpl<Preferences>, public preferences_page_instance
{
public:
    Preferences(preferences_page_callback::ptr callback) : m_bMsgHandled(FALSE), _Callback(callback)
    {
    }

    virtual ~Preferences()
    {
    }

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

            _State._NameFormat= msc::WideToUTF8(Text);
        }

        {
            GetDlgItemTextW(IDC_FOLDER_IMAGE_PATH, Text.data(), (int) Text.size());

            _State._FolderImageFilePath = msc::WideToUTF8(Text);
        }

        {
            GetDlgItemTextW(IDC_FOLDER_IMAGE_INDEX, Text.data(), (int) Text.size());

            _State._FolderImageIconIndex = ::_wtoi(Text.c_str());
        }

        {
            auto w = (CComboBox) GetDlgItem(IDC_RETENTION_UNIT);

//            _Configuration._RetentionUnit = (RetentionUnit) w.GetCurSel();
        }

//        _Configuration._WriteToTags = (SendDlgItemMessageW(IDC_WRITE_TO_TAGS, BM_GETCHECK) == BST_CHECKED) ? WriteToTags::Always : WriteToTags::Never;

//        _Configuration._WriteLegacyTags = (SendDlgItemMessageW(IDC_WRITE_LEGACY_TAGS, BM_GETCHECK) == BST_CHECKED);
//        _Configuration._RemoveTags = (SendDlgItemMessageW(IDC_REMOVE_TAGS, BM_GETCHECK) == BST_CHECKED);

        {
//          GetDlgItemTextW(IDC_TAGS_TO_REMOVE, Text, _countof(Text));

//            _Configuration._TagsToRemove = pfc::utf8FromWide(Text);
        }

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
    BEGIN_MSG_MAP_EX(Preferences)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_HANDLER_EX(IDC_TEXT_FORMAT,         EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_FOLDER_IMAGE_PATH,   EN_CHANGE, OnEditChange)
        COMMAND_HANDLER_EX(IDC_FOLDER_IMAGE_INDEX,  EN_CHANGE, OnEditChange)

        COMMAND_HANDLER_EX(IDC_WRITE_TO_TAGS, BN_CLICKED, OnButtonClicked)
        COMMAND_HANDLER_EX(IDC_WRITE_LEGACY_TAGS, BN_CLICKED, OnButtonClicked)
        COMMAND_HANDLER_EX(IDC_REMOVE_TAGS, BN_CLICKED, OnButtonClicked)

        COMMAND_HANDLER_EX(IDC_TAGS_TO_REMOVE, EN_CHANGE, OnEditChange)
    END_MSG_MAP()

private:
    /// <summary>
    /// Initializes the dialog.
    /// </summary>
    BOOL OnInitDialog(CWindow, LPARAM) noexcept
    {
        _DarkModeHooks.AddDialogWithControls(*this);

        InitializeControls();

        return FALSE;
    }

    /// <summary>
    /// Initializes the controls.
    /// </summary>
    void InitializeControls()
    {
        SetDlgItemTextW(IDC_TEXT_FORMAT,        msc::UTF8ToWide(_State._NameFormat).c_str());

        SetDlgItemTextW(IDC_FOLDER_IMAGE_PATH,  msc::UTF8ToWide(_State._FolderImageFilePath).c_str());
        SetDlgItemTextW(IDC_FOLDER_IMAGE_INDEX, msc::UTF8ToWide(pfc::format_int(_State._FolderImageIconIndex).c_str()).c_str());


//      ((CCheckBox) GetDlgItem(IDC_WRITE_TO_TAGS)).SetCheck((_Configuration._WriteToTags == WriteToTags::Always) ? BST_CHECKED : BST_UNCHECKED);
    }

    /// <summary>
    /// Handles an update of the selected item of a combo box.
    /// </summary>
    void OnSelectionChanged(UINT, int, CWindow) noexcept
    {
        OnChanged();
    }

    /// <summary>
    /// Handles a textbox change.
    /// </summary>
    void OnEditChange(UINT, int, CWindow) noexcept
    {
        OnChanged();
    }

    /// <summary>
    /// Handles a click on a button.
    /// </summary>
    void OnButtonClicked(UINT, int id, CWindow w) noexcept
    {
        if (id == IDC_REMOVE_TAGS)
            GetDlgItem(IDC_TAGS_TO_REMOVE).EnableWindow(((CCheckBox) w).IsChecked());

        OnChanged();
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

        GetDlgItemTextW(IDC_TEXT_FORMAT, Text.data(), (int) Text.size());

        if (_State._NameFormat != msc::WideToUTF8(Text))
            return true;

        GetDlgItemTextW(IDC_FOLDER_IMAGE_PATH, Text.data(), (int) Text.size());

        if (_State._NameFormat != msc::WideToUTF8(Text))
            return true;

        GetDlgItemTextW(IDC_FOLDER_IMAGE_INDEX, Text.data(), (int) Text.size());

        if (_State._FolderImageFilePath != msc::WideToUTF8(Text))
            return true;

        if (_State._FolderImageIconIndex != (int32_t) ::_wtoi(Text.c_str()))
            return true;
/*
        if (_Configuration._RetentionUnit != (RetentionUnit) ((CComboBox) GetDlgItem(IDC_RETENTION_UNIT)).GetCurSel())
            return true;

        if (SendDlgItemMessageW(IDC_WRITE_TO_TAGS, BM_GETCHECK) != ((_Configuration._WriteToTags == WriteToTags::Always) ? BST_CHECKED : BST_UNCHECKED))
            return true;

        if (SendDlgItemMessageW(IDC_WRITE_LEGACY_TAGS, BM_GETCHECK) != (_Configuration._WriteLegacyTags ? BST_CHECKED : BST_UNCHECKED))
            return true;

        if (SendDlgItemMessageW(IDC_REMOVE_TAGS, BM_GETCHECK) != (_Configuration._RemoveTags ? BST_CHECKED : BST_UNCHECKED))
            return true;

        GetDlgItemTextW(IDC_TAGS_TO_REMOVE, Text, _countof(Text));

        if (_Configuration._TagsToRemove != pfc::utf8FromWide(Text))
            return true;
*/
        return false;
    }

private:
    const preferences_page_callback::ptr _Callback;

    fb2k::CDarkModeHooks _DarkModeHooks;
};

#pragma region PreferencesPage

/// <summary>
/// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
/// </summary>
class PreferencesPage : public preferences_page_impl<Preferences>
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
