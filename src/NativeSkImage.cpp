#include "NativeSkImage.h"

#include <SkCanvas.h>
#include <SkBitmap.h>
#include <SkRect.h>
#include <SkImageRef_GlobalPool.h>
#include <SkStream.h>
#include <SkData.h>
#include <SkColor.h>
#include <SkColorPriv.h>
#include <SkUnPreMultiply.h>

#include "NativeSkia.h"

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

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

const uint8_t *NativeSkImage::getPixels(size_t *len)
{
    if (len) {
        *len = 0;
    }
    if (!img) {
        return NULL;
    }

    if (len) {
        *len = img->getSize();
    }
    void *data = img->getPixels();

    printf("Pixels : %x %d\n", ((uint8_t *)data)[500], img->height());

    return (const uint8_t *)img->getPixels();
}

NativeSkImage::NativeSkImage(SkCanvas *canvas)
{
    //canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &img);

    isCanvas = 1;
    canvasRef = canvas;
    canvas->ref();
    img = NULL;
}

NativeSkImage::NativeSkImage(void *data, size_t len) :
    canvasRef(NULL)
{
    img = new SkBitmap();
    isCanvas = 0;

    if (!SkImageDecoder::DecodeMemory(data, len, img)) {
        printf("failed to decode Image\n");
        delete img;
        img = NULL;
    }
}

NativeSkImage::~NativeSkImage()
{
    if (canvasRef) canvasRef->unref();
    if (img) {
        delete img;
    }
}

int NativeSkImage::getWidth()
{
    return img->width();
}

int NativeSkImage::getHeight()
{
    return img->height();
}

void NativeSkImage::shiftHue(int val, U8CPU alpha)
{
    if (!img) return;

    size_t size = img->getSize() >> img->shiftPerPixel();

    SkColor *pixels = (SkColor *)img->getPixels();

    for (int i = 0; i < size; i++) {

        SkColor pixel = pixels[i];

        /* Skip alpha pixel and not matching color */
        if (SkColorGetA(pixel) == 0 || SkColorGetA(pixel) != alpha) {
            continue;
        }

        SkScalar hsv[3];
        SkColorToHSV(pixel, hsv);

        hsv[0] = native_min(native_max(SkIntToScalar(val), 0), 360);

        pixels[i] = SkHSVToColor(SkColorGetA(pixel), hsv);
    }

    img->notifyPixelsChanged();
}

void NativeSkImage::markColorsInAlpha()
{
    if (!img) return;

    size_t size = img->getSize() >> img->shiftPerPixel();

    SkColor *pixels = (SkColor *)img->getPixels();
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

    img->notifyPixelsChanged();

}

void NativeSkImage::desaturate()
{
    if (!img) return;

    size_t size = img->getSize() >> img->shiftPerPixel();

    SkColor *pixels = (SkColor *)img->getPixels();
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

    img->notifyPixelsChanged();
}


bool NativeSkImage::ConvertToRGBA(NativeSkImage *nimg, unsigned char* rgba,
        bool flipY, bool premultiply)
{
#if 1
    int length;
    int k;
    const unsigned char *pixels;
    int width;

    nimg->img->lockPixels();
    if ((pixels = static_cast<const unsigned char*>(nimg->img->getPixels())) == NULL) {
        nimg->img->unlockPixels();
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

