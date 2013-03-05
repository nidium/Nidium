#ifndef nativeskimage_h__
#define nativeskimage_h__

#include "SkBitmap.h"
#include "SkImage.h"
/*
drawImage(R,9,2,282,148,0,3,300,150)
*/
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

    int getWidth();
    int getHeight();
    void shiftHue(int val, U8CPU alpha);
    void markColorsInAlpha();
    void desaturate();
};

#endif
