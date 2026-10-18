#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <wincodec.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "jpginterfce.h"

extern fileOpenInfo filesOpen[MAX_FILES_OPEN + 1];

static bool32 jpgReadAllBytes(filehandle handle, ubyte **buffer, udword *size)
{
    sdword length;

    if (filesOpen[handle].usingBigfile)
    {
        length = filesOpen[handle].length;
    }
    else
    {
        long original = ftell(filesOpen[handle].fileP);
        fseek(filesOpen[handle].fileP, 0, SEEK_END);
        length = ftell(filesOpen[handle].fileP);
        fseek(filesOpen[handle].fileP, original, SEEK_SET);
    }

    if (length <= 0)
    {
        return FALSE;
    }

    *buffer = malloc(length);
    if (*buffer == NULL)
    {
        return FALSE;
    }

    fileSeek(handle, 0, FS_Start);
    if (fileBlockReadNoError(handle, *buffer, length) != length)
    {
        free(*buffer);
        *buffer = NULL;
        return FALSE;
    }

    *size = length;
    return TRUE;
}

static HRESULT jpgCreateStream(const ubyte *buffer, udword size, IStream **stream)
{
    HGLOBAL global;
    void *globalBuffer;
    HRESULT hr;

    global = GlobalAlloc(GMEM_MOVEABLE, size);
    if (global == NULL)
    {
        return E_OUTOFMEMORY;
    }

    globalBuffer = GlobalLock(global);
    if (globalBuffer == NULL)
    {
        GlobalFree(global);
        return E_OUTOFMEMORY;
    }

    memcpy(globalBuffer, buffer, size);
    GlobalUnlock(global);

    hr = CreateStreamOnHGlobal(global, TRUE, stream);
    if (FAILED(hr))
    {
        GlobalFree(global);
    }

    return hr;
}

static HRESULT jpgDecodeInternal(JPEGDATA *data, bool32 decodePixels)
{
    HRESULT hr;
    HRESULT coInitHr;
    bool32 shouldUninitialize = FALSE;
    ubyte *encoded = NULL;
    udword encodedSize = 0;
    IStream *stream = NULL;
    IWICImagingFactory *factory = NULL;
    IWICBitmapDecoder *decoder = NULL;
    IWICBitmapFrameDecode *frame = NULL;
    IWICFormatConverter *converter = NULL;
    UINT width = 0;
    UINT height = 0;
    UINT stride;
    UINT imageSize;
    UINT i;

    if (!jpgReadAllBytes(data->input_file, &encoded, &encodedSize))
    {
        return E_FAIL;
    }

    coInitHr = CoInitialize(NULL);
    if (coInitHr == S_OK || coInitHr == S_FALSE)
    {
        shouldUninitialize = TRUE;
    }
    else if (coInitHr != RPC_E_CHANGED_MODE)
    {
        free(encoded);
        return coInitHr;
    }

    hr = jpgCreateStream(encoded, encodedSize, &stream);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = CoCreateInstance(&CLSID_WICImagingFactory,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_IWICImagingFactory,
                          (LPVOID *)&factory);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = IWICImagingFactory_CreateDecoderFromStream(factory,
                                                    stream,
                                                    NULL,
                                                    WICDecodeMetadataCacheOnLoad,
                                                    &decoder);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = IWICBitmapFrameDecode_GetSize(frame, &width, &height);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    data->width = (sdword)width;
    data->height = (sdword)height;
    data->components = 3;

    if (!decodePixels)
    {
        hr = S_OK;
        goto cleanup;
    }

    if (data->ptr == NULL)
    {
        hr = E_POINTER;
        goto cleanup;
    }

    hr = IWICImagingFactory_CreateFormatConverter(factory, &converter);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = IWICFormatConverter_Initialize(converter,
                                        (IWICBitmapSource *)frame,
                                        &GUID_WICPixelFormat24bppBGR,
                                        WICBitmapDitherTypeNone,
                                        NULL,
                                        0.0f,
                                        WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    stride = width * 3;
    imageSize = stride * height;
    hr = IWICFormatConverter_CopyPixels(converter, NULL, stride, imageSize, data->ptr);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    for (i = 0; i < imageSize; i += 3)
    {
        ubyte blue = data->ptr[i];
        data->ptr[i] = data->ptr[i + 2];
        data->ptr[i + 2] = blue;
    }

cleanup:
    if (converter != NULL)
    {
        IWICFormatConverter_Release(converter);
    }
    if (frame != NULL)
    {
        IWICBitmapFrameDecode_Release(frame);
    }
    if (decoder != NULL)
    {
        IWICBitmapDecoder_Release(decoder);
    }
    if (factory != NULL)
    {
        IWICImagingFactory_Release(factory);
    }
    if (stream != NULL)
    {
        IStream_Release(stream);
    }
    if (shouldUninitialize)
    {
        CoUninitialize();
    }
    if (encoded != NULL)
    {
        free(encoded);
    }
    return hr;
}

void JpegWrite(JPEGDATA *data)
{
    data->status = 1;
    dbgFatalf(DBG_Loc, "JpegWrite is not implemented in this build");
}

void JpegInfo(JPEGDATA *data)
{
    data->status = FAILED(jpgDecodeInternal(data, FALSE));
}

void JpegRead(JPEGDATA *data)
{
    data->status = FAILED(jpgDecodeInternal(data, TRUE));
}
