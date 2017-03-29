/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/SkiaContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <vector>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#include <Shlwapi.h>
#else
#include <unistd.h>
#include <strings.h>
#endif

#define NIDIUM_SKIA_CACHE_MAX_RESOURCES 4096
#define NIDIUM_SKIA_CACHE_MAX_BYTES (256 * 1024 * 1024)

#if defined(__linux__) && !defined(UINT32_MAX)
#define UINT32_MAX 4294967295u
#endif

#include <SkGpuDevice.h>
#include <SkBlurDrawLooper.h>
#include <SkColorFilter.h>
#include <SkParse.h>
#include <SkTypeface.h>
#include <SkLightingImageFilter.h>
#include <SkStream.h>

#include "Interface/SystemInterface.h"

#include "Graphics/Image.h"
#include "Graphics/Gradient.h"
#include "Graphics/ShadowLooper.h"
#include "Binding/JSDocument.h"
#include "Binding/JSCanvas2DContext.h"

using Nidium::Core::PtrAutoDelete;
using Nidium::Core::Path;
using Nidium::Interface::SystemInterface;
using Nidium::IO::Stream;
using Nidium::Binding::JSDocument;
using Nidium::Binding::JSCanvasPattern;
using Nidium::Frontend::Context;

namespace Nidium {
namespace Graphics {

SkSurface *SkiaContext::m_GlSurface = nullptr;


static struct _nidium_blend_mode
{
    const char *str;
    SkBlendMode mode;
} nidium_blend_mode[] = { { "source-over", SkBlendMode::kSrcOver},
                          { "source-in", SkBlendMode::kSrcIn},
                          { "source-out", SkBlendMode::kSrcOut},
                          { "source-atop", SkBlendMode::kSrcATop},
                          { "destination-over", SkBlendMode::kDstOver},
                          { "destination-in", SkBlendMode::kDstIn},
                          { "destination-out", SkBlendMode::kDstOut},
                          { "destination-atop", SkBlendMode::kDstATop},
                          { "lighter", SkBlendMode::kPlus},
                          { "darker", SkBlendMode::kDarken},
                          { "copy", SkBlendMode::kSrc},
                          { "xor", SkBlendMode::kXor},
                          { "lighten", SkBlendMode::kColorDodge},
                          { NULL, SkBlendMode::kSrcOver} };

// {{{ Static funcs and macros
//#define CANVAS_FLUSH() canvas->flush()
#define CANVAS_FLUSH()

/* Current SkPaint (change during a this::save()/restore()) */
#define PAINT m_State->m_Paint
#define PAINT_STROKE m_State->m_PaintStroke

/* TODO: Move this to an util file */
#define WHITESPACE      \
    while (' ' == *str) \
        ++str;

/*
 * Parse color channel value
 */

#define CHANNEL(NAME)                         \
    c = 0;                                    \
    if (*str >= '0' && *str <= '9') {         \
        do {                                  \
            c *= 10;                          \
            c += *str++ - '0';                \
        } while (*str >= '0' && *str <= '9'); \
    } else {                                  \
        return 0;                             \
    }                                         \
    if (c > 255) c = 255;                     \
    NAME = c;                                 \
    while (' ' == *str || ',' == *str)        \
        str++;


static int count_separators(const char *str, const char *sep)
{
    char c;
    int separators = 0;
    while ((c = *str++) != '\0') {
        if (strchr(sep, c) == NULL) continue;
        do {
            if ((c = *str++) == '\0') goto goHome;
        } while (strchr(sep, c) != NULL);
        separators++;
    }
goHome:
    return separators;
}

static inline int32_t argb_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return a << 24 | r << 16 | g << 8 | b;
}

static int32_t rgba_from_rgba_string(const char *str, short *ok)
{
    if (str == strstr(str, "rgba(")) {
        str += 5;
        WHITESPACE;
        uint8_t r = 0, g = 0, b = 0;
        int c;
        float a = 0;
        CHANNEL(r);
        CHANNEL(g);
        CHANNEL(b);
        if (*str >= '1' && *str <= '9') {
            a = 1;
        } else {
            if ('0' == *str) ++str;
            if ('.' == *str) {
                ++str;
                float n = .1;
                while (*str >= '0' && *str <= '9') {
                    a += (*str++ - '0') * n;
                    n *= .1;
                }
            }
        }
        return *ok = 1, argb_from_rgba(r, g, b, static_cast<int>(a * 255));
    }
    return *ok = 0;
}


static int32_t argb_from_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return argb_from_rgba(r, g, b, 255);
}

/*
 * Return rgb from "rgb()"
 */

static int32_t rgba_from_rgb_string(const char *str, short *ok)
{
    if (str == strstr(str, "rgb(")) {
        str += 4;
        WHITESPACE;
        uint8_t r = 0, g = 0, b = 0;
        int c;
        CHANNEL(r);
        CHANNEL(g);
        CHANNEL(b);
        return *ok = 1, argb_from_rgb(r, g, b);
    }
    return *ok = 0;
}


static inline double calcHue(double temp1, double temp2, double hueVal)
{
    if (hueVal < 0.0)
        hueVal++;
    else if (hueVal > 1.0)
        hueVal--;

    if (hueVal * 6.0 < 1.0) return temp1 + (temp2 - temp1) * hueVal * 6.0;
    if (hueVal * 2.0 < 1.0) return temp2;
    if (hueVal * 3.0 < 2.0)
        return temp1 + (temp2 - temp1) * (2.0 / 3.0 - hueVal) * 6.0;

    return temp1;
}

static inline bool isBreakable(const unsigned char c)
{
    return (c == ' ' || c == '.' || c == ',' || c == '-' /*|| c == 0xAD*/);
}

SkiaContext::SkiaContext()
    : m_CanvasBindMode(SkiaContext::BIND_NO), m_State(NULL),
      m_PaintSystem(NULL), m_CurrentPath(NULL), m_GlobalAlpha(0),
      m_AsComposite(0), m_Screen(NULL), m_CurrentShadow({ 0, 0, 0, 0 }),
      m_Debug(false), m_FontSkew(-0.25)
{
}

SkColor
makeRGBAFromHSLA(double hue, double saturation, double lightness, double alpha)
{
    const double scaleFactor = nextafter(256.0, 0.0);

    if (!saturation) {
        int greyValue = static_cast<int>(lightness * scaleFactor);
        return SkColorSetARGB(static_cast<int>(alpha * scaleFactor), greyValue,
                              greyValue, greyValue);
    }

    double temp2 = lightness < 0.5
                       ? lightness * (1.0 + saturation)
                       : lightness + saturation - lightness * saturation;
    double temp1 = 2.0 * lightness - temp2;

    return SkColorSetARGB(
        static_cast<int>(alpha * scaleFactor),
        static_cast<int>(calcHue(temp1, temp2, hue + 1.0 / 3.0) * scaleFactor),
        static_cast<int>(calcHue(temp1, temp2, hue) * scaleFactor),
        static_cast<int>(calcHue(temp1, temp2, hue - 1.0 / 3.0) * scaleFactor));
}

