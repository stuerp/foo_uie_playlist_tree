
/** $VER: StringEnumerator.cpp (2026.08.14) P. Stuer - Implements an IEnumString enumerator for AutoComplete **/

#include "pch.h"

#include "StringEnumerator.h"

#include <algorithm>
#include <shlwapi.h>

#pragma hdrstop

/// <summary>
/// Retrieves pointers to the supported interfaces on an object.
/// </summary>
HRESULT STDMETHODCALLTYPE string_enumerator_t::QueryInterface(REFIID riid, void ** ppv) noexcept
{
    if (riid == IID_IUnknown || riid == IID_IEnumString)
    {
        *ppv = (IEnumString *) this;

        AddRef();

        return S_OK;
    }

    *ppv = nullptr;

    return E_NOINTERFACE;
}

/// <summary>
/// Increments the reference count for an interface pointer to a COM object.
/// </summary>
STDMETHODIMP_(ULONG) string_enumerator_t::AddRef() noexcept
{
    return (ULONG) InterlockedIncrement(&_ReferenceCount);
}

/// <summary>
/// Decrements the reference count for an interface on a COM object.
/// </summary>
STDMETHODIMP_(ULONG) string_enumerator_t::Release() noexcept
{
    const auto NewReferenceCount = InterlockedDecrement(&_ReferenceCount);

    if (NewReferenceCount == 0)
        delete this;

    return (ULONG) NewReferenceCount;
}

/// <summary>
/// Retrieves the specified number of items in the enumeration sequence.
/// </summary>
HRESULT STDMETHODCALLTYPE string_enumerator_t::Next(ULONG itemCount, LPOLESTR * items, ULONG * fetchCount) noexcept
{
    if (items == nullptr)
        return E_POINTER;

    ULONG NumFetched = 0;

    while ((NumFetched < itemCount) && (_Index < _FilteredItems.size()))
    {
        const auto & Item = _FilteredItems[_Index++];

        items[NumFetched] = CreateOLEString(Item);

        if (items[NumFetched] == nullptr)
            return E_OUTOFMEMORY;

        ++NumFetched;
    }

    if (fetchCount != nullptr)
        *fetchCount = NumFetched;

    return (NumFetched == itemCount) ? S_OK : S_FALSE;
}

/// <summary>
/// Skips over the specified number of items in the enumeration sequence.
/// </summary>
HRESULT STDMETHODCALLTYPE string_enumerator_t::Skip(ULONG itemCount) noexcept
{
    _Index = std::min(_Index + itemCount, (ULONG) _Items.size());

    return (_Index < _Items.size()) ? S_OK : S_FALSE;
}

/// <summary>
/// Resets the enumeration sequence to the beginning.
/// </summary>
HRESULT STDMETHODCALLTYPE string_enumerator_t::Reset() noexcept
{
    _Index = 0;

    return S_OK;
}

/// <summary>
/// Creates a new enumerator that contains the same enumeration state as the current one.
/// </summary>
HRESULT STDMETHODCALLTYPE string_enumerator_t::Clone(IEnumString ** other) noexcept
{
    if (other == nullptr)
        return E_POINTER;

    auto p = new (std::nothrow) string_enumerator_t();

    if (p == nullptr)
        return E_OUTOFMEMORY;

    p->_Items = _Items;
    p->_Index = _Index;

    *other = p;

    return S_OK;
}

/// <summary>
/// Creates an OLE string from the specified wide string.
/// </summary>
LPOLESTR string_enumerator_t::CreateOLEString(const std::wstring & s) noexcept
{
    const size_t Size = (s.size() + 1) * sizeof(WCHAR);

    const auto p = (LPOLESTR) ::CoTaskMemAlloc(Size); // The caller (AutoComplete) frees the memory with CoTaskMemFree().

    if (p == nullptr)
        return p;

    ::memcpy(p, s.c_str(), Size);

    return p;
}

/// <summary>
/// Filters the items and retains those that contain the specified text.
/// </summary>
void string_enumerator_t::FilterItems(const std::wstring & text) noexcept
{
    _Index = 0;

    if (text.empty())
    {
        _FilteredItems = _Items;

        return;
    }

    _FilteredItems.clear();

    for (auto & Item : _Items)
    {
        if (::StrStrIW(Item.c_str(), text.c_str()) != nullptr)
            _FilteredItems.push_back(Item);
    }

    std::sort(_FilteredItems.begin(), _FilteredItems.end(), [](const std::wstring & x, const std::wstring & y) { return x < y; });
}
