
/** $VER: StringEnumerator.h (2026.08.14) P. Stuer - Implements an IEnumString enumerator for AutoComplete **/

#pragma once

#include <Windows.h>

#include <string>
#include <utility>
#include <vector>

/// <summary>
/// Implements an IEnumString enumerator for AutoComplete.
/// </summary>
class string_enumerator_t : public IEnumString
{
public:
    string_enumerator_t() noexcept { }

    virtual ~string_enumerator_t() noexcept { }

    void Clear() noexcept
    {
        _Items.clear();
    }

    void AddItem(const std::wstring & item) noexcept
    {
        _Items.push_back(item);
    }

    void FilterItems(const std::wstring & text) noexcept;

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv) noexcept final;
    STDMETHODIMP_(ULONG) AddRef() noexcept final;
    STDMETHODIMP_(ULONG) Release() noexcept final;

    // IEnumString
    HRESULT STDMETHODCALLTYPE Next(ULONG itemCount, LPOLESTR * items, ULONG * fetchCount) noexcept final;
    HRESULT STDMETHODCALLTYPE Skip(ULONG itemCount) noexcept final;
    HRESULT STDMETHODCALLTYPE Reset() noexcept final;
    HRESULT STDMETHODCALLTYPE Clone(IEnumString ** other) noexcept final;

private:
    static LPOLESTR CreateOLEString(const std::wstring & s) noexcept;

private:
    std::vector<std::wstring> _Items;
    std::vector<std::wstring> _FilteredItems;

    ULONG _Index = 0;

    LONG _ReferenceCount = 1;
};
