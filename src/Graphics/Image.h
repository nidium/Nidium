/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_image_h__
#define graphics_image_h__

#include <SkBitmap.h>
#include <SkSurface.h>
#include <SkImage.h>
#include <SkRefCnt.h>

class SkCanvas;
class SkBitmap;
class SkData;

/**
  *  Image hold a reference to an image into the GPU.
  *  It's meant to be later drawn into a surface.
  *
*/
namespace Nidium {
namespace Graphics {

class Image
{
public:
    /*
        Create from encoded data source (png, jpeg, ...)
    */
    static Image *CreateFromEncoded(void *data, size_t len);
    /*
        Create from pixel bitmap
    */
    static Image *CreateFromRGBA(void *data, int width, int height);

    /*  
        Create from a surface
    */
    static Image *CreateFromSurface(sk_sp<SkSurface> surface);

    /*
        Create from an existing SkImage. Will hold a reference
    */
    static Image *CreateFromSkImage(sk_sp<SkImage> skimage);

    /*
        Try to encode the pixels as a PNG image
        This can fail and return nullptr.
    */
    SkData   *getPNG();
    SkBitmap *getBitmap();

    bool readPixels(unsigned char *buf, bool flipY, bool premultiply);
    
    /*
        Try to readback the pixels.
        This can fail and return nullptr.
    */
    const uint8_t *getPixels(size_t *len);

    uint32_t getSize() const;
    int      getWidth();
    int      getHeight();

    ~Image();

    int m_IsCanvas;
    sk_sp<SkImage> m_Image;
private:
    Image() = default;

    SkBitmap *m_ImageBitmap = nullptr;
};

} // namespace Graphics
} // namespace Nidium

#endif
