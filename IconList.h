
/** $VER: IconList.h (2026.07.09) **/

#pragma once

#define WC_ICON_LIST L"icon_list_t"

namespace icon_list_t
{
    bool Register(HINSTANCE hInstance) noexcept;
}

#define ILM_ADDITEM         WM_USER + 1000
#define ILM_SELECTITEM      WM_USER + 1001
#define ILM_SETIMAGELIST    WM_USER + 1002