/* TODO: Only accept ints int rgb(a)() */
uint32_t SkiaContext::ParseColor(const char *str)
{
    SkColor color = SK_ColorBLACK;
    /* TODO: use strncasecmp */
    if (str == strstr(str, "rgba")) {
        short ok;
        color = rgba_from_rgba_string(str, &ok);

        if (!ok) color = 0;

    } else if (str == strstr(str, "rgb")) {
        short ok;
        color = rgba_from_rgb_string(str, &ok);

        if (!ok) color = 0;

    } else if (strncasecmp(str, "hsl", 3) == 0) {
        SkScalar array[4];

        int count = count_separators(str, ",") + 1;

        if (count == 4) {
            if (str[3] != 'a') {
                count = 3;
            }
        } else if (count != 3) {
            return 0;
        }
        array[3] = SK_Scalar1;

        const char *end
            = SkParse::FindScalars(&str[(str[3] == 'a' ? 5 : 4)], array, count);

        if (end == NULL) {
            ndm_log(NDM_LOG_WARN, "Canvas", "Couldn't parse color");
        } else {
            /* TODO: limits? */
            return makeRGBAFromHSLA(SkScalarToDouble(array[0]) / 360.,
                                    SkScalarToDouble(array[1]) / 100.,
                                    SkScalarToDouble(array[2]) / 100.,
                                    SkScalarToDouble(array[3]));
        }

    } else {
        SkParse::FindColor(str, &color);
    }

    return color;
}

void SkiaContext::initPaints()
{
    m_State->m_Baseline = BASELINE_ALPHABETIC;

    PAINT = new SkPaint;
    /*
        TODO : setHintingScaleFactor(m_hintingScaleFactor);
        http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/PlatformContextSkia.cpp#363
    */
    memset(&m_CurrentShadow, 0, sizeof(Shadow_t));
    m_CurrentShadow.m_Color = SkColorSetARGB(0, 0, 0, 0);

    PAINT->setARGB(255, 0, 0, 0);
    PAINT->setAntiAlias(true);
    PAINT->setDither(true);
    PAINT->setLCDRenderText(false);

    PAINT->setStyle(SkPaint::kFill_Style);
    PAINT->setFilterQuality(kNone_SkFilterQuality);

    PAINT->setSubpixelText(true);
    PAINT->setAutohinted(true);
    PAINT->setHinting(SkPaint::kFull_Hinting);

    m_PaintSystem = new SkPaint;

    m_PaintSystem->setARGB(255, 255, 0, 0);
    m_PaintSystem->setAntiAlias(true);
    // paint_system->setLCDRenderText(true);
    m_PaintSystem->setStyle(SkPaint::kFill_Style);
    PAINT->setSubpixelText(true);
    PAINT->setAutohinted(true);

    PAINT_STROKE = new SkPaint;

    PAINT_STROKE->setARGB(255, 0, 0, 0);
    PAINT_STROKE->setAntiAlias(true);
    // PAINT_STROKE->setLCDRenderText(true);
    PAINT_STROKE->setStyle(SkPaint::kStroke_Style);
    PAINT_STROKE->setSubpixelText(true);
    PAINT_STROKE->setAutohinted(true);
    PAINT_STROKE->setHinting(SkPaint::kFull_Hinting);
    PAINT_STROKE->setDither(true);
    PAINT_STROKE->setFilterQuality(kNone_SkFilterQuality);

    this->setLineWidth(1);
    this->setMiterLimit(10);

    m_AsComposite = 0;
}

SkiaContext *SkiaContext::CreateWithTextureBackend(Frontend::Context *fctx,
                                                    int width, int height)
{
    float ratio = SystemInterface::GetInstance()->backingStorePixelRatio();

    GrContext *gr = GetGrContext(fctx);

    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(gr, SkBudgeted::kNo,
        SkImageInfo::MakeN32Premul(ceilf(width * ratio), ceilf(height * ratio)));

    if (!surface) {
        return nullptr;
    }

    SkiaContext *skcontext = new SkiaContext();

    skcontext->initWithSurface(surface);

    skcontext->m_CanvasBindMode = SkiaContext::BIND_ONSCREEN;

    return skcontext;
}

SkiaContext *SkiaContext::CreateWithFBOBackend(Frontend::Context *fctx,
                                               int width, int height, uint32_t fbo)
{
    float ratio = SystemInterface::GetInstance()->backingStorePixelRatio();
    GrContext *gr = GetGrContext(fctx);

    GrBackendRenderTargetDesc desc;

    desc.fWidth       = ceilf(width * ratio);
    desc.fHeight      = ceilf(height * ratio);
    desc.fConfig      = kSkia8888_GrPixelConfig;
    desc.fOrigin      = kBottomLeft_GrSurfaceOrigin;
    desc.fStencilBits = 0;
    desc.fSampleCnt   = 0;

    desc.fRenderTargetHandle = fbo;

    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendRenderTarget(gr, desc, nullptr);

    SkiaContext *skcontext = new SkiaContext();

    skcontext->initWithSurface(surface);

    skcontext->m_CanvasBindMode = SkiaContext::BIND_GL;

    return skcontext;
}

bool SkiaContext::initWithSurface(sk_sp<SkSurface> surface)
{
    float ratio = SystemInterface::GetInstance()->backingStorePixelRatio();

    m_Surface = surface;

    scale(ratio, ratio);

    m_GlobalAlpha = 255;
    m_CurrentPath = NULL;

    m_State       = new struct _State;
    m_State->next = NULL;

    initPaints();

    this->setSmooth(true);

    getCanvas()->clear(0x00000000);


    return true;
}

GrContext *SkiaContext::CreateGrContext(GLContext *glcontext)
{
    const GrGLInterface *interface = nullptr;
    GrContext *context             = nullptr;

    if ((interface = glcontext->iface()) == nullptr) {
        ndm_logf(NDM_LOG_ERROR, "SkiaContext", "Can't get OpenGL interface");
        return nullptr;
    }

    context = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)interface);

    if (context == nullptr) {
        return nullptr;
    }

    context->setResourceCacheLimits(NIDIUM_SKIA_CACHE_MAX_RESOURCES,
        NIDIUM_SKIA_CACHE_MAX_BYTES);

    return context;
}

uint32_t SkiaContext::getOpenGLTextureId()
{
    GrGLTextureInfo *glinfo = (GrGLTextureInfo *)m_Surface->getTextureHandle(SkSurface::kFlushRead_BackendHandleAccess);

    if (!glinfo) {
        return 0;
    }

    return glinfo->fID;
}

