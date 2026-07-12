
/** $VER: Preferences.h (2026.07.12) **/

#pragma once

#define IDC_TEXT_FORMAT     1000
#define IDC_IMAGE_TYPE      1001
#define IDC_FILE_PATH       1002
#define IDC_IMAGE_LIST      1003

#define W_A00   332 // Dialog width as set by foobar2000, in dialog units
#define H_A00   288 // Dialog height as set by foobar2000, in dialog units

#define H_LBL     8 // Label

#define W_BTN    50 // Button
#define H_BTN    14 // Button

#define H_EBX    14 // Edit box
#define H_CBX    14 // Combo box

#define W_CHB    10 // Check box
#define H_CHB    10 // Check box

#define DX        7
#define DY        7

#define IX        4 // Spacing between two related controls
#define IY        3

/** Text Format **/

// Label
#define X_A11   0
#define Y_A11   0
#define W_A11   42
#define H_A11   H_LBL

// EditBox
#define X_A12   X_A11 + W_A11 + IX
#define Y_A12   Y_A11
#define W_A12   W_A00 - W_A11 - IX - IX
#define H_A12   H_EBX

/** Image **/

// Label
#define X_A13   0
#define Y_A13   Y_A12 + H_A12 + IY
#define W_A13   W_A11
#define H_A13   H_LBL

// ComboBox
#define X_A14   X_A13 + W_A13 + IX
#define Y_A14   Y_A13
#define W_A14   70
#define H_A14   H_CBX

/** File Path **/

// Label
#define X_A15   0
#define Y_A15   Y_A14 + H_A14 + IY
#define W_A15   W_A11
#define H_A15   H_LBL

// EditBox
#define X_A16   X_A15 + W_A15 + IX
#define Y_A16   Y_A15
#define W_A16   W_A00 - W_A15 - IX - IX
#define H_A16   H_EBX

/** Icon List **/

// IconList
#define X_A17   X_A16
#define Y_A17   Y_A16 + H_A16 + IY
#define W_A17   W_A16
#define H_A17   64
