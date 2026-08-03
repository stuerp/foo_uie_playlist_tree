
/** $VER: DropTarget.cpp (2026.07.17) P. Stuer - Implements an OLE2 drop target **/

#include "pch.h"

#include "DropTarget.h"

#include "PlaylistUIElement.h"

#pragma hdrstop

// IUnknown
STDMETHODIMP drop_target_t::QueryInterface(REFIID riid, void ** ppv) noexcept
{
    if (riid == IID_IUnknown || riid == IID_IDropTarget)
    {
        *ppv = (IDropTarget *) this;

        AddRef();

        return S_OK;
    }

    *ppv = nullptr;

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) drop_target_t::AddRef() noexcept
{
    return (ULONG) ::InterlockedIncrement(&_ReferenceCount);
}

STDMETHODIMP_(ULONG) drop_target_t::Release() noexcept
{
    const auto NewReferenceCount = InterlockedDecrement(&_ReferenceCount);

    if (NewReferenceCount == 0)
        delete this;

    return (ULONG) NewReferenceCount;
}

// IDropTarget
STDMETHODIMP drop_target_t::DragEnter(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD * effect) noexcept
{
    POINT Point{ pt.x, pt.y };

    *effect = _UIElement->GetDropEffect(keyState, Point);

    (void) _DragDropHelper->DragEnter(_hWnd, dataObject, &Point, *effect);

    static_api_ptr_t<playlist_incoming_item_filter>()->process_dropped_files_check_ex(dataObject, effect);

    return S_OK;
}

STDMETHODIMP drop_target_t::DragOver(DWORD keyState, POINTL pt, DWORD* effect) noexcept
{
    POINT Point{ pt.x, pt.y };

    *effect = _UIElement->GetDropEffect(keyState, Point);

    (void) _DragDropHelper->DragOver(&Point, *effect);

    return S_OK;
}

STDMETHODIMP drop_target_t::DragLeave() noexcept
{
    (void) _DragDropHelper->DragLeave();

    return S_OK;
}

STDMETHODIMP drop_target_t::Drop(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD * effect) noexcept
{
    POINT Point{ pt.x, pt.y };

    *effect = _UIElement->GetDropEffect(keyState, Point);

    if (*effect == DROPEFFECT_NONE)
        return S_OK;

    HRESULT hr = _DragDropHelper->Drop(dataObject, &Point, *effect);

    if (!SUCCEEDED(hr))
        return hr;

#ifdef _DEBUG
    ExamineDataObject(dataObject);
#endif

    _UIElement->DropFiles(dataObject);

    return S_OK;
}

/// <summary>
/// Examines the formats in a data object.
/// </summary>
void drop_target_t::ExamineDataObject(IDataObject * dataObject) const noexcept
{
    static const std::unordered_map<int, std::string_view> Formats
    {
        { CF_TEXT,          "Text" },
        { CF_BITMAP,        "Bitmap" },
        { CF_METAFILEPICT,  "Metafile" },
        { CF_SYLK,          "SYLK" },
        { CF_DIF,           "DIF" },
        { CF_TIFF,          "TIFF" },
        { CF_OEMTEXT,       "OEM Text" },
        { CF_DIB,           "DIB" },
        { CF_PALETTE,       "Palette" },
        { CF_PENDATA,       "Pen Data" },
        { CF_RIFF,          "RIFF" },
        { CF_WAVE,          "WAV" },
        { CF_UNICODETEXT,   "Unicode Text" },
        { CF_ENHMETAFILE,   "EMF" },
        { CF_HDROP,         "HDROP" },
        { CF_LOCALE,        "Locale" },
        { CF_DIBV5,         "DIB v5" },
    };

    IEnumFORMATETC * EnumFORMATETC = nullptr;

    HRESULT hr = dataObject->EnumFormatEtc(DATADIR_GET, &EnumFORMATETC);

    if (!SUCCEEDED(hr))
        return;

    FORMATETC fmt = { };
    DWORD FetchCount = 0;

    while (EnumFORMATETC->Next(1, &fmt, &FetchCount) == S_OK)
    {
        std::string FormatName;

        if (auto Iter = Formats.find(fmt.cfFormat); Iter != Formats.end())
            FormatName = Iter->second;
        else
        {
            FormatName.resize(256);

            if (::GetClipboardFormatNameA(fmt.cfFormat, FormatName.data(), (int) FormatName.size()) == 0)
                FormatName = "<Unknown format>";
        }

        Log.AtDebug().Write("Format: 0x%04X %s", fmt.cfFormat, FormatName.c_str());
    }

    EnumFORMATETC->Release();
}
