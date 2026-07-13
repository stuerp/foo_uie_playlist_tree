
/** $VER: DropTarget.cpp (2026.07.13) P. Stuer - Implements an OLE2 drop target **/

#include "pch.h"

#include "DropTarget.h"

#pragma hdrstop

STDMETHODIMP drop_target_t::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IDropTarget)
    {
        *ppv = static_cast<IDropTarget*>(this);

        AddRef();

        return S_OK;
    }

    *ppv = nullptr;

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) drop_target_t::AddRef()
{
    return (ULONG) ::InterlockedIncrement(&_ReferenceCount);
}

STDMETHODIMP_(ULONG) drop_target_t::Release()
{
    auto NewReferenceCount = (ULONG) InterlockedDecrement(&_ReferenceCount);

    if (NewReferenceCount == 0)
        delete this;

    return NewReferenceCount;
}

// IDropTarget
STDMETHODIMP drop_target_t::DragEnter(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD * effect)
{
    POINT Point{ pt.x, pt.y };

    *effect = _UIElement->GetDropEffect(keyState, Point);

    (void) _DragDropHelper->DragEnter(_hWnd, dataObject, &Point, *effect);

    return S_OK;
}

STDMETHODIMP drop_target_t::DragOver(DWORD keyState, POINTL pt, DWORD* effect)
{
    POINT Point{ pt.x, pt.y };

    *effect = _UIElement->GetDropEffect(keyState, Point);

    (void) _DragDropHelper->DragOver(&Point, *effect);

    return S_OK;
}

STDMETHODIMP drop_target_t::DragLeave()
{
    (void) _DragDropHelper->DragLeave();

    return S_OK;
}

STDMETHODIMP drop_target_t::Drop(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD * effect)
{
    POINT Point{ pt.x, pt.y };

    *effect = _UIElement->GetDropEffect(keyState, Point);

    if (*effect == DROPEFFECT_NONE)
        return S_OK;

    HRESULT hResult = _DragDropHelper->Drop(dataObject, &Point, *effect);

#ifdef _DEBUG
    ExamineDataObject(dataObject);
#endif

    FORMATETC FormatEtc =
    {
        .cfFormat = CF_HDROP,
        .ptd      = nullptr,
        .dwAspect = DVASPECT_CONTENT,
        .lindex   = -1,
        .tymed    = TYMED_HGLOBAL
    };

    STGMEDIUM StgMedium = { };

    hResult = dataObject->GetData(&FormatEtc, &StgMedium);

    if (!SUCCEEDED(hResult))
        return hResult;

    // Enumerate all files.
    {
        const auto hDrop = (HDROP) StgMedium.hGlobal;

        const UINT DropCount = ::DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

        std::vector<std::wstring> FilePaths;

        FilePaths.reserve(DropCount);

        for (UINT i = 0; i < DropCount; ++i)
        {
            wchar_t FilePath[MAX_PATH];

            if (::DragQueryFileW(hDrop, i, FilePath, _countof(FilePath)))
                FilePaths.push_back(FilePath);
        }

        if (FilePaths.size() != 0)
            _UIElement->DropFiles(FilePaths);
    }

    ::ReleaseStgMedium(&StgMedium);

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

    HRESULT hResult = dataObject->EnumFormatEtc(DATADIR_GET, &EnumFORMATETC);

    if (!SUCCEEDED(hResult))
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

        Log.Write("Format: 0x%04X %s", fmt.cfFormat, FormatName.c_str());
    }

    EnumFORMATETC->Release();
}
