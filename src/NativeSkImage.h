#ifndef nativeskimage_h__
#define nativeskimage_h__

#include <SkBitmap.h>
#include <SkImage.h>

class SkCanvas;
class SkBitmap;

class NativeSkImage
{
  public:
    int isCanvas;
    SkCanvas *canvasRef;
    SkBitmap *img;
#if 0
    SkImage *fixedImg;
#endif
    NativeSkImage(SkCanvas *canvas);
    NativeSkImage(void *data, size_t len);
    static bool ConvertToRGBA(NativeSkImage *nimg, unsigned char* rgba,
        bool flipY, bool premultiply);

    ~NativeSkImage();

    const uint8_t *getPixels(size_t *len);

    int getWidth();
    int getHeight();
    void shiftHue(int val, U8CPU alpha);
    void markColorsInAlpha();
    void desaturate();
};

#endif

