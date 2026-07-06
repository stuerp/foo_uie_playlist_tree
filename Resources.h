
/** $VER: Resources.h (2026.07.06) P. Stuer **/

#pragma once

#define TOSTRING_IMPL(x) #x
#define TOSTRING(x) TOSTRING_IMPL(x)

#define NUM_FILE_MAJOR          0
#define NUM_FILE_MINOR          1
#define NUM_FILE_PATCH          0
#define NUM_FILE_PRERELEASE     0

#define NUM_PRODUCT_MAJOR       0
#define NUM_PRODUCT_MINOR       1
#define NUM_PRODUCT_PATCH       0
#define NUM_PRODUCT_PRERELEASE  0

#define STR_PRERELEASE          "-alpha1"

/** Component specific **/

#define STR_COMPONENT_NAME          "Playlist Tree"
#define STR_COMPONENT_VERSION       TOSTRING(NUM_FILE_MAJOR) "." TOSTRING(NUM_FILE_MINOR) "." TOSTRING(NUM_FILE_PATCH) "." TOSTRING(NUM_FILE_PRERELEASE) STR_PRERELEASE
#define STR_COMPONENT_BASENAME      "foo_uie_playlist_tree"
#define STR_COMPONENT_FILENAME      STR_COMPONENT_BASENAME ".dll"
#define STR_COMPONENT_COMPANY_NAME  ""
#define STR_COMPONENT_COPYRIGHT     "Copyright (c) 2026 P. Stuer. All rights reserved."
#define STR_COMPONENT_COMMENTS      ""
#define STR_COMPONENT_DESCRIPTION   "A user interface element to organize playlists using a treeview"
#define STR_COMPONENT_COMMENT       ""
#define STR_COMPONENT_URL           "https://github.com/stuerp/" STR_COMPONENT_BASENAME

/** Generic **/

#define STR_COMPANY_NAME            TEXT(STR_COMPONENT_COMPANY_NAME)
#define STR_INTERNAL_NAME           TEXT(STR_COMPONENT_NAME)
#define STR_COMMENTS                TEXT(STR_COMPONENT_COMMENTS)
#define STR_COPYRIGHT               TEXT(STR_COMPONENT_COPYRIGHT)

#define STR_FILE_NAME               TEXT(STR_COMPONENT_FILENAME)
#define STR_FILE_VERSION            TEXT(STR_COMPONENT_VERSION)
#define STR_FILE_DESCRIPTION        TEXT(STR_COMPONENT_DESCRIPTION)

#define STR_PRODUCT_NAME            STR_INTERNAL_NAME
#define STR_PRODUCT_VERSION         TEXT(TOSTRING(NUM_PRODUCT_MAJOR)) TEXT(".") TEXT(TOSTRING(NUM_PRODUCT_MINOR)) TEXT(".") TEXT(TOSTRING(NUM_PRODUCT_PATCH)) TEXT(".") TEXT(TOSTRING(NUM_PRODUCT_PRERELEASE)) TEXT(STR_PRERELEASE)

#define STR_ABOUT_NAME              STR_INTERNAL_NAME
#define STR_ABOUT_WEB               TEXT(STR_COMPONENT_URL)

#define STR_ABOUT_EMAIL             TEXT("mailto:peter.stuer@outlook.com")

/** Window **/

#define GUID_UI_ELEMENT             { 0x91b12d45, 0xa6e4, 0x462b, { 0xaa, 0xd8, 0x04, 0x67, 0xab, 0x48, 0x1a, 0xa2 } }
#define GUID_PREFERENCES            { 0xd4ea153a, 0x8c25, 0x4ef3, { 0xaf, 0xc8, 0xf3, 0x1b, 0x5f, 0x48, 0x1b, 0xde } }
#define STR_WINDOW_CLASS_NAME       STR_COMPONENT_BASENAME "_{54B8006E-CFE2-4AE1-BCDD-DFD0DF91D2BB}"

/** Controls **/

#define IDC_TREEVIEW                           1

/** Configuration **/

#define IDD_PREFERENCES                      101

#define IDC_NAME                            1000

#define IDC_USER_DATA_FOLDER_PATH           1002
#define IDC_USER_DATA_FOLDER_PATH_SELECT    1004

#define IDC_FILE_PATH                       1006
#define IDC_FILE_PATH_SELECT                1008
#define IDC_FILE_PATH_EDIT                  1010

#define IDC_BROWSER_FLAGS                   1012

#define IDC_WINDOW_SIZE                     1020
#define IDC_WINDOW_SIZE_UNIT                1022

#define IDC_REACTION_ALIGNMENT              1030
#define IDC_WINDOW_OFFSET                   1032

#define IDC_CLEAR_BROWSING_DATA             1040
#define IDC_IN_PRIVATE_MODE                 1042
#define IDC_SCROLLBAR_STYLE                 1044

#define IDC_READ_FILES                      2000
#define IDC_READ_DIRECTORIES                2002
#define IDC_EXECUTE_SHELL_OPERATIONS        2004

#define IDC_WARNING                         9999

/** Resources **/

#define IDM_CONTEXT_MENU                       1

#define IDM_NEW_FOLDER                         2
#define IDM_NEW_PLAYLIST                       3

#define IDM_RENAME                             4
#define IDM_REMOVE                             5

#define IDM_CONTENTS                           6

#define IDM_SORT                               7