GrContext *SkiaContext::GetGrContext(Frontend::Context *fctx)
{
    Frontend::LocalContext *lc = Frontend::LocalContext::Get();
    GrContext *gr = lc->getGrContext();

    if (!gr) {
        gr = CreateGrContext(fctx->getGLState()->getNidiumGLContext());
        lc->setGrContext(gr);
    }

    return gr;
}

GrContext *SkiaContext::getGrContext()
{
    return getCanvas()->getGrContext();
}

void SkiaContext::resetGrBackendContext(uint32_t flag)
{
    GrContext *context = getCanvas()->getGrContext();

#ifdef DEBUG
    context->resetContext(kAll_GrBackendState);

    return;
#endif

    if (flag == 0) {
        flag = kProgram_GrGLBackendState | kTextureBinding_GrGLBackendState
               | kVertex_GrGLBackendState | kView_GrGLBackendState
               | kBlend_GrGLBackendState;
    }

    context->resetContext(flag);
}

bool SkiaContext::setSize(int width, int height, bool redraw)
{
    float ratio = Interface::SystemInterface::GetInstance()->backingStorePixelRatio();

    const SkImageInfo &info = SkImageInfo::MakeN32Premul(ceilf(width * ratio),
                                    ceilf(height * ratio));

    // XXX TODO: Resize for fbo

    sk_sp<SkSurface> newSurface = m_Surface->makeSurface(info);

    if (!newSurface) {
        ndm_log(NDM_LOG_ERROR, "Canvas", "Can't create new surface");
        return false;
    }

    newSurface->getCanvas()->clear(0x00000000);

    // Blit the old surface into the new one
    m_Surface->draw(newSurface->getCanvas(), 0, 0, nullptr);

    // Keep the old matrix in place
    newSurface->getCanvas()->setMatrix(m_Surface->getCanvas()->getTotalMatrix());

    m_Surface = newSurface;

    return true;
}


void glcb(const GrGLInterface *)
{
    ndm_log(NDM_LOG_INFO, "Canvas", "Got a gl call");
}


/* TODO: check if there is a best way to do this;
    context->clear() ?
*/
void SkiaContext::clearRect(double x, double y, double width, double height)
{
    /*
    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    paint.setXfermodeMode(SkXfermode::kClear_Mode);
*/
    // CANVAS_FLUSH();
    // glClear(GL_COLOR_BUFFER_BIT);
    SkRect r;
    SkPaint clearPaint;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y), SkDoubleToScalar(width),
              SkDoubleToScalar(height));

    clearPaint.setStyle(SkPaint::kFill_Style);
    clearPaint.setARGB(0, 0, 0, 0);
    clearPaint.setBlendMode(SkBlendMode::kClear);

    getCanvas()->drawRect(r, clearPaint);

    CANVAS_FLUSH();
}

void SkiaContext::system(const char *text, int x, int y)
{
    getCanvas()->drawText(text, strlen(text), SkIntToScalar(x), SkIntToScalar(y),
                       *m_PaintSystem);

    CANVAS_FLUSH();
}

sk_sp<ShadowLooper> SkiaContext::buildShadow()
{
    if (m_CurrentShadow.m_Blur == 0) {
        return nullptr;
    }

    return ShadowLooper::Make(SkDoubleToScalar(m_CurrentShadow.m_Blur),
                                SkDoubleToScalar(m_CurrentShadow.m_X),
                                SkDoubleToScalar(m_CurrentShadow.m_Y),
                                m_CurrentShadow.m_Color,
                                SkBlurDrawLooper::kIgnoreTransform_BlurFlag
                                    | SkBlurDrawLooper::kHighQuality_BlurFlag);
}

void SkiaContext::beginPath()
{
    if (m_CurrentPath) {
        delete m_CurrentPath;
    }

    m_CurrentPath = new SkPath();

    // currentPath->moveTo(SkIntToScalar(0), SkIntToScalar(0));
}

/* TODO: bug? looks like we need to add to the previous value (strange) */
void SkiaContext::moveTo(double x, double y)
{
    if (!m_CurrentPath) {
        beginPath();
    }
    const SkMatrix &m = getCanvas()->getTotalMatrix();
    SkPoint pt        = SkPoint::Make(SkDoubleToScalar(x), SkDoubleToScalar(y));

    SkMatrix::MapPtsProc proc = m.getMapPtsProc();

    proc(m, &pt, &pt, 1);

    m_CurrentPath->moveTo(pt);
}

void SkiaContext::lineTo(double x, double y)
{
    /* moveTo is set? */
    if (!m_CurrentPath) {
        beginPath();
    }

    const SkMatrix &m = getCanvas()->getTotalMatrix();
    SkPoint pt        = SkPoint::Make(SkDoubleToScalar(x), SkDoubleToScalar(y));

    SkMatrix::MapPtsProc proc = m.getMapPtsProc();

    proc(m, &pt, &pt, 1);

    if (!m_CurrentPath->countPoints()) {
        m_CurrentPath->moveTo(pt);
    } else {
        m_CurrentPath->lineTo(pt);
    }
}

void SkiaContext::fill()
{
    if (!m_CurrentPath) {
        return;
    }

    SkCanvas *canvas = getCanvas();

    sk_sp<SkShader> shader = PAINT->refShader();

    if (shader.get() != nullptr) {
        sk_sp<SkShader> tmpShader = shader->makeWithLocalMatrix(canvas->getTotalMatrix());
        PAINT->setShader(tmpShader);
    }
    /* The matrix was already applied point by point */
    canvas->save();
    canvas->resetMatrix();
    canvas->drawPath(*m_CurrentPath, *PAINT);
    canvas->restore();

    if (shader.get() != nullptr) {
        PAINT->setShader(shader);
    }

    CANVAS_FLUSH();
}

void SkiaContext::stroke()
{
    if (!m_CurrentPath) {
        return;
    }

    SkCanvas *canvas = getCanvas();

    sk_sp<SkShader> shader = PAINT_STROKE->refShader();

    if (shader.get() != nullptr) {
        sk_sp<SkShader> tmpShader = shader->makeWithLocalMatrix(canvas->getTotalMatrix());
        PAINT_STROKE->setShader(tmpShader);
    }

    /* The matrix was already applied point by point */
    canvas->save();
    canvas->resetMatrix();

    SkScalar lineWidth = PAINT_STROKE->getStrokeWidth();
    float ratio        = SystemInterface::GetInstance()->backingStorePixelRatio();

    PAINT_STROKE->setStrokeWidth(SkFloatToScalar(ratio) * lineWidth);

    canvas->drawPath(*m_CurrentPath, *PAINT_STROKE);
    PAINT_STROKE->setStrokeWidth(lineWidth);

    canvas->restore();

    if (shader.get() != nullptr) {
        PAINT_STROKE->setShader(shader);
    }

    CANVAS_FLUSH();
}

