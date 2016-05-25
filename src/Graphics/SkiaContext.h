/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_skiacontext_h__
#define graphics_skiacontext_h__

#include <stdint.h>
#include <stdlib.h>

#include "Graphics/CanvasHandler.h"

class SkCanvas;
class SkPaint;
class SkPath;
class SkBitmap;
class GrContext;
class SkGpuDevice;

namespace Nidium {
namespace Binding {
     class JSDocument;
     class CanvasPattern;
}
namespace Graphics {

class Image;
class Gradient;
class ShadowLooper;

typedef uint32_t SkPMColor;
typedef uint32_t SkColor;
typedef unsigned U8CPU;

// {{{ _Shadow
typedef struct _Shadow
{
    double m_X;
    double m_Y;
    double m_Blur;
    SkColor m_Color;
} Shadow_t;
// }}}

// {{{ _Line and Baseline
struct _Line
{
    const char *m_Line;
    size_t m_Len;
};

enum _Baseline
{
    BASELINE_ALPHABETIC,
    BASELINE_TOP,
    BASELINE_HANGING,
    BASELINE_MIDDLE,
    BASELINE_IDEOGRAPHIC,
    BASELINE_BOTTOM
};
// }}}

// {{{ _State
struct _State {
    SkPaint *m_Paint;
    SkPaint *m_PaintStroke;
    enum _Baseline m_Baseline;

    struct _State *next;
};
// }}}

// {{{ SkiaContext
class SkiaContext
{
    private:
        struct _State *m_State;

        SkPaint *m_PaintSystem;
        SkPath *m_CurrentPath;
        uint8_t m_GlobalAlpha;
        uint8_t m_AsComposite;
        SkBitmap *m_Screen;
        Shadow_t m_CurrentShadow;
        ShadowLooper *buildShadow();
        SkCanvas *m_Canvas;
        void initPaints();
        void addPath(const SkPath& path, SkPath *to);
        bool m_Debug;
        double m_FontSkew;

    public:
        ~SkiaContext();
        SkiaContext();

        enum BindMode {
            BIND_NO,
            BIND_GL,
            BIND_OFFSCREEN,
            BIND_ONSCREEN
        } m_CanvasBindMode;

        friend class CanvasHandler;
        friend class JSCanvas;

        static SkCanvas *m_GlContext;

        SkCanvas *getCanvas() const {
            return m_Canvas;
        }

        static void GetStringColor(uint32_t color, char *out);

        /*
            Assign canvas (hold a ref and unref existing value)
        */
        void setCanvas(SkCanvas *canvas);
        SkGpuDevice *createNewGPUDevice(GrContext *gr, int width, int height);

        double breakText(const char *str, size_t len,
            struct _Line lines[], double maxWidth, int *length = NULL);
        int bindOnScreen(int width, int height);
        static SkCanvas *CreateGLCanvas(int width, int height, Frontend::Context *nctx);
        int bindGL(int width, int height, Frontend::Context *nctx);
        void flush();
        void unlink();

        int readPixels(int top, int left, int width, int height,
            uint8_t *pixels);
        void drawPixelsGL(uint8_t *pixels, int width, int height, int x, int y);
        void drawPixels(uint8_t *pixels, int width, int height, int x, int y);
        void drawText(const char *text, int x, int y, bool stroke = false);
        void drawTextf(int x, int y, const char *text, ...);
        void drawImage(Image *image, double x, double y);
        void drawImage(Image *image, double x, double y,
            double width, double height);
        void drawImage(Image *image,
            int sx, int sy, int swidth, int sheight,
            double dx, double dy, double dwidth, double dheight);
        void drawRect(double x, double y, double width,
            double height, int stroke);
        void drawRect(double x, double y, double width,
            double height, double rx, double ry, int stroke);
        void drawLine(double x1, double y1, double x2, double y2);
        void system(const char *text, int x, int y);

        uint32_t getFillColor() const;
        uint32_t getStrokeColor() const;
        double getLineWidth() const;
        double getMiterLimit() const;
        double getGlobalAlpha() const;

        double getShadowOffsetX() const
        {
            return m_CurrentShadow.m_X;
        }

        double getShadowOffsetY() const
        {
            return m_CurrentShadow.m_Y;
        }

        double getShadowBlur() const
        {
            return m_CurrentShadow.m_Blur;
        }

        uint32_t getShadowColor() const
        {
            return m_CurrentShadow.m_Color;
        }

        const char *getLineCap() const;
        const char *getLineJoin() const;

        int getWidth();
        int getHeight();

        int getSmooth() const;

        void setShadowOffsetX(double x);
        void setShadowOffsetY(double y);
        void setShadowBlur(double blur);
        void setShadowColor(const char *str);
        void setSmooth(bool val, const int level = 1);
        void setFontSize(double size);
        void setFontStyle(const char *style);
        void setFontSkew(double val) {
            m_FontSkew = val;
        }
        void setFillColor(const char *str);
        void setFillColor(Gradient *gradient);
        void setFillColor(Binding::CanvasPattern *pattern);
        void setFillColor(uint32_t color);
        void setStrokeColor(const char *str);
        void setStrokeColor(Gradient *gradient);
        void setStrokeColor(uint32_t color);
        void setLineWidth(double size);
        void setMiterLimit(double size);
        void setGlobalAlpha(double value);
        void setGlobalComposite(const char *str);
        void setLineCap(const char *capStyle);
        void setLineJoin(const char *joinStyle);
        void setFontType(char *str, Binding::JSDocument *doc = NULL);
        bool setFontFile(const char *str);

        void clearRect(double, double, double, double);

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
        void arcTo(int x1, int y1, int x2, int y2, int r);
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
        static uint32_t ParseColor(const char *str);
        static SkPMColor HSLToSKColor(U8CPU alpha, float hsl[3]);
#if 0
        static SkiaContext &GetInstance() {
            static SkiaContext ret;

            return ret;
        }
#endif
};
// }}}

} // namespace Graphics
} // namespace Nidium

#endif

