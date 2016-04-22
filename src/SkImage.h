#ifndef nidium_skimage_h__
#define nidium_skimage_h__

#include <SkBitmap.h>
#include <SkImage.h>

class SkCanvas;
class SkBitmap;
class SkData;

class NativeSkImage
{
  public:
    int m_IsCanvas;
    SkCanvas *m_CanvasRef;
    SkBitmap *m_Image;
#if 0
    SkImage *fixedImg;
#endif
    NativeSkImage(SkCanvas *canvas);
    NativeSkImage(void *data, size_t len);
    NativeSkImage(void *data, int width, int height);


    SkData *getPNG();
    
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

