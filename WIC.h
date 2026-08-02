
/** $VER: WIC.h (2026.08.02) P. Stuer **/

#pragma once

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4100 4625 4626 4710 4711 5045 ALL_CPPCORECHECK_WARNINGS)

#include <SDKDDKVer.h>
#include <wincodec.h>

#include <wrl/client.h>

using namespace Microsoft::WRL;

#include <string>

#include <SDK/foobar2000.h>

/// <summary>
/// Wraps a Windows Imaging Component (WIC) factory in a foobar2000 singleton service.
/// </summary>
class NOVTABLE wic_t : public service_base
{
public:
    virtual IWICImagingFactory * Get() = 0;

    ComPtr<IWICImagingFactory> GetPtr()
    {
        ComPtr<IWICImagingFactory> Ptr;

        Ptr.Attach(Get());

        return Ptr;
    }

    virtual HRESULT Load(const uint8_t * data, size_t size, IWICBitmapFrameDecode ** frame) const noexcept = 0;
    virtual HRESULT Load(const std::wstring & filePath, IWICBitmapFrameDecode ** frame) const noexcept = 0;
    virtual HRESULT Load(const std::wstring & filePath, UINT targetWidth, UINT targetHeight, HBITMAP * hBitmap) const noexcept = 0;

    virtual HRESULT GetFormatConverter(IWICBitmapSource * bitmapSource, IWICFormatConverter ** formatConverter) const noexcept = 0;

    virtual HRESULT CreateBitmapFromSource(IWICBitmapSource * bitmapSource, WICBitmapCreateCacheOption option, IWICBitmap ** bitmap) = 0;

    virtual HRESULT GetBitsPerPixel(const WICPixelFormatGUID & pixelFormat, UINT & BitsPerPixel) const noexcept = 0;

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(wic_t);
};
