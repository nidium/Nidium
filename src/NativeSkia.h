#ifndef nativeskia_h__
#define nativeskia_h__

#include <stdint.h>
#include <stdlib.h>
#include "NativeCanvasHandler.h"

class SkCanvas;
class SkPaint;
class SkPath;
class NativeSkGradient;
class NativeSkImage;
class SkBitmap;
class NativeShadowLooper;
class GrContext;
class NativeCanvasPattern;
class SkGpuDevice;

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

struct _NativeLine
{
    const char *line;
    size_t len;
};

enum _NativeBaseline
{
    BASELINE_ALPHABETIC,
    BASELINE_TOP,
    BASELINE_HANGING,
    BASELINE_MIDDLE,
    BASELINE_IDEOGRAPHIC,
    BASELINE_BOTTOM
};

struct _nativeState {
    SkPaint *paint;
    SkPaint *paint_stroke;
    enum _NativeBaseline baseline;

    struct _nativeState *next;    
};

class NativeSkia
{
    private:

        struct _nativeState *state;

        SkPaint *paint_system;
        SkPath *currentPath;

        uint8_t globalAlpha;
        uint8_t asComposite;
        SkBitmap *screen;
        NativeShadow_t currentShadow;
        NativeShadowLooper *buildShadow();

        SkCanvas *m_Canvas;

        void initPaints();
        void addPath(const SkPath& path, SkPath *to);

        bool m_Debug;
        
    public:
        ~NativeSkia();
        NativeSkia();

        enum BindMode {
            BIND_NO,
            BIND_GL,
            BIND_OFFSCREEN,
            BIND_ONSCREEN
        } native_canvas_bind_mode;

        friend class NativeCanvasHandler;
        friend class NativeJSCanvas;

        static SkCanvas *glcontext;

        SkCanvas *getCanvas() const {
            return m_Canvas;
        }

        /*
            Assign canvas (hold a ref and unref existing value)
        */
        void setCanvas(SkCanvas *canvas);
        SkGpuDevice *createNewGPUDevice(GrContext *gr, int width, int height);

        double breakText(const char *str, size_t len,
            struct _NativeLine lines[], double maxWidth, int *length = NULL);
        int bindOnScreen(int width, int height);
        static SkCanvas *createGLCanvas(int width, int height);
        int bindGL(int width, int height);
        void flush();
        void unlink();
        /* Basics */
        int readPixels(int top, int left, int width, int height,
            uint8_t *pixels);
        void drawPixelsGL(uint8_t *pixels, int width, int height, int x, int y);
        void drawPixels(uint8_t *pixels, int width, int height, int x, int y);
        void setShadowOffsetX(double x);
        void setShadowOffsetY(double y);
        void setShadowBlur(double blur);
        void setShadowColor(const char *str);
        void setSmooth(bool val, int level = 1);
        void setFontSize(double size);
        void setFontType(const char *str);
        void setFontFile(const char *str);
        void drawText(const char *text, int x, int y);
        void drawTextf(int x, int y, const char *text, ...);
        void system(const char *text, int x, int y);
        void drawRect(double x, double y, double width, 
            double height, int stroke);
        void drawRect(double x, double y, double width,
            double height, double rx, double ry, int stroke);
        void drawLine(double x1, double y1, double x2, double y2);
        void setFillColor(const char *str);
        void setFillColor(NativeSkGradient *gradient);
        void setFillColor(NativeCanvasPattern *pattern);
        void setFillColor(uint32_t color);
        void setStrokeColor(const char *str);
        void setStrokeColor(NativeSkGradient *gradient);
        void setStrokeColor(uint32_t color);
        void setLineWidth(double size);
        void setGlobalAlpha(double value);
        void setGlobalComposite(const char *str);
        void clearRect(double, double, double, double);
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
        void light(double x, double y, double z);
        void fill();
        void stroke();
        void closePath();
        void clip();
        void rect(double x, double y, double width, double height);
        void arc(int, int, int, double, double, int);
        void quadraticCurveTo(double cpx, double cpy, double x, double y);
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
        void itransform(double scalex, double skewy, double skewx,
            double scaley, double translatex, double translatey);
        double measureText(const char *str, size_t length);
        bool SkPathContainsPoint(double x, double y);
        void getPathBounds(double *left, double *right,
            double *top, double *bottom);
        void textAlign(const char *mode);
        void textBaseline(const char *mode);
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
