/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/Image.h"

#include <stdio.h>

#include <SkCanvas.h>
#include <SkImageRef_GlobalPool.h>
#include <SkColorPriv.h>
#include <SkUnPreMultiply.h>

#include <Core/Utils.h>


namespace Nidium {
namespace Graphics {

// {{{ Constructors
Image::Image(SkCanvas *canvas)
{
    //canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &img);

    m_IsCanvas = 1;
    m_CanvasRef = canvas;
    canvas->ref();
    m_Image = NULL;
}

Image::Image(void *data, size_t len) :
    m_CanvasRef(NULL)
{
    m_Image = new SkBitmap();
    m_IsCanvas = 0;

    if (!SkImageDecoder::DecodeMemory(data, len, m_Image)) {
        printf("failed to decode Image\n");
        delete m_Image;
        m_Image = NULL;
    }
}

Image::Image(void *data, int width, int height)
{
    uint8_t *px = (uint8_t *)data;
    m_Image = new SkBitmap();
    m_IsCanvas = 0;

    m_Image->setConfig(SkBitmap::kARGB_8888_Config, width, height);

    printf("First pixel value %.2x\n", px[0]);
    printf("First pixel value %.2x\n", px[1]);
    printf("First pixel value %.2x\n", px[2]);
    printf("First pixel value %.2x\n", px[3]);

    m_Image->setPixels(data);
}
// }}}

// {{{ Methods
#if 0
static bool SetImageRef(SkBitmap* bitmap, SkStream* stream,
                        SkBitmap::Config pref, const char name[] = NULL)
{
    if (SkImageDecoder::DecodeStream(stream, bitmap, pref,
                                 SkImageDecoder::kDecodeBounds_Mode, NULL)) {
        SkASSERT(bitmap->config() != SkBitmap::kNo_Config);

        SkImageRef* ref = new SkImageRef_GlobalPool(stream, bitmap->config(), 1);
        ref->setURI(name);
        bitmap->setPixelRef(ref)->unref();
        return true;
    } else {
        return false;
    }
}
static SkData* dataToData(void *data, size_t size) {

    return SkData::NewWithProc(data, size, NULL, NULL);
}
#endif

const uint8_t *Image::getPixels(size_t *len)
{
    if (len) {
        *len = 0;
    }
    if (!m_Image) {
        return NULL;
    }

    if (len) {
        *len = m_Image->getSize();
    }
    void *data = m_Image->getPixels();

    printf("Pixels : %x %d\n", (static_cast<uint8_t *>(data)[500]), m_Image->height());

    return static_cast<const uint8_t *>(m_Image->getPixels());
}

SkData *Image::getPNG()
{
    if (!m_Image) {
        return NULL;
    }

    return SkImageEncoder::EncodeData(*m_Image, SkImageEncoder::kPNG_Type, 100);
}

int Image::getWidth()
{
    return m_Image->width();
}

int Image::getHeight()
{
    return m_Image->height();
}

void Image::shiftHue(int val, U8CPU alpha)
{
    if (!m_Image) return;

    size_t size = m_Image->getSize() >> m_Image->shiftPerPixel();

    SkColor *pixels = static_cast<SkColor *>(m_Image->getPixels());

    for (int i = 0; i < size; i++) {

        SkColor pixel = pixels[i];

        /* Skip alpha pixel and not matching color */
        if (SkColorGetA(pixel) == 0 || SkColorGetA(pixel) != alpha) {
            continue;
        }

        SkScalar hsv[3];
        SkColorToHSV(pixel, hsv);

        hsv[0] = nidium_min(nidium_max(SkIntToScalar(val), 0), 360);

        pixels[i] = SkHSVToColor(SkColorGetA(pixel), hsv);
    }

    m_Image->notifyPixelsChanged();
}

void Image::markColorsInAlpha()
{
    if (!m_Image) return;

    size_t size = m_Image->getSize() >> m_Image->shiftPerPixel();

    SkColor *pixels = static_cast<SkColor *>(m_Image->getPixels());
    for (int i = 0; i < size; i++) {
        U8CPU alpha = 0;

        SkColor pixel = pixels[i];

        /* Skip alpha */
        if (SkColorGetA(pixel) != 255) {
            continue;
        }
        if ((pixel & 0xFFFF00FF) == pixel) {
            alpha = 254;
        } else if ((pixel & 0xFFFFFF00) == pixel) {
            alpha = 253;
        } else if ((pixel & 0xFF00FFFF) == pixel) {
            alpha = 252;
        }

        pixels[i] = SkColorSetA(pixels[i], alpha);
    }

    m_Image->notifyPixelsChanged();

}

void Image::desaturate()
{
    if (!m_Image) return;

    size_t size = m_Image->getSize() >> m_Image->shiftPerPixel();

    SkColor *pixels = static_cast<SkColor *>(m_Image->getPixels());
    for (int i = 0; i < size; i++) {
        SkColor pixel = pixels[i];

        /* Skip alpha */
        if (SkColorGetA(pixel) == 0) {
            continue;
        }
        SkScalar hsv[3];
        SkColorToHSV(pixel, hsv);
        hsv[1] = SkDoubleToScalar(0);

        pixels[i] = SkHSVToColor(SkColorGetA(pixel), hsv);
    }

    m_Image->notifyPixelsChanged();
}


bool Image::ConvertToRGBA(Image *nimg, unsigned char* rgba,
        bool flipY, bool premultiply)
{
#if 1
    int length;
    int k;
    const unsigned char *pixels;
    int width;

    nimg->m_Image->lockPixels();
    if ((pixels = static_cast<const unsigned char*>(nimg->m_Image->getPixels())) == NULL) {
        nimg->m_Image->unlockPixels();
        return false;
    }

    width = nimg->getWidth();
    length = width * nimg->getHeight() * 4;
    width *= 4;
    k = flipY ? length - width: 0;

    for (int i = 0; i < length; i += 4) {
        const uint32_t pixel = *reinterpret_cast<const uint32_t*>(&pixels[i]);
        int alpha = SkGetPackedA32(pixel);

        if (alpha != 0 && alpha != 255 && !premultiply) {
            SkColor unmultiplied = SkUnPreMultiply::PMColorToColor(pixel);
            rgba[k + 0] = SkColorGetR(unmultiplied);
            rgba[k + 1] = SkColorGetG(unmultiplied);
            rgba[k + 2] = SkColorGetB(unmultiplied);
            rgba[k + 3] = alpha;
        } else {
            rgba[k + 0] = SkGetPackedR32(pixel);
            rgba[k + 1] = SkGetPackedG32(pixel);
            rgba[k + 2] = SkGetPackedB32(pixel);
            rgba[k + 3] = alpha;
        }

        if (flipY && (i+4) % width == 0) {
            k = length - (i+4) - width;
        } else {
            k += 4;
        }
    }
#endif
    return true;
}
// }}}

Image::~Image()
{
    if (m_CanvasRef) m_CanvasRef->unref();
    if (m_Image) {
        delete m_Image;
    }
}

} // namespace Graphics
} // namespace Nidium