void SkiaContext::closePath()
{
    if (!m_CurrentPath) {
        return;
    }

    m_CurrentPath->close();
}

void SkiaContext::clip()
{
    if (!m_CurrentPath) {
        return;
    }

    getCanvas()->clipPath(*m_CurrentPath);
    CANVAS_FLUSH();
}

/*
void SkPath::addPath(const SkPath& path, const SkMatrix& matrix) {
    SkPathRef::Editor(&fPathRef, path.countVerbs(), path.countPoints());

    fIsOval = false;

    RawIter iter(path);
    SkPoint pts[4];
    Verb    verb;

    SkMatrix::MapPtsProc proc = matrix.getMapPtsProc();

    while ((verb = iter.next(pts)) != kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                proc(matrix, &pts[0], &pts[0], 1);
                this->moveTo(pts[0]);
                break;
            case kLine_Verb:
                proc(matrix, &pts[1], &pts[1], 1);
                this->lineTo(pts[1]);
                break;
            case kQuad_Verb:
                proc(matrix, &pts[1], &pts[1], 2);
                this->quadTo(pts[1], pts[2]);
                break;
            case kCubic_Verb:
                proc(matrix, &pts[1], &pts[1], 3);
                this->cubicTo(pts[1], pts[2], pts[3]);
                break;
            case kClose_Verb:
                this->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
    }
}

*/

void SkiaContext::rect(double x, double y, double width, double height)
{
    if (!m_CurrentPath) {
        beginPath();
    }
    SkMatrix m = getCanvas()->getTotalMatrix();

    SkRect r
        = SkRect::MakeXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
                           SkDoubleToScalar(width), SkDoubleToScalar(height));

    SkPath tmpPath;

    tmpPath.addRect(r);
    tmpPath.transform(m);
    m_CurrentPath->addPath(tmpPath);
}

void SkiaContext::addPath(const SkPath &path, SkPath *to)
{
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    int i = 0;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb: {
                if (i == 0) break;
                to->moveTo(pts[0]);
                break;
            }
            case SkPath::kLine_Verb:
                to->lineTo(pts[1]);
                break;
            case SkPath::kQuad_Verb:
                to->quadTo(pts[1], pts[2]);
                break;
            case SkPath::kConic_Verb:
                to->conicTo(pts[1], pts[2], iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                to->cubicTo(pts[1], pts[2], pts[3]);
                break;
            case SkPath::kClose_Verb:
                to->close();
                break;
            default:
                break;
        }
        i++;
    }
}

void SkiaContext::arc(
    int x, int y, int r, double startAngle, double endAngle, int CCW)
{
    if (!m_CurrentPath || (!startAngle && !endAngle) || !r) {
        return;
    }

    double sweep = endAngle - startAngle;
    SkMatrix m   = getCanvas()->getTotalMatrix();

    SkScalar cx     = SkIntToScalar(x);
    SkScalar cy     = SkIntToScalar(y);
    SkScalar s360   = SkIntToScalar(360);
    SkScalar radius = SkIntToScalar(r);

    SkScalar start = SkDoubleToScalar(180 * startAngle / SK_ScalarPI);
    SkScalar end   = SkDoubleToScalar(180 * sweep / SK_ScalarPI);

    SkRect rect;
    rect.set(cx - radius, cy - radius, cx + radius, cy + radius);

    SkPath tmppath;
    bool dropfirst = false;

    if (!m_CurrentPath->isEmpty()) {
        SkPoint lp;
        m_CurrentPath->getLastPt(&lp);
        tmppath.moveTo(lp);
        dropfirst = true;
    }

    if (end >= s360 || end <= -s360) {
        // Move to the start position (0 sweep means we add a single point).
        tmppath.arcTo(rect, start, 0, false);
        // Draw the circle.
        tmppath.addOval(rect);
        // Force a moveTo the end position.
        tmppath.arcTo(rect, start + end, 0, true);
    } else {
        if (CCW && end > 0) {
            end -= s360;
        } else if (!CCW && end < 0) {
            end += s360;
        }
        tmppath.arcTo(rect, start, end, false);
    }

    /* TODO: do the transform in addPath */
    tmppath.transform(m);
    if (dropfirst) {
        this->addPath(tmppath, m_CurrentPath);
    } else {
        m_CurrentPath->addPath(tmppath);
    }
}

void SkiaContext::arcTo(int x1, int y1, int x2, int y2, int r)
{
    if (!r) {
        return;
    }
    if (!m_CurrentPath) {
        beginPath();
    }

    SkScalar cx1    = SkIntToScalar(x1);
    SkScalar cy1    = SkIntToScalar(y1);
    SkScalar cx2    = SkIntToScalar(x2);
    SkScalar cy2    = SkIntToScalar(y2);
    SkScalar radius = SkIntToScalar(r);

    m_CurrentPath->arcTo(cx1, cy1, cx2, cy2, radius);
}

void SkiaContext::quadraticCurveTo(double cpx, double cpy, double x, double y)
{
    if (!m_CurrentPath) {
        return;
    }

    SkMatrix m = getCanvas()->getTotalMatrix();

    if (!m_CurrentPath->countPoints()) {
        m_CurrentPath->moveTo(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy));
    }

    SkPoint cp, p;

    m.mapXY(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy), &cp);
    m.mapXY(SkDoubleToScalar(x), SkDoubleToScalar(y), &p);

    m_CurrentPath->quadTo(cp, p);
}

void SkiaContext::bezierCurveTo(
    double cpx, double cpy, double cpx2, double cpy2, double x, double y)
{
    if (!m_CurrentPath) {
        return;
    }

    if (!m_CurrentPath->countPoints()) {
        m_CurrentPath->moveTo(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy));
    }


    SkMatrix m = getCanvas()->getTotalMatrix();
    SkPoint p1, p2, p3;

    m.mapXY(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy), &p1);
    m.mapXY(SkDoubleToScalar(cpx2), SkDoubleToScalar(cpy2), &p2);
    m.mapXY(SkDoubleToScalar(x), SkDoubleToScalar(y), &p3);

    m_CurrentPath->cubicTo(p1, p2, p3);
}

void SkiaContext::rotate(double angle)
{
    getCanvas()->rotate(SkDoubleToScalar(180 * angle / SK_ScalarPI));
}

