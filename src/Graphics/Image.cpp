/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/Image.h"

#include <stdio.h>

#include <SkCanvas.h>
#include <SkColorPriv.h>
#include <SkUnPreMultiply.h>

#include "Core/Utils.h"

namespace Nidium {
namespace Graphics {

// {{{ Constructors
Image::Image(SkCanvas *canvas)
{
    // canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &img);
#if 0
    m_IsCanvas  = 1;
    m_CanvasRef = sk_sp<SkCanvas>(canvas);
    m_Image = NULL;
#endif
}

Image::Image(void *data, size_t len)
{
    m_IsCanvas = 0;

    sk_sp<SkData> skdata = SkData::MakeWithoutCopy(data, len);
    m_Image = SkImage::MakeFromEncoded(skdata);

    if (!m_Image) {
        printf("failed to decode Image\n");
    }
}

Image::Image(void *data, int width, int height)
{
#if 0
    m_Image     = new SkBitmap();
    m_IsCanvas  = 0;

    m_Image->setConfig(SkBitmap::kARGB_8888_Config, width, height);

    m_Image->setPixels(data);
#endif
}

const uint8_t *Image::getPixels(size_t *len)
{
#if 0
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

    printf("Pixels : %x %d\n", (static_cast<uint8_t *>(data)[500]),
           m_Image->height());

    return static_cast<const uint8_t *>(m_Image->getPixels());
#endif
    return 0;
}

SkData *Image::getPNG()
{
#if 0
    if (!m_Image) {
        return NULL;
    }

    // It seems that SkImageEncoder expect the pixels to be BGRA 
    // but they are RGBA. Switch blue and red to workaround this.
    // FIXME : There must be a better way to handle this.
    uint8_t* data = static_cast<uint8_t *>(m_Image->getPixels());
    uint8_t* p = data;
    uint8_t* stop = p + m_Image->getSize();

    while (p < stop) {
        unsigned r = p[0];
        unsigned b = p[2];
        p[0] = b;
        p[2] = r;
        p += 4;
    }

    SkData *dataPNG = SkImageEncoder::EncodeData(*m_Image, SkImageEncoder::kPNG_Type, 100);

    // Switch back blue and red, so the image stays unchanged.
    p = data;
    while (p < stop) {
        unsigned b = p[0];
        unsigned r = p[2];
        p[0] = r;
        p[2] = b;
        p += 4;
    }

    return dataPNG;
#endif
}

int Image::getWidth()
{
    return m_Image->width();
}

int Image::getHeight()
{
    return m_Image->height();
}


bool Image::ConvertToRGBA(Image *nimg,
                          unsigned char *rgba,
                          bool flipY,
                          bool premultiply)
{
#if 0
    int length;
    int k;
    const unsigned char *pixels;
    int width;

    nimg->m_Image->lockPixels();
    if ((pixels
         = static_cast<const unsigned char *>(nimg->m_Image->getPixels()))
        == NULL) {
        nimg->m_Image->unlockPixels();
        return false;
    }

    width  = nimg->getWidth();
    length = width * nimg->getHeight() * 4;
    width *= 4;
    k = flipY ? length - width : 0;

    for (int i = 0; i < length; i += 4) {
        const uint32_t pixel = *reinterpret_cast<const uint32_t *>(&pixels[i]);
        int alpha            = SkGetPackedA32(pixel);

        if (alpha != 0 && alpha != 255 && !premultiply) {
            SkColor unmultiplied = SkUnPreMultiply::PMColorToColor(pixel);
            rgba[k + 0]          = SkColorGetR(unmultiplied);
            rgba[k + 1]          = SkColorGetG(unmultiplied);
            rgba[k + 2]          = SkColorGetB(unmultiplied);
            rgba[k + 3]          = alpha;
        } else {
            rgba[k + 0] = SkGetPackedR32(pixel);
            rgba[k + 1] = SkGetPackedG32(pixel);
            rgba[k + 2] = SkGetPackedB32(pixel);
            rgba[k + 3] = alpha;
        }

        if (flipY && (i + 4) % width == 0) {
            k = length - (i + 4) - width;
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

}

} // namespace Graphics
} // namespace Nidium
