
/** $VER: Win32.cpp (2026.07.16) P. Stuer - Win32 routines **/

#include "pch.h"

#include "Win32.h"

namespace msc
{

/// <summary>
/// Converts a GUID to a wide string.
/// </summary>
std::wstring GUIDToWide(const GUID & id) noexcept
{
    OLECHAR Text[39] = { }; // {00000000-0000-0000-0000-000000000000}\0

    (void) ::StringFromGUID2(id, Text, (int) _countof(Text));

    return std::wstring(Text);
}

/// <summary>
/// Converts a wide string to a GUID.
/// </summary>
GUID WideToGUID(const std::wstring & text) noexcept
{
    GUID Id;

    HRESULT hResult = ::IIDFromString(text.c_str(), &Id);

    if (!SUCCEEDED(hResult))
        return GUID();

    return Id;
}

}