void SkiaContext::scale(double x, double y)
{
    getCanvas()->scale(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void SkiaContext::translate(double x, double y)
{
    getCanvas()->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void SkiaContext::save()
{
    struct _State *nstate = new struct _State;

    nstate->m_Paint       = new SkPaint(*PAINT);
    nstate->m_PaintStroke = new SkPaint(*PAINT_STROKE);
    nstate->next          = m_State;
    nstate->m_Baseline    = BASELINE_ALPHABETIC;

    m_State = nstate;

    getCanvas()->save();
}

void SkiaContext::restore()
{
    if (m_State->next) {
        struct _State *dstate = m_State->next;
        delete m_State->m_Paint;
        delete m_State->m_PaintStroke;
        delete m_State;

        m_State = dstate;
    } else {
        ndm_logf(NDM_LOG_ERROR, "SkiaContext", "restore() without matching save()");
    }

    getCanvas()->restore();
}

void SkiaContext::skew(double x, double y)
{
    getCanvas()->skew(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

/*
    pointInPath :
    http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/SkiaUtils.cpp#115
*/
bool SkiaContext::SkPathContainsPoint(double x, double y)
{
    if (m_CurrentPath == NULL) {
        return false;
    }

    SkRect bounds       = m_CurrentPath->getBounds();
    SkPath::FillType ft = SkPath::kWinding_FillType;

    // We can immediately return false if the point is outside the bounding
    // rect.  We don't use bounds.contains() here, since it would exclude
    // points on the right and bottom edges of the bounding rect, and we want
    // to include them.
    SkScalar fX = SkDoubleToScalar(x);
    SkScalar fY = SkDoubleToScalar(y);
    if (fX < bounds.fLeft || fX > bounds.fRight || fY < bounds.fTop
        || fY > bounds.fBottom)
        return false;

    // Scale the path to a large size before hit testing for two reasons:
    // 1) Skia has trouble with coordinates close to the max signed 16-bit
    // values, so we scale
    // larger paths down.
    //    TODO: when Skia is patched to work properly with large values, this
    //    will not be necessary.
    // 2) Skia does not support analytic hit testing, so we scale paths up to do
    // raster hit testing
    // with subpixel accuracy.

    SkScalar biggestCoord = nidium_max(
        nidium_max(nidium_max(bounds.fRight, bounds.fBottom), -bounds.fLeft),
        -bounds.fTop);

    if (SkScalarNearlyZero(biggestCoord)) return false;

    biggestCoord = nidium_max(nidium_max(biggestCoord, fX + 1), fY + 1);

    const SkScalar kMaxCoordinate = SkIntToScalar(1 << 15);
    SkScalar scale                = kMaxCoordinate / biggestCoord;

    SkRegion rgn;
    SkRegion clip;
    SkMatrix m;
    SkPath scaledPath;

    SkPath::FillType originalFillType = m_CurrentPath->getFillType();
    m_CurrentPath->setFillType(ft);

    m.setScale(scale, scale);
    m_CurrentPath->transform(m, &scaledPath);

    int ix = static_cast<int>(floor(0.5 + x * scale));
    int iy = static_cast<int>(floor(0.5 + y * scale));
    clip.setRect(ix - 1, iy - 1, ix + 1, iy + 1);

    bool contains = rgn.setPath(scaledPath, clip);
    m_CurrentPath->setFillType(originalFillType);

    return contains;
}

void SkiaContext::itransform(double scalex,
                             double skewy,
                             double skewx,
                             double scaley,
                             double translatex,
                             double translatey)
{
    SkMatrix m;

    m.setAll(SkDoubleToScalar(scalex), SkDoubleToScalar(skewx),
             SkDoubleToScalar(translatex), SkDoubleToScalar(skewy),
             SkDoubleToScalar(scaley), SkDoubleToScalar(translatey), 0, 0, 0);

    SkMatrix im;
    if (m.invert(&im)) {
        ndm_log(NDM_LOG_DEBUG, "Canvas", "transformed");
        getCanvas()->concat(im);
    } else {
        ndm_log(NDM_LOG_ERROR, "Canvas", "Can't revert Matrix");
    }
}

void SkiaContext::transform(double scalex,
                            double skewy,
                            double skewx,
                            double scaley,
                            double translatex,
                            double translatey,
                            int set)
{
    SkMatrix m;

    float ratio = 1.0f;
    if (set) {
        ratio = SystemInterface::GetInstance()->backingStorePixelRatio();
    }

    m.setScaleX(SkDoubleToScalar(scalex * ratio));
    m.setSkewX(SkDoubleToScalar(skewx * ratio));
    m.setTranslateX(SkDoubleToScalar(translatex * ratio));

    m.setScaleY(SkDoubleToScalar(scaley * ratio));
    m.setSkewY(SkDoubleToScalar(skewy * ratio));
    m.setTranslateY(SkDoubleToScalar(translatey * ratio));

    m.setPerspX(0);
    m.setPerspY(0);

    m.set(SkMatrix::kMPersp2, SK_Scalar1);

    if (set) {
        getCanvas()->setMatrix(m);
    } else {
        getCanvas()->concat(m);
    }
}

int SkiaContext::readPixels(
    int top, int left, int width, int height, uint8_t *pixels)
{
    const SkImageInfo &info = SkImageInfo::Make(
        width, height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);

    if (!getCanvas()->readPixels(info, pixels, width * 4, left, top)) {
        ndm_log(NDM_LOG_DEBUG, "Canvas", "Failed to read pixels");
        return 0;
    }

    return 1;
}
// }}}

// {{{ Some Getters
void SkiaContext::GetStringColor(uint32_t color, char *out)
{
    /*
        Mimic Chrome and Firefox :

        Whenver we have some alpha, a literal rgba() string is
        returned instead of a literal #RRGGBB
    */
    if (SkColorGetA(color) != 0xff) {

        sprintf(out, "rgba(%d, %d, %d, %.2f)", SkColorGetR(color),
                SkColorGetG(color), SkColorGetB(color),
                SkColorGetA(color) / 255.f);
    } else {
        sprintf(out, "#%.2x%.2x%.2x", SkColorGetR(color), SkColorGetG(color),
                SkColorGetB(color));
    }
}

int SkiaContext::getWidth()
{
    return getCanvas()->getDeviceSize().fWidth;
}

int SkiaContext::getHeight()
{
    return getCanvas()->getDeviceSize().fHeight;
}

uint32_t SkiaContext::getFillColor() const
{
    return PAINT->getColor();
}

uint32_t SkiaContext::getStrokeColor() const
{
    return PAINT_STROKE->getColor();
}

int SkiaContext::getSmooth() const
{
    return (int)PAINT->getFilterQuality();
}

double SkiaContext::getGlobalAlpha() const
{
    return (double)m_GlobalAlpha / (double)255;
}

double SkiaContext::getLineWidth() const
{
    return SkScalarToDouble(PAINT_STROKE->getStrokeWidth());
}

void SkiaContext::setMiterLimit(double size)
{
    PAINT_STROKE->setStrokeMiter(SkDoubleToScalar(size));
}

double SkiaContext::getMiterLimit() const
{
    return SkScalarToDouble(PAINT_STROKE->getStrokeMiter());
}

/*
    composite :
    http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/SkiaUtils.cpp
*/
void SkiaContext::getPathBounds(double *left,
                                double *right,
                                double *top,
                                double *bottom)
{
    if (m_CurrentPath == NULL) {
        return;
    }
    SkRect bounds = m_CurrentPath->getBounds();

    *left   = SkScalarToDouble(bounds.fLeft);
    *right  = SkScalarToDouble(bounds.fRight);
    *top    = SkScalarToDouble(bounds.fTop);
    *bottom = SkScalarToDouble(bounds.fBottom);
}

const char *SkiaContext::getLineCap() const
{
    switch (PAINT_STROKE->getStrokeCap()) {
        case SkPaint::kRound_Cap:
            return "round";
        case SkPaint::kSquare_Cap:
            return "square";
        case SkPaint::kButt_Cap:
        default:
            return "butt";
    }
}

const char *SkiaContext::getLineJoin() const
{
    switch (PAINT_STROKE->getStrokeJoin()) {
        case SkPaint::kRound_Join:
            return "round";
        case SkPaint::kBevel_Join:
            return "bevel";
        case SkPaint::kMiter_Join:
        default:
            return "miter";
    }
}


// }}}
// {{{ Draw
void SkiaContext::drawRect(
    double x, double y, double width, double height, int stroke)
{
    SkRect r;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y), SkDoubleToScalar(width),
              SkDoubleToScalar(height));

    getCanvas()->drawRect(r, (stroke ? *PAINT_STROKE : *PAINT));

    CANVAS_FLUSH();
}

void SkiaContext::drawLine(double x1, double y1, double x2, double y2)
{
    getCanvas()->drawLine(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                       SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                       *PAINT_STROKE);
}

void SkiaContext::drawRect(double x,
                           double y,
                           double width,
                           double height,
                           double rx,
                           double ry,
                           int stroke)
{
    SkRect r;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y), SkDoubleToScalar(width),
              SkDoubleToScalar(height));

    getCanvas()->drawRoundRect(r, SkDoubleToScalar(rx), SkDoubleToScalar(ry),
                            (stroke ? *PAINT_STROKE : *PAINT));
}

void SkiaContext::drawImage(Image *image, double x, double y)
{
    SkColor old = PAINT->getColor();
    PAINT->setColor(SK_ColorBLACK);

    if (image->m_IsCanvas) {
        ndm_log(NDM_LOG_WARN, "Canvas", "Not implemented");
        #if 0
        getCanvas()->drawBitmap(
            image->m_CanvasRef->getDevice()->accessBitmap(false),
            SkDoubleToScalar(x), SkDoubleToScalar(y), PAINT);
        #endif

    } else if (image->m_Image != NULL) {
        getCanvas()->drawImage(image->m_Image, SkDoubleToScalar(x),
                             SkDoubleToScalar(y), PAINT);
    }

    PAINT->setColor(old);

    /* TODO: clear read'd pixel? */
    CANVAS_FLUSH();
}

void SkiaContext::drawImage(
    Image *image, double x, double y, double width, double height)
{
    SkRect r;
    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y), SkDoubleToScalar(width),
              SkDoubleToScalar(height));

    SkColor old = PAINT->getColor();
    PAINT->setColor(SK_ColorBLACK);

    if (image->m_IsCanvas) {
        ndm_log(NDM_LOG_WARN, "Canvas", "Not implemented");
        #if 0
        getCanvas()->drawBitmapRect(
            image->m_CanvasRef->getDevice()->accessBitmap(false), NULL, r,
            PAINT);
        #endif
    } else if (image->m_Image != NULL) {
        getCanvas()->drawImageRect(image->m_Image, r, PAINT);
    }

    PAINT->setColor(old);

    CANVAS_FLUSH();
}

