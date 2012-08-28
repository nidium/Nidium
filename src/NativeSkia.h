
#ifndef nativeskia_h__
#define nativeskia_h__

#include <stdint.h>
#include <stdlib.h>

class SkCanvas;
class SkPaint;
class SkPath;
class NativeSkGradient;
class NativeSkImage;
class SkBitmap;
class NativeShadowLooper;

typedef uint32_t SkPMColor;
typedef uint32_t SkColor;
typedef unsigned U8CPU;

typedef struct _NativeShadow
{
    double x;
    double y;
    double blur;

    SkColor color;

} NativeShadow_t;

class NativeSkia
{
    private:

        
        SkPaint *paint;
        SkPaint *paint_stroke;
        SkPaint *paint_system;
        SkPath *currentPath;

        uint8_t globalAlpha;
        uint8_t asComposite;
        SkBitmap *screen;
        NativeShadow_t currentShadow;
        NativeShadowLooper *buildShadow();
    public:
        SkCanvas *canvas;
        ~NativeSkia();
        int bindGL(int width, int height);
        void flush();
        /* Basics */
        void setShadowOffsetX(double x);
        void setShadowOffsetY(double y);
        void setShadowBlur(double blur);
        void setShadowColor(const char *str);
        void setShadow();
        void setFontSize(double size);
        void setFontType(const char *str);
        void drawText(const char *text, int x, int y);
        void system(const char *text, int x, int y);
        void drawRect(double, double, double, double, double);
        void setFillColor(const char *str);
        void setFillColor(NativeSkGradient *gradient);
        void setStrokeColor(const char *str);
        void setStrokeColor(NativeSkGradient *gradient);
        void setLineWidth(double size);
        void setGlobalAlpha(double value);
        void setGlobalComposite(const char *str);
        void clearRect(int, int, int, int);
        void drawImage(NativeSkImage *image, double x, double y);
        void drawImage(NativeSkImage *image, double x, double y,
            double width, double height);
        void drawImage(NativeSkImage *image,
            int sx, int sy, int swidth, int sheight,
            double dx, double dy, double dwidth, double dheight);
        int getWidth();
        int getHeight();

        void setLineCap(const char *capStyle);
        void setLineJoin(const char *joinStyle);
        /* Shapes */
        void beginPath();
        void moveTo(double x, double y);
        void lineTo(double x, double y);
        void fill();
        void stroke();
        void closePath();
        void clip();
        void rect(double x, double y, double width, double height);
        void arc(int, int, int, double, double, int);
        void quadraticCurveTo(int cpx, int cpy, int x, int y);
        void bezierCurveTo(double cpx, double cpy, double cpx2,
            double cpy2, double x, double y);
        void rotate(double angle);
        void scale(double x, double y);
        void translate(double x, double y);
        void skew(double x, double y);
        void transform(double scalex, double skewx, double skewy,
            double scaley, double translatex, double translatey, int set);
        void save();
        void restore();
        void redrawScreen();
        double measureText(const char *str, size_t length);
        static uint32_t parseColor(const char *str);
        static SkPMColor HSLToSKColor(U8CPU alpha, float hsl[3]);
#if 0
        static NativeSkia &getInstance() {
            static NativeSkia ret;

            return ret;
        }
#endif
};


#endif
