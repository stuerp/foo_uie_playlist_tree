
/** $VER: Preferences.h (2026.07.08) **/

#pragma once

#define IDC_TEXT_FORMAT                     1000
#define IDC_FOLDER_IMAGE_PATH                1002

#define IDC_FOLDER_IMAGE_INDEX                 1004
#define IDC_RETENTION_UNIT                  1006

#define IDC_WRITE_TO_TAGS                   1050
#define IDC_WRITE_LEGACY_TAGS               1051
#define IDC_REMOVE_TAGS                     1052

#define IDC_TAGS_TO_REMOVE                  1053

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

#pragma region Pin To

// Label
#define X_A11   0
#define Y_A11   0
#define W_A11   85
#define H_A11   H_LBL

// EditBox
#define X_A12   X_A11 + W_A11 + IX
#define Y_A12   Y_A11
#define W_A12   W_A00 - W_A11 - IX
#define H_A12   H_EBX

#pragma endregion

#pragma region Threshold Format

// Label
#define X_A13   0
#define Y_A13   Y_A12 + H_A12 + IY
#define W_A13   85
#define H_A13   H_LBL

// EditBox
#define X_A14   X_A13 + W_A13 + IX
#define Y_A14   Y_A13
#define W_A14   W_A00 - W_A13 - IX
#define H_A14   H_EBX

#pragma endregion

#pragma region Retention Period

// Label
#define X_A20   0
#define Y_A20   Y_A14 + H_A14 + IY
#define W_A20   85
#define H_A20   H_LBL

// EditBox : Value
#define X_A21   X_A20 + W_A20 + IX
#define Y_A21   Y_A20
#define W_A21   30
#define H_A21   H_EBX

// ComboBox: Unit
#define X_A22   X_A21 + W_A21 + IX
#define Y_A22   Y_A20
#define W_A22   44
#define H_A22   H_CBX

#pragma endregion

// Checkbox: Write to tags
#define X_A50   85 + IX
#define Y_A50   Y_A22 + H_A22 + IY
#define W_A50   W_A00 - X_A50
#define H_A50   H_LBL

// Checkbox: Write legacy tags
#define X_A51   85 + IX
#define Y_A51   Y_A50 + H_A50 + IY
#define W_A51   W_A00 - X_A51
#define H_A51   H_LBL

// Checkbox: Remove tags
#define X_A52   85 + IX
#define Y_A52   Y_A51 + H_A51 + IY
#define W_A52   W_A00 - X_A52
#define H_A52   H_LBL

#pragma region Tags to remove

// Label
#define X_A30   0
#define Y_A30   Y_A52 + H_A52 + IY
#define W_A30   85
#define H_A30   H_LBL

// EditBox
#define X_A31   X_A30 + W_A30 + IX
#define Y_A31   Y_A30
#define W_A31   W_A00 - W_A30 - IX
#define H_A31   H_CBX

#pragma endregion
