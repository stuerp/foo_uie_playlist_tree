
/** $VER: StringEnumerator.cpp (2026.07.29) P. Stuer - Implements an IEnumString enumerator for AutoComplete **/

#include "pch.h"

#include "StringEnumerator.h"

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

    while ((NumFetched < itemCount) && (_Index < _Items.size()))
    {
        const auto & Item = _Items[_Index++];

        const size_t Size = Item.size() + 1;

        items[NumFetched] = (LPOLESTR) ::CoTaskMemAlloc(Size * sizeof(WCHAR)); // The caller (AutoComplete) frees the memory with CoTaskMemFree().

        if (items[NumFetched] == nullptr)
            return E_OUTOFMEMORY;

        ::wcscpy_s(items[NumFetched], Size, Item.c_str());

        ++NumFetched;
    }

    if (fetchCount)
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
