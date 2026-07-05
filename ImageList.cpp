
/** $VER: ImageList.cpp (2026.07.05) P. Stuer **/

#include "pch.h"

#include "ImageList.h"

#pragma hdrstop

/// <summary>
/// Creates an image list from the icons in the specified file.
/// </summary>
HIMAGELIST image_list_factory_t::Create(_In_ const std::wstring & fileName, _In_ uint32_t iconSize, _In_ uint32_t maxIcons) noexcept
{
    #pragma warning(disable: 6388) // 'THIS_HINSTANCE' might not be '0': this does not adhere to the specification for the function 'ExtractIconW'.

    HIMAGELIST hImageList = NULL;

    const auto hModule = ::LoadLibraryExW(fileName.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);

    if (hModule != NULL)
    {
        if (maxIcons == ~0)
            maxIcons = GetIconCount(hModule);

        hImageList = ::ImageList_Create((int) iconSize, (int) iconSize, ILC_COLOR32 | ILC_MASK, (int) maxIcons, 0);

        if (hImageList != NULL)
        {
            for (uint32_t i = 0; i < maxIcons; ++i)
            {
                HICON hIcon = ::ExtractIconW(THIS_HINSTANCE, fileName.c_str(), (UINT) i);

                if (hIcon != NULL)
                {
                    const int Index = ImageList_AddIcon(hImageList, hIcon);

                    if (Index == -1)
                        break;

                    ::DestroyIcon(hIcon);
                }
                else
                    break; // No more icons available
            }
        }

        ::FreeLibrary(hModule);
    }

    return hImageList;
}

/// <summary>
/// Gets the number of icons in a module.
/// </summary>
uint32_t image_list_factory_t::GetIconCount(_In_ HMODULE hModule) noexcept
{
    #pragma warning(disable: 5039) // 'EnumResourceTypesW': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
    #pragma warning(disable: 6387) // 'type' could be '0':  this does not adhere to the specification for the function 'EnumResourceNamesW'.

    struct context_t
    {
        HMODULE hModule;
        uint32_t IconCount;
    } Context =
    {
        hModule
    };

    ::EnumResourceTypesW(hModule, (ENUMRESTYPEPROCW) [](HMODULE hModule, LPWSTR type, LONG_PTR lParam) -> BOOL
    {
        if (type != RT_GROUP_ICON)
            return TRUE; // Continue enumeration

        auto Context = (context_t *) lParam;

        ::EnumResourceNamesW(Context->hModule, type, (ENUMRESNAMEPROCW) [](HMODULE hModule, LPCWSTR type, LPWSTR name, LONG_PTR lParam) -> BOOL
        {
            auto Context = (context_t *) lParam;

            ++Context->IconCount;

            return TRUE;
        }, lParam);

        return FALSE;
    }, (LONG_PTR) &Context);

    return Context.IconCount;
}