void SkiaContext::drawImage(Image *image,
                            int sx,
                            int sy,
                            int swidth,
                            int sheight,
                            double dx,
                            double dy,
                            double dwidth,
                            double dheight)
{
    SkRect dst;
    SkIRect src;

    SkColor old = PAINT->getColor();
    /* DrawImage must not takes the paint alpha */
    PAINT->setColor(SK_ColorBLACK);
    /* TODO: ->readPixels : switch to accessBitmap; */
    src.setXYWH(sx, sy, swidth, sheight);

    dst.setXYWH(SkDoubleToScalar(dx), SkDoubleToScalar(dy),
                SkDoubleToScalar(dwidth), SkDoubleToScalar(dheight));

    if (image->m_IsCanvas) {
        ndm_log(NDM_LOG_WARN, "Canvas", "Not implemented");
#if 0
        SkBitmap bitmapImage;

        image->m_CanvasRef->readPixels(src, &bitmapImage);
        bitmapImage.setIsVolatile(true);

        getCanvas()->drawBitmapRect(bitmapImage, NULL, dst, PAINT);
#endif
    } else if (image->m_Image != NULL) {
        getCanvas()->drawImageRect(image->m_Image, src, dst, PAINT);
    }

    PAINT->setColor(old);

    CANVAS_FLUSH();
}

void SkiaContext::redrawScreen()
{
    SkCanvas *canvas = getCanvas();

    canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()),
                         m_Screen);
    canvas->writePixels(*m_Screen, 0, 0);
    CANVAS_FLUSH();
}

void SkiaContext::drawPixels(
    uint8_t *pixels, int width, int height, int x, int y)
{

    getCanvas()->writePixels(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                       kUnpremul_SkAlphaType), pixels, width * 4, x, y);
}

void SkiaContext::flush()
{
    getCanvas()->flush();
}
// }}}

// {{{ Some Setters
void SkiaContext::setGlobalAlpha(double value)
{
    if (value < 0) return;

    SkScalar maxuint            = SkIntToScalar(255);
    m_GlobalAlpha               = SkMinScalar(SkDoubleToScalar(value) * maxuint, maxuint);
    sk_sp<SkColorFilter> filter = SkColorFilter::MakeModeFilter(
        SkColorSetARGB(m_GlobalAlpha, 255, 255, 255),
        SkBlendMode::kModulate);

    PAINT->setColorFilter(filter);
    PAINT_STROKE->setColorFilter(filter);
}

void SkiaContext::setFontSize(double size)
{
    SkScalar ssize = SkDoubleToScalar(size);
    PAINT->setTextSize(ssize);
    PAINT_STROKE->setTextSize(ssize);
}

void SkiaContext::setFontStyle(const char *style)
{
    PAINT->setFakeBoldText((strcasestr(style, "bold")));
    PAINT->setUnderlineText((strcasestr(style, "underline")));
    PAINT->setStrikeThruText((strcasestr(style, "strike")));

    PAINT->setTextSkewX(strcasestr(style, "italic") ? m_FontSkew : 0);
}

