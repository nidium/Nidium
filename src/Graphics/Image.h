/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_image_h__
#define graphics_image_h__

#include <SkBitmap.h>
#include <SkImage.h>
#include <SkRefCnt.h>

class SkCanvas;
class SkBitmap;
class SkData;

namespace Nidium {
namespace Graphics {

class Image
{
public:
    int m_IsCanvas;
    //sk_sp<SkCanvas> m_CanvasRef;
    sk_sp<SkImage> m_Image;
#if 0
    SkImage *fixedImg;
#endif
    Image(SkCanvas *canvas);
    Image(void *data, size_t len);
    Image(void *data, int width, int height);

    SkData *getPNG();

    uint32_t getSize() const;

    SkBitmap *getBitmap() {
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

    static bool ConvertToRGBA(Image *nimg,
                              unsigned char *rgba,
                              bool flipY,
                              bool premultiply);

    ~Image();

    const uint8_t *getPixels(size_t *len);

    int getWidth();
    int getHeight();
private:
    SkBitmap *m_ImageBitmap = nullptr;
};

} // namespace Graphics
} // namespace Nidium

#endif
