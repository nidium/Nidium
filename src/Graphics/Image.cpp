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

Image *Image::CreateFromEncoded(void *data, size_t len)
{
    Image *img = new Image();
    img->m_IsCanvas = 0;

    sk_sp<SkData> skdata = SkData::MakeWithCopy(data, len);
    img->m_Image = SkImage::MakeFromEncoded(skdata);

    if (!img->m_Image) {
        delete img;
        return nullptr;
    }

    return img;
}

Image *Image::CreateFromRGBA(void *data, int width, int height)
{
    Image *img = new Image();
    img->m_IsCanvas = 0;

    SkBitmap bt;
    bt.setIsVolatile(true);

    bt.setInfo(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
    bt.setPixels(data);
    bt.setImmutable();

    img->m_Image = SkImage::MakeFromBitmap(bt);

    if (!img->m_Image) {
        delete img;
        return nullptr;
    }

    return img;
}

Image *Image::CreateFromSurface(sk_sp<SkSurface> surface)
{
    printf("Image::CreateFromSurface. Not implemented\n");
    return nullptr;
}

Image *Image::CreateFromSkImage(sk_sp<SkImage> skimage)
{
    if (!skimage.get()) {
        return nullptr;
    } 

    Image *img = new Image();
    img->m_IsCanvas = 0;

    img->m_Image = skimage;

    return img;
}

SkBitmap *Image::getBitmap()
{
    if (m_ImageBitmap) {
      return m_ImageBitmap;
    }
    m_ImageBitmap = new SkBitmap();
    
    if (!m_Image->asLegacyBitmap(m_ImageBitmap, SkImage::kRO_LegacyBitmapMode)) {
      delete m_ImageBitmap;

      return nullptr;
    }

    return m_ImageBitmap;
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

    APE_DEBUG("Graphics", "[Image] Pixels : %x %d\n", (static_cast<uint8_t *>(data)[500]),
           m_Image->height());

    return static_cast<const uint8_t *>(m_Image->getPixels());
#endif
    return 0;
}

uint32_t Image::getSize() const
{
    return m_Image->width() * m_Image->width() * 4;
}

SkData *Image::getPNG()
{
    return m_Image->encode(SkEncodedImageFormat::kPNG, 100);
}

int Image::getWidth()
{
    return m_Image->width();
}

int Image::getHeight()
{
    return m_Image->height();
}

bool Image::readPixels(unsigned char *buf, bool flipY, bool premultiply)
{
    if (!m_Image) {
        return false;
    }

    return m_Image->readPixels(
                    SkImageInfo::Make(m_Image->width(),m_Image->height(),
                        kRGBA_8888_SkColorType,
                        premultiply
                          ? kPremul_SkAlphaType
                          : kUnpremul_SkAlphaType),
                    buf, m_Image->width() * 4, 0, 0);
}

#if 0
bool Image::ConvertToRGBA(Image *nimg,
                          unsigned char *rgba,
                          bool flipY,
                          bool premultiply)
{

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

    return true;
}
#endif
// }}}

Image::~Image()
{
    if (m_ImageBitmap) {
        delete m_ImageBitmap;
    }
}

} // namespace Graphics
} // namespace Nidium