void SkiaContext::setFontType(const char *str, JSDocument *doc)
{
    if (doc) {
        sk_sp<SkTypeface> tf = doc->getFont(str);
        if (tf) {
            PAINT->setTypeface(tf);
            PAINT_STROKE->setTypeface(tf);

            return;
        }
    }
    // JSDocument *jdoc = JSDocument::
    sk_sp<SkTypeface> tf = SkTypeface::MakeFromName(str, SkFontStyle());
    // Workarround for skia bug #1648
    // https://code.google.com/p/skia/issues/detail?id=1648
    if (tf == NULL) {
        tf = SkTypeface::MakeFromName(NULL, SkFontStyle());
        if (tf == NULL) return;
    }

    PAINT->setTypeface(tf);
    PAINT_STROKE->setTypeface(tf);
}

bool SkiaContext::setFontFile(const char *str)
{
    char *data;
    size_t len;

    Path fontPath(str);
    Stream *stream;

    if ((stream = fontPath.CreateStream(true)) == NULL) {
        return false;
    }

    PtrAutoDelete<Stream *> npad(stream);

    if (!stream->getContentSync(&data, &len)) {
        return false;
    }

    SkMemoryStream *skmemory = new SkMemoryStream(data, len, true);
    free(data);

    sk_sp<SkTypeface> tf = SkTypeface::MakeFromStream(skmemory);
    if (tf == NULL) {
        delete skmemory;
        return false;
    }

    PAINT->setTypeface(tf);
    PAINT_STROKE->setTypeface(tf);

    return true;
}

void SkiaContext::setFillColor(JSCanvasPattern *pattern)
{
    if (pattern->m_JsImg->getImage()->m_Image != NULL) {
        sk_sp<SkShader> shader;

        bool repeat_x = false, repeat_y = false;

        switch (pattern->m_Mode) {
            case JSCanvasPattern::PATTERN_REPEAT_MIRROR:
            case JSCanvasPattern::PATTERN_REPEAT:
                repeat_x = repeat_y = true;
                break;
            case JSCanvasPattern::PATTERN_REPEAT_X:
                repeat_x = true;
                break;
            case JSCanvasPattern::PATTERN_REPEAT_Y:
                repeat_y = true;
                break;
            default:
                break;
        }

        if (repeat_x && repeat_y) {
            shader = SkShader::MakeBitmapShader(
                *pattern->m_JsImg->getImage()->getBitmap(),
                pattern->m_Mode == JSCanvasPattern::PATTERN_REPEAT_MIRROR
                    ? SkShader::kMirror_TileMode
                    : SkShader::kRepeat_TileMode,
                pattern->m_Mode == JSCanvasPattern::PATTERN_REPEAT_MIRROR
                    ? SkShader::kMirror_TileMode
                    : SkShader::kRepeat_TileMode);
        } else {
            SkShader::TileMode tileModeX = repeat_x ? SkShader::kRepeat_TileMode
                                                    : SkShader::kClamp_TileMode;
            SkShader::TileMode tileModeY = repeat_y ? SkShader::kRepeat_TileMode
                                                    : SkShader::kClamp_TileMode;

            int expandW = repeat_x ? 0 : 1;
            int expandH = repeat_y ? 0 : 1;

            sk_sp<SkImage> bm = pattern->m_JsImg->getImage()->m_Image;

            sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul
                (bm->width() + expandW, bm->height() + expandH);

            surface->getCanvas()->clear(SK_ColorTRANSPARENT);

            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kSrc);

            surface->getCanvas()->drawImage(bm.get(), 0, 0, &paint);

            sk_sp<SkImage> expandedImage = surface->makeImageSnapshot();

            shader = expandedImage->makeShader(tileModeX, tileModeY);
        }

        PAINT->setColor(SK_ColorBLACK);
        PAINT->setShader(shader);
    }
}

void SkiaContext::setFillColor(Gradient *gradient)
{
    sk_sp<SkShader> shader;

    if ((shader = gradient->build()) == NULL) {
        /* Make paint invalid (no future draw) */
        // paint->setShader(NULL);
        ndm_logf(NDM_LOG_ERROR, "SkiaContext", "Invalid gradient");
        return;
    }
    PAINT->setColor(SK_ColorBLACK);
    PAINT->setShader(shader);
    // ndm_printf("Add gradient : %p (%d)", shader, shader->getRefCnt());
}

void SkiaContext::setFillColor(const char *str)
{
    SkColor color = ParseColor(str);

    PAINT->setShader(nullptr);
    PAINT->setColor(color);
}

void SkiaContext::setFillColor(uint32_t color)
{
    PAINT->setShader(nullptr);
    PAINT->setColor(color);
}

void SkiaContext::setStrokeColor(const char *str)
{
    SkColor color = ParseColor(str);

    PAINT_STROKE->setShader(nullptr);
    PAINT_STROKE->setColor(color);
}

void SkiaContext::setStrokeColor(Gradient *gradient)
{
    sk_sp<SkShader> shader;

    if ((shader = gradient->build()) == NULL) {
        return;
    }
    PAINT_STROKE->setColor(SK_ColorBLACK);
    PAINT_STROKE->setShader(shader);
}


void SkiaContext::setStrokeColor(uint32_t color)
{

    PAINT_STROKE->setShader(nullptr);
    PAINT_STROKE->setColor(color);
}

void SkiaContext::setShadowOffsetX(double x)
{
    if (m_CurrentShadow.m_X == x) return;
    m_CurrentShadow.m_X = x;
    PAINT->setLooper(buildShadow());
}

void SkiaContext::setShadowOffsetY(double y)
{
    if (m_CurrentShadow.m_Y == y) return;
    m_CurrentShadow.m_Y = y;
    PAINT->setLooper(buildShadow());
}

void SkiaContext::setShadowBlur(double blur)
{
    if (m_CurrentShadow.m_Blur == blur) return;
    m_CurrentShadow.m_Blur = blur;

    PAINT->setLooper(buildShadow());
}

void SkiaContext::setShadowColor(const char *str)
{
    SkColor color = ParseColor(str);

    if (m_CurrentShadow.m_Color == color) return;
    m_CurrentShadow.m_Color = color;

    PAINT->setLooper(buildShadow());
}

void SkiaContext::setSmooth(bool val, int quality)
{
    SkFilterQuality fquality = kNone_SkFilterQuality;

    if (val) {
        switch (quality) {
            case 0:
                fquality = kNone_SkFilterQuality;
                break;
            case 1:
                fquality = kLow_SkFilterQuality;
                break;
            case 2:
                fquality = kMedium_SkFilterQuality;
                break;
            case 3:
            default:
                fquality = kHigh_SkFilterQuality;
                break;
        }
    }

    PAINT->setFilterQuality(fquality);
    PAINT_STROKE->setFilterQuality(fquality);
}

