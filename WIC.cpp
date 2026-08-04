
/** $VER: WIC.cpp (2026.08.03) P. Stuer **/

#include "pch.h"

#include "WIC.h"

#pragma comment(lib, "windowscodecs")

const GUID wic_t::class_guid = { 0x5af88ed, 0xa6be, 0x4600, { 0x85, 0x28, 0x75, 0xd3, 0x1b, 0x89, 0xb3, 0x4b } };

/// <summary>
/// Implements the wic_t service.
/// </summary>
class wic_t_impl : public wic_t
{
public:
    wic_t_impl()
    {
        HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        if (FAILED(hr))
            throw exception_com(hr);

        hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(_Factory.ReleaseAndGetAddressOf()));

        if (FAILED(hr))
            hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(_Factory.ReleaseAndGetAddressOf()));

        if (FAILED(hr))
            throw exception_com(hr);
    }

    virtual ~wic_t_impl() noexcept
    {
        ::CoUninitialize();
    }

    IWICImagingFactory * Get() override
    {
        _Factory->AddRef();

        return _Factory.Get();
    }

    HRESULT Load(const uint8_t * data, size_t size, IWICBitmapFrameDecode ** frame) const noexcept override;
    HRESULT Load(const std::wstring & filePath, IWICBitmapFrameDecode ** frame) const noexcept override;
    HRESULT Load(const std::wstring & filePath, UINT targetWidth, UINT targetHeight, HBITMAP * hBitmap) const noexcept;

    HRESULT GetFormatConverter(IWICBitmapSource * bitmapSource, IWICFormatConverter ** formatConverter) const noexcept override;

    HRESULT CreateBitmapFromSource(IWICBitmapSource * bitmapSource, WICBitmapCreateCacheOption option, IWICBitmap ** bitmap) override
    {
        return _Factory->CreateBitmapFromSource(bitmapSource, option, bitmap);
    }

    HRESULT GetBitsPerPixel(const WICPixelFormatGUID & pixelFormat, UINT & BitsPerPixel) const noexcept override;

private:
    ComPtr<IWICImagingFactory> _Factory;
};

static service_factory_single_t<wic_t_impl> _WICFactoryFactory;

/// <summary>
/// Creates a WIC bitmap frame from raw image data.
/// </summary>
HRESULT wic_t_impl::Load(const uint8_t * data, size_t size, IWICBitmapFrameDecode ** frame) const noexcept
{
    if ((data == nullptr) || (size == 0))
        return E_FAIL;

    ComPtr<IWICStream> Stream;

    HRESULT hr = _Factory->CreateStream(&Stream);

    if (!SUCCEEDED(hr))
        return hr;

    hr = Stream->InitializeFromMemory((BYTE *) data, (DWORD) size);

    ComPtr<IWICBitmapDecoder> Decoder;

    if (!SUCCEEDED(hr))
        return hr;

    hr = _Factory->CreateDecoderFromStream(Stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, Decoder.ReleaseAndGetAddressOf());

    if (!SUCCEEDED(hr))
        return hr;

    hr = Decoder->GetFrame(0, frame);

    return hr;
}

/// <summary>
/// Creates a WIC bitmap frame from a file.
/// </summary>
HRESULT wic_t_impl::Load(const std::wstring & filePath, IWICBitmapFrameDecode ** frame) const noexcept
{
    if (filePath.empty())
        return E_FAIL;

    ComPtr<IWICBitmapDecoder> Decoder;

    HRESULT hr = _Factory->CreateDecoderFromFilename(filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, Decoder.ReleaseAndGetAddressOf());

    if (!SUCCEEDED(hr))
        return hr;

    hr = Decoder->GetFrame(0, frame);

    return hr;
}

/// <summary>
/// Creates a GDI bitmap from a file.
/// </summary>
HRESULT wic_t_impl::Load(const std::wstring & filePath, UINT targetWidth, UINT targetHeight, HBITMAP * hBitmap) const noexcept
{
    if (hBitmap == nullptr)
        return E_INVALIDARG;

    ComPtr<IWICBitmapFrameDecode> Frame;

    HRESULT hr = Load(filePath, &Frame);

    if (!SUCCEEDED(hr))
        return hr;

    ComPtr<IWICBitmapScaler> Scaler;

    hr = _Factory->CreateBitmapScaler(Scaler.ReleaseAndGetAddressOf());

    Scaler->Initialize(Frame.Get(), targetWidth, targetHeight, WICBitmapInterpolationModeFant);

    ComPtr<IWICFormatConverter> Converter;

    hr = GetFormatConverter(Scaler.Get(), Converter.ReleaseAndGetAddressOf());

    if (!SUCCEEDED(hr))
        return hr;

    UINT Width = 0;
    UINT Height = 0;

    hr = Converter->GetSize(&Width, &Height);

    if (!SUCCEEDED(hr))
        return hr;

    const BITMAPINFO bmi
    {
        .bmiHeader =
        {
            .biSize        = sizeof(BITMAPINFOHEADER),
            .biWidth       = (LONG) Width,
            .biHeight      = -(LONG) Height,
            .biPlanes      = 1,
            .biBitCount    = 32,
            .biCompression = BI_RGB
        }
    };

    // Get the screen DC.
    HDC hDC = ::GetDC(NULL);

    BYTE * Bits = nullptr;

    const HBITMAP hDIB = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void **) &Bits, NULL, 0);

    ::ReleaseDC(NULL, hDC);

    if ((hDIB == NULL) || (Bits == nullptr))
        return hr;

    constexpr UINT BytesPerPixel = 4;

    const UINT Stride = ((BytesPerPixel * Width) + 3) & ~3;
    const UINT BufferSize = Stride * Height;

    hr = Converter->CopyPixels(nullptr, Stride, BufferSize, Bits);

    if (!SUCCEEDED(hr))
    {
        ::DeleteObject(hDIB);

        return hr;
    }

    *hBitmap = hDIB;

    return S_OK;
}

/// <summary>
/// Gets the number of bits per pixel for the specified pixel format.
/// </summary>
HRESULT wic_t_impl::GetBitsPerPixel(const WICPixelFormatGUID & pixelFormat, UINT & bitsPerPixel) const noexcept
{
    ComPtr<IWICComponentInfo> ComponentInfo;

    HRESULT hr = _Factory->CreateComponentInfo(pixelFormat, ComponentInfo.ReleaseAndGetAddressOf());

    ComPtr<IWICPixelFormatInfo> PixelFormatInfo;

    if (!SUCCEEDED(hr))
        return hr;

    hr = ComponentInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), (void **) PixelFormatInfo.ReleaseAndGetAddressOf());

    if (!SUCCEEDED(hr))
        return hr;

    hr = PixelFormatInfo->GetBitsPerPixel(&bitsPerPixel);

    return hr;
}

/// <summary>
/// Creates a format converter to convert a WIC frame to the specfied format.
/// </summary>
HRESULT wic_t_impl::GetFormatConverter(IWICBitmapSource * bitmapSource, IWICFormatConverter ** formatConverter) const noexcept
{
    // Convert the format of the frame to 32bppPBGRA.
    HRESULT hr = _Factory->CreateFormatConverter(formatConverter);

    if (!SUCCEEDED(hr))
        return hr;

    hr = (*formatConverter)->Initialize(bitmapSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);

    return hr;
}
