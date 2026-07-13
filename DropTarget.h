
/** $VER: DropTarget.h (2026.07.13) P. Stuer - Implements an OLE2 drop target **/

#pragma once

#include <ole2.h>
#include <shlobj.h>
#include <shobjidl.h> // For IDropTargetHelper

#include "PlaylistUIElement.h"
#include "Resources.h"
#include "Log.h"

/// <summary>
/// Implements an OLE2 drop target.
/// </summary>
class drop_target_t : public IDropTarget
{
public:
    drop_target_t(HWND hWnd, playlist_uielement_t * uiElement) : _hWnd(hWnd), _UIElement(uiElement)
    {
        HRESULT hResult = ::CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_DragDropHelper));

        if (!SUCCEEDED(hResult))
            Log.AtWarn().Write(STR_COMPONENT_BASENAME " failed to create shell drag/drop helper: 0x%08X", hResult);
    }

    virtual ~drop_target_t()
    {
        if (_DragDropHelper)
        {
            _DragDropHelper->Release();
            _DragDropHelper = nullptr;
        }
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IDropTarget
    STDMETHODIMP DragEnter(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD * effect);
    STDMETHODIMP DragOver(DWORD keyState, POINTL pt, DWORD* effect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject * dataObject, DWORD keyState, POINTL pt, DWORD * effect);

private:
    void ExamineDataObject(IDataObject * dataObject) const noexcept;

private:
    LONG _ReferenceCount = 1;

    HWND _hWnd;
    playlist_uielement_t * _UIElement;
    IDropTargetHelper * _DragDropHelper; // Allows drop targets to display a drag image while the image is over the target window. Exposed by the Shell's drag-image manager. It is not implemented by applications.
};