void SkiaContext::setGlobalComposite(const char *str)
{
    for (int i = 0; nidium_blend_mode[i].str != NULL; i++) {
        if (strcasecmp(nidium_blend_mode[i].str, str) == 0) {
            SkBlendMode mode = nidium_blend_mode[i].mode;
            PAINT->setBlendMode(mode);
            PAINT_STROKE->setBlendMode(mode);
            break;
        }
    }

    m_AsComposite = 1;
}

void SkiaContext::setLineWidth(double size)
{
    PAINT_STROKE->setStrokeWidth(SkDoubleToScalar(size));
}

void SkiaContext::setLineCap(const char *capStyle)
{
    if (strcasecmp(capStyle, "round") == 0) {
        PAINT_STROKE->setStrokeCap(SkPaint::kRound_Cap);
    } else if (strcasecmp(capStyle, "square") == 0) {
        PAINT_STROKE->setStrokeCap(SkPaint::kSquare_Cap);
    } else {
        PAINT_STROKE->setStrokeCap(SkPaint::kButt_Cap);
    }
}

void SkiaContext::setLineJoin(const char *joinStyle)
{
    if (strcasecmp(joinStyle, "round") == 0) {
        PAINT_STROKE->setStrokeJoin(SkPaint::kRound_Join);
    } else if (strcasecmp(joinStyle, "bevel") == 0) {
        PAINT_STROKE->setStrokeJoin(SkPaint::kBevel_Join);
    } else {
        PAINT_STROKE->setStrokeJoin(SkPaint::kMiter_Join);
    }
}

// }}}

// {{{ Text
/* TODO: bug with alpha */
void SkiaContext::drawText(const char *text, int x, int y, bool stroke)
{
    SkPaint::FontMetrics metrics;
    PAINT->getFontMetrics(&metrics);

    SkScalar sx = SkIntToScalar(x), sy = SkIntToScalar(y);

    switch (m_State->m_Baseline) {
        case BASELINE_TOP:
            sy -= metrics.fTop;
            break;
        case BASELINE_BOTTOM:
            sy -= metrics.fBottom;
            break;
        case BASELINE_MIDDLE:
            /* TODO: remove hack */
            sy += ((metrics.fXHeight) / 2)
                  + ((metrics.fXHeight) / 2) * 13. / 100.;
            break;
        default:
            break;
    }

    getCanvas()->drawText(text, strlen(text), sx, sy,
                       (stroke ? *PAINT_STROKE : *PAINT));

    CANVAS_FLUSH();
}

void SkiaContext::textBaseline(const char *mode)
{
    if (strcasecmp("top", mode) == 0) {
        m_State->m_Baseline = BASELINE_TOP;
    } else if (strcasecmp("hanging", mode) == 0) {
        m_State->m_Baseline = BASELINE_ALPHABETIC;
    } else if (strcasecmp("middle", mode) == 0) {
        m_State->m_Baseline = BASELINE_MIDDLE;
    } else if (strcasecmp("ideographic", mode) == 0) {
        m_State->m_Baseline = BASELINE_ALPHABETIC;
    } else if (strcasecmp("bottom", mode) == 0) {
        m_State->m_Baseline = BASELINE_BOTTOM;
    } else {
        m_State->m_Baseline = BASELINE_ALPHABETIC;
    }
}

void SkiaContext::textAlign(const char *mode)
{
    if (strcasecmp("left", mode) == 0 || strcasecmp("start", mode) == 0) {
        PAINT->setTextAlign(SkPaint::kLeft_Align);
        PAINT_STROKE->setTextAlign(SkPaint::kLeft_Align);

    } else if (strcasecmp("center", mode) == 0) {
        PAINT->setTextAlign(SkPaint::kCenter_Align);
        PAINT_STROKE->setTextAlign(SkPaint::kLeft_Align);

    } else if (strcasecmp("right", mode) == 0 || strcasecmp("end", mode) == 0) {
        PAINT->setTextAlign(SkPaint::kRight_Align);
        PAINT_STROKE->setTextAlign(SkPaint::kRight_Align);
    }
}

void SkiaContext::drawTextf(int x, int y, const char text[], ...)
{
    static const size_t BUFFER_SIZE = 4096;

    char buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, BUFFER_SIZE, text, args);
    va_end(args);

    drawText(buffer, x, y);
}

double SkiaContext::breakText(const char *str,
                              size_t len,
                              struct _Line lines[],
                              double maxWidth,
                              int *length)
{
    struct
    {
        SkScalar curWordWidth;
        SkScalar curLineWidth;
        size_t curWordLen;
        const char *ptr;
        int curLine;
    } curState;

    int i;

    std::vector<SkScalar> widths(len);

    PAINT->getTextWidths(str, len, &widths[0]);
    curState.ptr          = str;
    curState.curWordWidth = SkIntToScalar(0);
    curState.curLineWidth = SkIntToScalar(0);
    curState.curWordLen   = 0;
    curState.curLine      = 0;

    lines[0].m_Line = str;
    lines[0].m_Len  = 0;

    for (i = 0; i < len; i++) {

        lines[curState.curLine].m_Len++;

        if (isBreakable((unsigned char)str[i])) {
            curState.ptr          = &str[i + 1];
            curState.curWordWidth = SkIntToScalar(0);
            curState.curWordLen = 0;
            if (curState.ptr == NULL) {
                break;
            }
        } else {
            curState.curWordWidth += widths[i];
            curState.curWordLen++;
        }

        curState.curLineWidth += widths[i];

        if (curState.curLineWidth > maxWidth) {
            lines[curState.curLine].m_Len
                = curState.ptr - lines[curState.curLine].m_Line;
            curState.curLine++;

            lines[curState.curLine].m_Line = curState.ptr;
            lines[curState.curLine].m_Len  = 0;
            curState.curLineWidth          = curState.curWordWidth;
        }
    }

    lines[curState.curLine].m_Len = &str[i] - lines[curState.curLine].m_Line;
    if (length) {
        *length = curState.curLine + 1;
    }
    return (curState.curLine + 1) * PAINT->getFontSpacing();
}

double SkiaContext::measureText(const char *str, size_t length)
{
    return SkScalarToDouble(PAINT->measureText(str, length));
}
// }}}

SkiaContext::~SkiaContext()
{
    struct _State *nstate = m_State;

    if (m_Surface != nullptr) {
        getCanvas()->flush();
    }
    while (nstate) {
        struct _State *tmp = nstate->next;
        // ndm_printf("Delete pain %p with shader : %p", nstate->paint,
        // nstate->paint->getShader());
        delete nstate->m_Paint;
        delete nstate->m_PaintStroke;
        delete nstate;
        nstate = tmp;
    }

    if (m_PaintSystem) delete m_PaintSystem;
    if (m_CurrentPath) delete m_CurrentPath;
}


} // namespace Graphics
} // namespace Nidium
