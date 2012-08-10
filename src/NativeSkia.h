
#ifndef nativeskia_h__
#define nativeskia_h__

#include <stdint.h>
#include <stdlib.h>

class SkCanvas;
class SkPaint;
class SkPath;
class NativeSkGradient;

typedef uint32_t SkPMColor;
typedef unsigned U8CPU;

class NativeSkia
{
    private:

        SkCanvas *canvas;
        SkPaint *paint;
        SkPaint *paint_stroke;
        SkPaint *paint_system;
        SkPath *currentPath;

        uint8_t globalAlpha;
        uint8_t asComposite;
    public:
        ~NativeSkia();
        int bindGL(int width, int height);
        void flush();
        /* Basics */
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
        void drawImage();
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
