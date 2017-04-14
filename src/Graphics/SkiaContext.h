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
#include "Graphics/ShadowLooper.h"

#include <SkSurface.h>

class SkCanvas;
class SkPaint;
class SkPath;
class SkBitmap;
class GrContext;
class SkGpuDevice;

typedef uint32_t SkPMColor;
typedef uint32_t SkColor;
typedef unsigned U8CPU;

namespace Nidium {
namespace Binding {
class JSDocument;
class JSCanvasPattern;
}
namespace Graphics {

class Image;
class Gradient;
class ShadowLooper;
class GLContext;


typedef struct _Shadow
{
    double m_X;
    double m_Y;
    double m_Blur;
    SkColor m_Color;
} Shadow_t;

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

struct _State
{
    SkPaint *m_Paint;
    SkPaint *m_PaintStroke;
    enum _Baseline m_Baseline;

    struct _State *next;
};

class SkiaContext
{

public:
    /*
        Create a new SkiaContext backed by an OpenGL texture.

        A new texture is also created
    */
    static SkiaContext *CreateWithTextureBackend(Frontend::Context *fctx,
                                                        int width, int height);

    /*
        Create a new SkiaContext backed by a Frame Buffer Object.

        The framebuffer must be provided.
        0 being the window framebuffer.
    */
    static SkiaContext *CreateWithFBOBackend(Frontend::Context *fctx,
                                               int width, int height,
                                               uint32_t fbo = 0);

    /*
        Returns the underlying OpenGL texture id
    */
    uint32_t getOpenGLTextureId();

    /*
        Change the underlying canvas size.
        This internally recreate a new SkSurface.

        Reference to old SkSurface should be updated

        Size are logical pixels units
    */
    bool setSize(int width, int height, bool redraw = true);

    /*
        Ask Skia to reset its internal GL state in case we have altered it
    */
    void resetGrBackendContext(uint32_t flag = 0);


    static uint32_t ParseColor(const char *str);
    static void GetStringColor(uint32_t color, char *out);


    SkCanvas *getCanvas() const
    {
        return m_Surface->getCanvas();
    }

    sk_sp<SkSurface> getSurface()
    {
        return m_Surface;
    }

    double breakText(const char *str, size_t len, struct _Line lines[],
                     double maxWidth,
                     int *length = NULL);

    int bindOnScreen(int width, int height);
    static sk_sp<SkSurface>
        CreateGLSurface(int width, int height, Frontend::Context *nctx);
    int bindGL(int width, int height, Frontend::Context *nctx);
    void flush();
    void unlink();

    int readPixels(int top, int left, int width, int height, uint8_t *pixels);
    void drawPixels(uint8_t *pixels, int width, int height, int x, int y);
    void drawText(const char *text, int x, int y, bool stroke = false);
    void drawTextf(int x, int y, const char *text, ...);
    void drawImage(Image *image, double x, double y);
    void drawImage(Image *image, double x, double y, double width, double height);
    void drawImage(Image *image, int sx, int sy, int swidth, int sheight,
                double dx, double dy, double dwidth, double dheight);
    void drawRect(double x, double y, double width, double height, int stroke);
    void drawRect(double x, double y, double width, double height,
        double rx, double ry, int stroke);
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
    void setSmooth(bool val, const int quality = 2);
    void setFontSize(double size);
    void setFontStyle(const char *style);
    void setFontSkew(double val) {
        m_FontSkew = val;
    }
    void setFillColor(const char *str);
    void setFillColor(Gradient *gradient);
    void setFillColor(Binding::JSCanvasPattern *pattern);
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
    void setFontType(const char *str, Binding::JSDocument *doc = NULL);
    bool setFontFile(const char *str);

    void clearRect(double, double, double, double);

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
    void arcTo(int x1, int y1, int x2, int y2, int r);
    void quadraticCurveTo(double cpx, double cpy, double x, double y);
    void bezierCurveTo(
        double cpx, double cpy, double cpx2, double cpy2, double x, double y);
    void rotate(double angle);
    void scale(double x, double y);
    void translate(double x, double y);
    void skew(double x, double y);
    void transform(double scalex,
                   double skewx,
                   double skewy,
                   double scaley,
                   double translatex,
                   double translatey,
                   int set);
    void save();
    void restore();
    void redrawScreen();
    void itransform(double scalex,
                    double skewy,
                    double skewx,
                    double scaley,
                    double translatex,
                    double translatey);
    double measureText(const char *str, size_t length);
    bool SkPathContainsPoint(double x, double y);
    void
    getPathBounds(double *left, double *right, double *top, double *bottom);
    void textAlign(const char *mode);
    void textBaseline(const char *mode);

    enum BindMode
    {
        BIND_NO,
        BIND_GL,
        BIND_OFFSCREEN,
        BIND_ONSCREEN
    } m_CanvasBindMode;

    ~SkiaContext();

    friend class CanvasHandler;
    friend class JSCanvas;

    static SkSurface *m_GlSurface;


private:
    SkiaContext();

    sk_sp<SkSurface> createNewGPUSurface(GrContext *gr, int width, int height);
    static GrContext *CreateGrContext(GLContext *glcontext);

    void initPaints();
    void addPath(const SkPath &path, SkPath *to);
    
    sk_sp<ShadowLooper> buildShadow();

    bool initWithSurface(sk_sp<SkSurface> surface);

    /*
        Get Skia GrContext.
        It's lazy created if it's not yet created
    */
    static GrContext *GetGrContext(Frontend::Context *fctx);

    GrContext *getGrContext();

    struct _State *m_State;
    SkPaint *m_PaintSystem;
    SkPath *m_CurrentPath;
    uint8_t m_GlobalAlpha;
    uint8_t m_AsComposite;
    SkBitmap *m_Screen;
    Shadow_t m_CurrentShadow;
    sk_sp<SkSurface> m_Surface;
    bool m_Debug;
    double m_FontSkew;

};
// }}}

} // namespace Graphics
} // namespace Nidium

#endif
