#ifdef __linux__
   #define UINT32_MAX 4294967295u
#endif

#include <NativeOpenGLHeader.h>

#include <jsapi.h>
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeCanvas2DContext.h"
#include "NativeSystemInterface.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGpuDevice.h"
#include "SkBlurDrawLooper.h"
#include "SkColorFilter.h"
#include "SkConfig8888.h"
//#include "SkGLCanvas.h"

#include "SkParse.h"

#include "GrContext.h"
#include "SkTypeface.h"

#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"

#include "GrGLRenderTarget.h"

#include "gl/SkNativeGLContext.h"

#include "SkGraphics.h"
#include "SkXfermode.h"

#include "NativeShadowLooper.h"
#include "SkBlurMaskFilter.h"
#include "SkBlurImageFilter.h"
#include "SkLightingImageFilter.h"
#include "NativeMacros.h"

#include "Sk2DPathEffect.h"

#include <NativeContext.h>

#include <NativePath.h>
#include <NativeJSDocument.h>
#include <SkStream.h>
#include <SkImageInfo.h>

SkCanvas *NativeSkia::glcontext = NULL;

//#define CANVAS_FLUSH() canvas->flush()
#define CANVAS_FLUSH()

/* Current SkPaint (change during a this::save()/restore()) */
#define PAINT state->paint
#define PAINT_STROKE state->paint_stroke

/* TODO: Move this to an util file */
#define WHITESPACE \
  while (' ' == *str) ++str;

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

/*
 * Parse color channel value
 */

#define CHANNEL(NAME) \
   c = 0; \
   if (*str >= '0' && *str <= '9') { \
     do { \
       c *= 10; \
       c += *str++ - '0'; \
     } while (*str >= '0' && *str <= '9'); \
   } else { \
     return 0; \
   } \
   if (c > 255) c = 255; \
   NAME = c; \
   while (' ' == *str || ',' == *str) str++;


static int count_separators(const char* str, const char* sep) {
    char c;
    int separators = 0;
    while ((c = *str++) != '\0') {
        if (strchr(sep, c) == NULL)
            continue;
        do {
            if ((c = *str++) == '\0')
            goto goHome;
        } while (strchr(sep, c) != NULL);
        separators++;
    }
    goHome:
    return separators;
}

static inline int32_t
argb_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return a << 24
    | r << 16
    | g << 8
    | b;
}

static int32_t
rgba_from_rgba_string(const char *str, short *ok) {
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
    return *ok = 1, argb_from_rgba(r, g, b, (int) (a * 255));
  }
  return *ok = 0;
}


static int32_t
argb_from_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return argb_from_rgba(r, g, b, 255);
}

/*
 * Return rgb from "rgb()"
 */

static int32_t
rgba_from_rgb_string(const char *str, short *ok) {
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


static inline double calcHue(double temp1, double temp2, double hueVal) {
  if (hueVal < 0.0)
    hueVal++;
  else if (hueVal > 1.0)
    hueVal--;

  if (hueVal * 6.0 < 1.0)
    return temp1 + (temp2 - temp1) * hueVal * 6.0;
  if (hueVal * 2.0 < 1.0)
    return temp2;
  if (hueVal * 3.0 < 2.0)
    return temp1 + (temp2 - temp1) * (2.0 / 3.0 - hueVal) * 6.0;

  return temp1;
}


int NativeSkia::getWidth()
{
    return m_Canvas->getDeviceSize().fWidth;
}

int NativeSkia::getHeight()
{
    return m_Canvas->getDeviceSize().fHeight;
}

#if 0
static U8CPU InvScaleByte(U8CPU component, uint32_t scale)
{
    SkASSERT(component == (uint8_t)component);
    return (component * scale + 0x8000) >> 16;
}


static SkColor SkPMColorToColor(SkPMColor pm)
{
    if (!pm)
        return 0;
    unsigned a = SkGetPackedA32(pm);
    if (!a) {
        // A zero alpha value when there are non-zero R, G, or B channels is an
        // invalid premultiplied color (since all channels should have been
        // multiplied by 0 if a=0).
        SkASSERT(false);
        // In production, return 0 to protect against division by zero.
        return 0;
    }
   
    uint32_t scale = (255 << 16) / a;
   
    return SkColorSetARGB(a,
                          InvScaleByte(SkGetPackedR32(pm), scale),
                          InvScaleByte(SkGetPackedG32(pm), scale),
                          InvScaleByte(SkGetPackedB32(pm), scale));
}
#endif

SkColor makeRGBAFromHSLA(double hue, double saturation, double lightness, double alpha)
{
    const double scaleFactor = nextafter(256.0, 0.0);

    if (!saturation) {
        int greyValue = static_cast<int>(lightness * scaleFactor);
        return SkColorSetARGB(static_cast<int>(alpha * scaleFactor),
            greyValue, greyValue, greyValue);
    }

    double temp2 = lightness < 0.5 ? lightness * (1.0 + saturation) : lightness + saturation - lightness * saturation;
    double temp1 = 2.0 * lightness - temp2;
    
    return SkColorSetARGB(static_cast<int>(alpha * scaleFactor),
                    static_cast<int>(calcHue(temp1, temp2, hue + 1.0 / 3.0) * scaleFactor), 
                    static_cast<int>(calcHue(temp1, temp2, hue) * scaleFactor),
                    static_cast<int>(calcHue(temp1, temp2, hue - 1.0 / 3.0) * scaleFactor));
}

/* TODO: Only accept ints int rgb(a)() */
uint32_t NativeSkia::parseColor(const char *str)
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

        const char* end = SkParse::FindScalars(&str[(str[3] == 'a' ? 5 : 4)],
            array, count);

        if (end == NULL) printf("Not found\n");
        else {
            /* TODO: limits? */
            return makeRGBAFromHSLA(SkScalarToDouble(array[0])/360.,
                SkScalarToDouble(array[1])/100.,
                SkScalarToDouble(array[2])/100.,
                SkScalarToDouble(array[3]));
        }

    } else {
        SkParse::FindColor(str, &color);
    }

    return color;
}

void NativeSkia::initPaints()
{
    state->baseline = BASELINE_ALPHABETIC;

    PAINT = new SkPaint;
    /*
        TODO : setHintingScaleFactor(m_hintingScaleFactor);
        http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/PlatformContextSkia.cpp#363
    */
    memset(&currentShadow, 0, sizeof(NativeShadow_t));
    currentShadow.color = SkColorSetARGB(255, 0, 0, 0);

    PAINT->setARGB(255, 0, 0, 0);
    PAINT->setAntiAlias(true);
    PAINT->setDither(true);
    PAINT->setLCDRenderText(false);

    PAINT->setStyle(SkPaint::kFill_Style);
    PAINT->setFilterLevel(SkPaint::kNone_FilterLevel);
 
    PAINT->setSubpixelText(true);
    PAINT->setAutohinted(true);
    PAINT->setHinting(SkPaint::kFull_Hinting);

    paint_system = new SkPaint;

    paint_system->setARGB(255, 255, 0, 0);
    paint_system->setAntiAlias(true);
    //paint_system->setLCDRenderText(true);
    paint_system->setStyle(SkPaint::kFill_Style);
    PAINT->setSubpixelText(true);
    PAINT->setAutohinted(true);
   
    PAINT_STROKE = new SkPaint;

    PAINT_STROKE->setARGB(255, 0, 0, 0);
    PAINT_STROKE->setAntiAlias(true);
    //PAINT_STROKE->setLCDRenderText(true);
    PAINT_STROKE->setStyle(SkPaint::kStroke_Style);
    PAINT_STROKE->setSubpixelText(true);
    PAINT_STROKE->setAutohinted(true);
    PAINT_STROKE->setHinting(SkPaint::kFull_Hinting);
    PAINT_STROKE->setDither(true);
    PAINT_STROKE->setFilterLevel(SkPaint::kNone_FilterLevel);
    
    this->setLineWidth(1);
    this->setMiterLimit(10);

    asComposite = 0;
}

SkGpuDevice *NativeSkia::createNewGPUDevice(GrContext *gr, int width, int height)
{
    GrTextureDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fSampleCnt = 0;
    GrTexture *tex = gr->createUncachedTexture(desc, NULL, 0);
    
    return SkGpuDevice::Create(tex);
}

int NativeSkia::bindOnScreen(int width, int height)
{
    if (NativeSkia::glcontext == NULL) {
        printf("Cant find GL context\n");
        return 0;
    }

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

#if 0
    SkBaseDevice *dev = NativeSkia::glcontext
                        ->createCompatibleDevice(SkBitmap::kARGB_8888_Config,
                            width*ratio, height*ratio, false);
#else
    GrContext *gr = ((SkGpuDevice *)NativeSkia::glcontext->getDevice())->context();
    SkBaseDevice *dev = this->createNewGPUDevice(gr, width*ratio, height*ratio);
#endif
    if (dev == NULL) {
        return 0;
    }
    
    m_Canvas = new SkCanvas(dev);
    this->scale(ratio, ratio);

    dev->unref();

    globalAlpha = 255;
    currentPath = NULL;

    state = new struct _nativeState;
    state->next = NULL;

    initPaints();

    this->setSmooth(true);

    m_Canvas->clear(0x00000000);

    this->native_canvas_bind_mode = NativeSkia::BIND_ONSCREEN;

    return 1;
}


void glcb(const GrGLInterface*) {
    printf("Got a gl call\n");
}

SkCanvas *NativeSkia::createGLCanvas(int width, int height,
    NativeContext *nativectx)
{

    if (!nativectx) {
        NLOG("createGLCanvas() : invalid native context");
        return NULL;
    }

    const GrGLInterface *interface = NULL;
    GrContext *context = NULL;

    if (NativeSkia::glcontext) {
        context = ((SkGpuDevice *)NativeSkia::glcontext->getDevice())->context();
        context->ref();
    } else {

        if ((interface = nativectx->getGLState()->getNativeGLContext()->iface()) == NULL) {
            NLOG("Cant get OpenGL interface");
            return NULL;
        }

        context = GrContext::Create(kOpenGL_GrBackend,
            (GrBackendContext)interface);

        if (context == NULL) {
            return NULL;
        }

    }
    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();
    
    GrBackendRenderTargetDesc desc;
    //GrGLRenderTarget *t = new GrGLRenderTarget();

    desc.fWidth = SkScalarRoundToInt(width*ratio);
    desc.fHeight = SkScalarRoundToInt(height*ratio);
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fStencilBits = 0;
    desc.fSampleCnt = 0;
    //GR_GL_GetIntegerv(interface, GR_GL_STENCIL_BITS, &desc.fStencilBits);
#if 0
    GrGLint buffer = 0;
    GR_GL_GetIntegerv(interface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
#endif
    desc.fRenderTargetHandle = 0;
    GrRenderTarget * target = context->wrapBackendRenderTarget(desc);

    if (target == NULL) {
        NLOG("Failed to init Skia render target");
        return NULL;
    }
    SkGpuDevice *dev = new SkGpuDevice(context, target);

    if (dev == NULL) {
        NLOG("Failed to init Skia GPU device");
        return NULL;
    }
    SkCanvas *ret;
    ret = new SkCanvas(dev);
    ret->clear(0x00000000);
    dev->unref();
    context->unref();

    return ret;

}

int NativeSkia::bindGL(int width, int height, NativeContext *nativectx)
{
    this->native_canvas_bind_mode = NativeSkia::BIND_GL;

    if ((m_Canvas = NativeSkia::createGLCanvas(width, height, nativectx)) == NULL) {
        return 0;
    }

    if (NativeSkia::glcontext == NULL) {
        NativeSkia::glcontext = m_Canvas;
    }

    globalAlpha = 255;
    currentPath = NULL;

    state = new struct _nativeState;
    state->next = NULL;

    initPaints();

    /*
        TODO. (dirty hack)
        Skia bug? If we don't draw something first, clear does nothing.
    */
    this->drawRect(0, 0, 1, 1, 0);

    m_Canvas->clear(0xFFFFFFFF);

    m_Debug = true;

    return 1;
}

void NativeSkia::drawRect(double x, double y, double width,
    double height, int stroke)
{
    SkRect r;
    
    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));

    m_Canvas->drawRect(r, (stroke ? *PAINT_STROKE : *PAINT));

    CANVAS_FLUSH();

}

void NativeSkia::drawLine(double x1, double y1, double x2, double y2)
{
    m_Canvas->drawLine(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
        SkDoubleToScalar(x2), SkDoubleToScalar(y2), *PAINT_STROKE);
}

void NativeSkia::drawRect(double x, double y, double width,
    double height, double rx, double ry, int stroke)
{
    SkRect r;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));

    m_Canvas->drawRoundRect(r, SkDoubleToScalar(rx), SkDoubleToScalar(ry),
        (stroke ? *PAINT_STROKE : *PAINT));
}

NativeSkia::NativeSkia() :
    m_Canvas(NULL),
    native_canvas_bind_mode(NativeSkia::BIND_NO),
    state(NULL), paint_system(NULL), currentPath(NULL),
    m_Debug(false), m_FontSkew(-0.25)
{

}

NativeSkia::~NativeSkia()
{
    struct _nativeState *nstate = state;

    if (m_Canvas != NULL) {
        m_Canvas->flush();
    }
    while (nstate) {
        struct _nativeState *tmp = nstate->next;
        //NLOG("Delete pain %p with shader : %p", nstate->paint, nstate->paint->getShader());
        delete nstate->paint;
        delete nstate->paint_stroke;
        delete nstate;
        nstate = tmp;
    }

    if (paint_system) delete paint_system;
    if (currentPath) delete currentPath;

    SkSafeUnref(m_Canvas);
}

/* TODO: check if there is a best way to do this;
    context->clear() ?
*/
void NativeSkia::clearRect(double x, double y, double width, double height)
{
/*
    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    paint.setXfermodeMode(SkXfermode::kClear_Mode);
*/
    //CANVAS_FLUSH();
    //glClear(GL_COLOR_BUFFER_BIT);
    SkRect r;
    SkPaint clearPaint;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));
    
    clearPaint.setStyle(SkPaint::kFill_Style);
    clearPaint.setARGB(0,0,0,0);
    clearPaint.setXfermodeMode(SkXfermode::kClear_Mode);

    m_Canvas->drawRect(r, clearPaint);

    CANVAS_FLUSH();

}

void NativeSkia::setFontSize(double size)
{
    SkScalar ssize = SkDoubleToScalar(size);
    PAINT->setTextSize(ssize);
    PAINT_STROKE->setTextSize(ssize);
}

void NativeSkia::setFontStyle(const char *style)
{
    PAINT->setFakeBoldText((strcasestr(style, "bold")));
    PAINT->setUnderlineText((strcasestr(style, "underline")));
    PAINT->setStrikeThruText((strcasestr(style, "strike")));

    PAINT->setTextSkewX(strcasestr(style, "italic") ? m_FontSkew : 0);
}

void NativeSkia::setFontType(char *str, NativeJSdocument *doc)
{
    if (doc) {
        SkTypeface *tf = doc->getFont(str);
        if (tf) {
            PAINT->setTypeface(tf);
            PAINT_STROKE->setTypeface(tf);

            return;
        }
    }
    //NativeJSdocument *jdoc = NativeJSdocument::
    SkTypeface *tf = SkTypeface::CreateFromName(str,
        SkTypeface::kNormal);
    // Workarround for skia bug #1648
    // https://code.google.com/p/skia/issues/detail?id=1648
    if (tf == NULL) {
        tf = SkTypeface::CreateFromName(NULL, 
                SkTypeface::kNormal);
        if (tf == NULL) return;
    }

    PAINT->setTypeface(tf);
    PAINT_STROKE->setTypeface(tf);

    tf->unref();
}

bool NativeSkia::setFontFile(const char *str)
{
    char *data;
    size_t len;

    NativePath fontPath(str);
    NativeBaseStream *stream;

    if ((stream = fontPath.createStream(true)) == NULL) {
        return false;
    }

    NativePtrAutoDelete<NativeBaseStream *> npad(stream);

    if (!stream->getContentSync(&data, &len)) {
        return false;
    }

    SkMemoryStream *skmemory = new SkMemoryStream(data, len, true);
    free(data);

    SkTypeface *tf = SkTypeface::CreateFromStream(skmemory);
    if (tf == NULL) {
        delete skmemory;
        return false;
    }

    PAINT->setTypeface(tf);
    PAINT_STROKE->setTypeface(tf);

    tf->unref();

    return true;
}

/* TODO: bug with alpha */
void NativeSkia::drawText(const char *text, int x, int y, bool stroke)
{
    SkPaint::FontMetrics metrics;
    PAINT->getFontMetrics(&metrics);

    SkScalar sx = SkIntToScalar(x), sy = SkIntToScalar(y);

    switch(state->baseline) {
        case BASELINE_TOP:
            sy -= metrics.fTop;
            break;
        case BASELINE_BOTTOM:
            sy -= metrics.fBottom;
            break;
        case BASELINE_MIDDLE:
        /* TODO: remove hack */
            sy += ((metrics.fXHeight)/2) + ((metrics.fXHeight)/2)*13./100.;
            break;
        default:
            break;
    }

    m_Canvas->drawText(text, strlen(text),
        sx, sy, (stroke ? *PAINT_STROKE : *PAINT));

    CANVAS_FLUSH();
}

void NativeSkia::textBaseline(const char *mode)
{
    if (strcasecmp("top", mode) == 0) {
        state->baseline = BASELINE_TOP;
    } else if (strcasecmp("hanging", mode) == 0) {
        state->baseline = BASELINE_ALPHABETIC;
    } else if (strcasecmp("middle", mode) == 0) {
        state->baseline = BASELINE_MIDDLE;
    } else if (strcasecmp("ideographic", mode) == 0) {
        state->baseline = BASELINE_ALPHABETIC;
    } else if (strcasecmp("bottom", mode) == 0) {
        state->baseline = BASELINE_BOTTOM;
    } else {
        state->baseline = BASELINE_ALPHABETIC;
    }
}

void NativeSkia::textAlign(const char *mode)
{
    if (strcasecmp("left", mode) == 0) {
        PAINT->setTextAlign(SkPaint::kLeft_Align);
        PAINT_STROKE->setTextAlign(SkPaint::kLeft_Align);

    } else if (strcasecmp("center", mode) == 0) {
        PAINT->setTextAlign(SkPaint::kCenter_Align);
        PAINT_STROKE->setTextAlign(SkPaint::kLeft_Align);

    } else if (strcasecmp("right", mode) == 0) {
        PAINT->setTextAlign(SkPaint::kRight_Align);
        PAINT_STROKE->setTextAlign(SkPaint::kRight_Align);
    }
}

void NativeSkia::drawTextf(int x, int y, const char text[], ...)
{
    static const size_t BUFFER_SIZE = 4096;

    char    buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, BUFFER_SIZE, text, args);
    va_end(args);

    drawText(buffer, x, y);
}

void NativeSkia::system(const char *text, int x, int y)
{
    m_Canvas->drawText(text, strlen(text),
        SkIntToScalar(x), SkIntToScalar(y), *paint_system);

    CANVAS_FLUSH();
}

void NativeSkia::setFillColor(NativeCanvasPattern *pattern)
{ 
    SkShader *shader = NULL;

    if (pattern->jsimg->img->img != NULL) {
        bool repeat_x = false, repeat_y = false;

        switch(pattern->mode) {
            case NativeCanvasPattern::PATTERN_REPEAT_MIRROR:
            case NativeCanvasPattern::PATTERN_REPEAT:
                repeat_x = repeat_y = true;
                break;
            case NativeCanvasPattern::PATTERN_REPEAT_X:
                repeat_x = true;
                break;
            case NativeCanvasPattern::PATTERN_REPEAT_Y:
                repeat_y = true;
                break;
            default:
                break;
        }

        if (repeat_x && repeat_y) {
            shader = SkShader::CreateBitmapShader(*pattern->jsimg->img->img,
                pattern->mode == NativeCanvasPattern::PATTERN_REPEAT_MIRROR ?
                    SkShader::kMirror_TileMode : SkShader::kRepeat_TileMode,
                pattern->mode == NativeCanvasPattern::PATTERN_REPEAT_MIRROR ?
                    SkShader::kMirror_TileMode : SkShader::kRepeat_TileMode);            
        } else {
            SkShader::TileMode tileModeX = repeat_x ? SkShader::kRepeat_TileMode : SkShader::kClamp_TileMode;
            SkShader::TileMode tileModeY = repeat_y ? SkShader::kRepeat_TileMode : SkShader::kClamp_TileMode;

            int expandW = repeat_x ? 0 : 1;
            int expandH = repeat_y ? 0 : 1;

            SkBitmap *bm = pattern->jsimg->img->img;
            SkBitmap bm2;

            bm2.setConfig(bm->config(), bm->width() + expandW, bm->height() + expandH);
            bm2.allocPixels();
            bm2.eraseARGB(0x00, 0x00, 0x00, 0x00);

            SkCanvas canvas(bm2);
            canvas.drawBitmap(*bm, 0, 0);

            shader = SkShader::CreateBitmapShader(bm2, tileModeX, tileModeY);

        }

        PAINT->setColor(SK_ColorBLACK);
        PAINT->setShader(shader);
    }
}

void NativeSkia::setFillColor(NativeSkGradient *gradient)
{ 
    SkShader *shader;

    if ((shader = gradient->build()) == NULL) {
        /* Make paint invalid (no future draw) */
        //paint->setShader(NULL);
        NLOG("Invalid gradient");
        return;
    }
    PAINT->setColor(SK_ColorBLACK);

    PAINT->setShader(shader);
    //NLOG("Add gradient : %p (%d)", shader, shader->getRefCnt());    
}

void NativeSkia::setFillColor(const char *str)
{   
    SkColor color = parseColor(str);

    SkShader *shader = PAINT->getShader();

    if (shader) {
        PAINT->setShader(NULL);
    }

    PAINT->setColor(color);
}

void NativeSkia::setFillColor(uint32_t color)
{   
    SkShader *shader = PAINT->getShader();

    if (shader) {
        PAINT->setShader(NULL);
    }

    PAINT->setColor(color);
}

void NativeSkia::setStrokeColor(const char *str)
{   
    SkColor color = parseColor(str);

    SkShader *shader = PAINT_STROKE->getShader();

    if (shader) {
        PAINT_STROKE->setShader(NULL);
    }

    PAINT_STROKE->setColor(color);

}

void NativeSkia::setStrokeColor(NativeSkGradient *gradient)
{ 
    SkShader *shader;

    if ((shader = gradient->build()) == NULL) {
        return;
    }
    PAINT_STROKE->setColor(SK_ColorBLACK);
    PAINT_STROKE->setShader(shader);
}


void NativeSkia::setStrokeColor(uint32_t color)
{   
    SkShader *shader = PAINT_STROKE->getShader();

    if (shader) {
        PAINT_STROKE->setShader(NULL);
    }

    PAINT_STROKE->setColor(color);
}


NativeShadowLooper *NativeSkia::buildShadow()
{
    if (currentShadow.blur == 0) {
        return NULL;
    }

    return NativeShadowLooper::Create(SkDoubleToScalar(currentShadow.blur),
                                SkDoubleToScalar(currentShadow.x),
                                SkDoubleToScalar(currentShadow.y),
                                currentShadow.color,
                                SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                SkBlurDrawLooper::kHighQuality_BlurFlag);
}

void NativeSkia::setShadowOffsetX(double x)
{
    if (currentShadow.x == x) return;
    currentShadow.x = x;
    SkSafeUnref(PAINT->setLooper(buildShadow()));
}

void NativeSkia::setShadowOffsetY(double y)
{
    if (currentShadow.y == y) return;
    currentShadow.y = y;
    SkSafeUnref(PAINT->setLooper(buildShadow()));
}

void NativeSkia::setShadowBlur(double blur)
{
    if (currentShadow.blur == blur) return;
    currentShadow.blur = blur;

    SkSafeUnref(PAINT->setLooper(buildShadow()));
}

void NativeSkia::setShadowColor(const char *str)
{
    SkColor color = parseColor(str);

    if (currentShadow.color == color) return;
    currentShadow.color = color;

    SkSafeUnref(PAINT->setLooper(buildShadow()));
}

void NativeSkia::setSmooth(bool val, int level)
{
    SkPaint::FilterLevel flevel = SkPaint::kNone_FilterLevel;

    if (val) {
        switch (level) {
            case 0: flevel = SkPaint::kNone_FilterLevel;break;
            case 1: flevel = SkPaint::kLow_FilterLevel;break;
            case 2: flevel = SkPaint::kMedium_FilterLevel;break;
            case 3: default: flevel = SkPaint::kHigh_FilterLevel;break;
        }
    }

    PAINT->setFilterLevel(flevel);
    PAINT_STROKE->setFilterLevel(flevel);
}

void NativeSkia::setGlobalAlpha(double value)
{
    if (value < 0) return;

    SkScalar maxuint = SkIntToScalar(255);
    globalAlpha = SkMinScalar(SkDoubleToScalar(value) * maxuint, maxuint);
    SkColorFilter *filter = SkColorFilter::CreateModeFilter(
        SkColorSetARGB(globalAlpha, 255, 255, 255),
        SkXfermode::kModulate_Mode);

    PAINT->setColorFilter(filter);
    PAINT_STROKE->setColorFilter(filter);

    filter->unref();
}

static struct _native_xfer_mode {
    const char *str;
    SkXfermode::Mode mode;
} native_xfer_mode[] = {
    {"source-over",        SkXfermode::kSrcOver_Mode},
    {"source-in",          SkXfermode::kSrcIn_Mode},
    {"source-out",         SkXfermode::kSrcOut_Mode},
    {"source-atop",        SkXfermode::kSrcATop_Mode},
    {"destination-over",   SkXfermode::kDstOver_Mode},
    {"destination-in",     SkXfermode::kDstIn_Mode},
    {"destination-out",    SkXfermode::kDstOut_Mode},
    {"destination-atop",   SkXfermode::kDstATop_Mode},
    {"lighter",            SkXfermode::kPlus_Mode},
    {"darker",             SkXfermode::kDarken_Mode},
    {"copy",               SkXfermode::kSrc_Mode},
    {"xor",                SkXfermode::kXor_Mode},
    {"lighten",            SkXfermode::kColorDodge_Mode},
    {NULL,                 SkXfermode::kSrcOver_Mode}
};

void NativeSkia::setGlobalComposite(const char *str)
{
    for (int i = 0; native_xfer_mode[i].str != NULL; i++) {
        if (strcasecmp(native_xfer_mode[i].str, str) == 0) {
            SkXfermode *mode = SkXfermode::Create(native_xfer_mode[i].mode);
            PAINT->setXfermode(mode);
            PAINT_STROKE->setXfermode(mode);
            SkSafeUnref(mode);
            break;
        }
    }
    
    asComposite = 1;
}

void NativeSkia::setLineWidth(double size)
{
    PAINT_STROKE->setStrokeWidth(SkDoubleToScalar(size));
}

void NativeSkia::setMiterLimit(double size)
{
    PAINT_STROKE->setStrokeMiter(SkDoubleToScalar(size));
}

void NativeSkia::beginPath()
{
    if (currentPath) {
        delete currentPath;
    }

    currentPath = new SkPath();

    //currentPath->moveTo(SkIntToScalar(0), SkIntToScalar(0));
}

/* TODO: bug? looks like we need to add to the previous value (strange) */
void NativeSkia::moveTo(double x, double y)
{
    if (!currentPath) {
        beginPath();
    }
    const SkMatrix &m = m_Canvas->getTotalMatrix();
    SkPoint pt = SkPoint::Make(SkDoubleToScalar(x), SkDoubleToScalar(y));

    SkMatrix::MapPtsProc proc = m.getMapPtsProc();

    proc(m, &pt, &pt, 1);

    currentPath->moveTo(pt);
}

void NativeSkia::lineTo(double x, double y)
{
    /* moveTo is set? */
    if (!currentPath) {
        beginPath();
    }

    const SkMatrix &m = m_Canvas->getTotalMatrix();
    SkPoint pt = SkPoint::Make(SkDoubleToScalar(x), SkDoubleToScalar(y));

    SkMatrix::MapPtsProc proc = m.getMapPtsProc();

    proc(m, &pt, &pt, 1);

    if (!currentPath->countPoints()) {
        currentPath->moveTo(pt);
    } else {
        currentPath->lineTo(pt);
    }
}

void NativeSkia::fill()
{
    if (!currentPath) {
        return;
    }

    SkShader *shader = PAINT->getShader();
    SkShader *tmpShader;

    if (shader != NULL) {
        tmpShader = SkShader::CreateLocalMatrixShader(shader, m_Canvas->getTotalMatrix());
        shader->ref();
        PAINT->setShader(tmpShader)->unref();
    }
    /* The matrix was already applied point by point */
    m_Canvas->save(SkCanvas::kMatrix_SaveFlag);
    m_Canvas->resetMatrix();
    m_Canvas->drawPath(*currentPath, *PAINT);
    m_Canvas->restore();

    if (shader != NULL) {
        PAINT->setShader(shader)->unref();
    }

    CANVAS_FLUSH();
}

void NativeSkia::stroke()
{
    if (!currentPath) {
        return;
    }
    SkShader *shader = PAINT_STROKE->getShader();
    SkShader *tmpShader;

    if (shader != NULL) {
        tmpShader = SkShader::CreateLocalMatrixShader(shader, m_Canvas->getTotalMatrix());
        shader->ref();
        PAINT->setShader(tmpShader)->unref();
    }

    /* The matrix was already applied point by point */
    m_Canvas->save(SkCanvas::kMatrix_SaveFlag);
    m_Canvas->resetMatrix();

    SkScalar lineWidth = PAINT_STROKE->getStrokeWidth();
    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    PAINT_STROKE->setStrokeWidth(SkFloatToScalar(ratio) * lineWidth);

#if 0
    SkMatrix mat;
    mat.setIdentity();
    PAINT_STROKE->setPathEffect(new SkLine2DPathEffect(SK_Scalar1, mat))->unref();
#endif
    m_Canvas->drawPath(*currentPath, *PAINT_STROKE);
    PAINT_STROKE->setStrokeWidth(lineWidth);

    m_Canvas->restore();

    if (shader != NULL) {
        PAINT->setShader(shader)->unref();
    }

    CANVAS_FLUSH();
}

void NativeSkia::closePath()
{
    if (!currentPath) {
        return;
    }

    currentPath->close();

}

void NativeSkia::clip()
{
    if (!currentPath) {
        return;
    }

    m_Canvas->clipPath(*currentPath);
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

void NativeSkia::rect(double x, double y, double width, double height)
{
    if (!currentPath) {
        beginPath();
    }
    SkMatrix m = m_Canvas->getTotalMatrix();

    SkRect r = SkRect::MakeXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));
    
    SkPath tmpPath;

    tmpPath.addRect(r);
    tmpPath.transform(m);
    currentPath->addPath(tmpPath);
}

void NativeSkia::addPath(const SkPath& path, SkPath *to)
{
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb    verb;
    int i = 0;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
            {
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
            default:break;
        }
        i++;
    }
}

void NativeSkia::arc(int x, int y, int r,
    double startAngle, double endAngle, int CCW)
{
    if (!currentPath || (!startAngle && !endAngle) || !r) {
        return;
    }

    double sweep = endAngle - startAngle;
    SkMatrix m = m_Canvas->getTotalMatrix();

    SkScalar cx = SkIntToScalar(x);
    SkScalar cy = SkIntToScalar(y);
    SkScalar s360 = SkIntToScalar(360);
    SkScalar radius = SkIntToScalar(r);

    SkScalar start = SkDoubleToScalar(180 * startAngle / SK_ScalarPI);
    SkScalar end = SkDoubleToScalar(180 * sweep / SK_ScalarPI);

    SkRect rect;
    rect.set(cx-radius, cy-radius, cx+radius, cy+radius);

    SkPath tmppath;
    bool dropfirst = false;

    if (!currentPath->isEmpty()) {
        SkPoint lp;
        currentPath->getLastPt(&lp);
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
        this->addPath(tmppath, currentPath);
    } else {
        currentPath->addPath(tmppath);
    }
}

void NativeSkia::quadraticCurveTo(double cpx, double cpy, double x, double y)
{
    if (!currentPath) {
        return;
    }

    SkMatrix m = m_Canvas->getTotalMatrix();

    if (!currentPath->countPoints()) {
        currentPath->moveTo(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy));
    }

    SkPoint cp, p;

    m.mapXY(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy), &cp);
    m.mapXY(SkDoubleToScalar(x), SkDoubleToScalar(y), &p);

    currentPath->quadTo(cp, p);
}

void NativeSkia::bezierCurveTo(double cpx, double cpy, double cpx2, double cpy2,
    double x, double y)
{
    if (!currentPath) {
        return;
    }
    
    if (!currentPath->countPoints()) {
        currentPath->moveTo(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy));
    }


    SkMatrix m = m_Canvas->getTotalMatrix();
    SkPoint p1, p2, p3;

    m.mapXY(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy), &p1);
    m.mapXY(SkDoubleToScalar(cpx2), SkDoubleToScalar(cpy2), &p2);
    m.mapXY(SkDoubleToScalar(x), SkDoubleToScalar(y), &p3);

    currentPath->cubicTo(p1, p2, p3);

}

void NativeSkia::light(double x, double y, double z)
{
    SkPoint3 pt(SkDoubleToScalar(x), SkDoubleToScalar(y), SkDoubleToScalar(z));

    SkColor white(0xAAFFFFFF);
    PAINT->setImageFilter(SkLightingImageFilter::CreatePointLitDiffuse(pt, white,
        SkIntToScalar(1), SkIntToScalar(2)));

    printf("Light created\n");
}

void NativeSkia::rotate(double angle)
{
    m_Canvas->rotate(SkDoubleToScalar(180 * angle / SK_ScalarPI));
}

void NativeSkia::scale(double x, double y)
{
    m_Canvas->scale(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::translate(double x, double y)
{
    m_Canvas->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::save()
{
    struct _nativeState *nstate = new struct _nativeState;

    nstate->paint = new SkPaint(*PAINT);
    nstate->paint_stroke = new SkPaint(*PAINT_STROKE);
    nstate->next = state;
    nstate->baseline = BASELINE_ALPHABETIC;

    state = nstate;

    m_Canvas->save();
}

void NativeSkia::restore()
{
    if (state->next) {
        struct _nativeState *dstate = state->next;
        delete state->paint;
        delete state->paint_stroke;
        delete state;

        state = dstate;
    } else {
        NLOG("restore() without matching save()\n");
    }
    
    m_Canvas->restore();
}

double NativeSkia::measureText(const char *str, size_t length)
{
    return SkScalarToDouble(PAINT->measureText(str, length));
}

void NativeSkia::skew(double x, double y)
{
    m_Canvas->skew(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

/*
    composite :
    http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/SkiaUtils.cpp
*/


void NativeSkia::getPathBounds(double *left, double *right,
    double *top, double *bottom)
{
    if (currentPath == NULL) {
        return;
    }
    SkRect bounds = currentPath->getBounds();

    *left = SkScalarToDouble(bounds.fLeft);
    *right = SkScalarToDouble(bounds.fRight);
    *top = SkScalarToDouble(bounds.fTop);
    *bottom = SkScalarToDouble(bounds.fBottom);
}

/*
    pointInPath :
    http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/SkiaUtils.cpp#115
*/
bool NativeSkia::SkPathContainsPoint(double x, double y)
{
    if (currentPath == NULL) {
        return false;
    }

    SkRect bounds = currentPath->getBounds();
    SkPath::FillType ft = SkPath::kWinding_FillType;

    // We can immediately return false if the point is outside the bounding
    // rect.  We don't use bounds.contains() here, since it would exclude
    // points on the right and bottom edges of the bounding rect, and we want
    // to include them.
    SkScalar fX = SkDoubleToScalar(x);
    SkScalar fY = SkDoubleToScalar(y);
    if (fX < bounds.fLeft || fX > bounds.fRight ||
        fY < bounds.fTop || fY > bounds.fBottom)
        return false;

    // Scale the path to a large size before hit testing for two reasons:
    // 1) Skia has trouble with coordinates close to the max signed 16-bit values, so we scale larger paths down.
    //    TODO: when Skia is patched to work properly with large values, this will not be necessary.
    // 2) Skia does not support analytic hit testing, so we scale paths up to do raster hit testing with subpixel accuracy.

    SkScalar biggestCoord = native_max(native_max(native_max(bounds.fRight,
        bounds.fBottom), -bounds.fLeft), -bounds.fTop);

    if (SkScalarNearlyZero(biggestCoord))
        return false;

    biggestCoord = native_max(native_max(biggestCoord, fX + 1), fY + 1);

    const SkScalar kMaxCoordinate = SkIntToScalar(1 << 15);
    SkScalar scale = SkScalarDiv(kMaxCoordinate, biggestCoord);

    SkRegion rgn;  
    SkRegion clip;
    SkMatrix m;
    SkPath scaledPath;

    SkPath::FillType originalFillType = currentPath->getFillType();
    currentPath->setFillType(ft);

    m.setScale(scale, scale);
    currentPath->transform(m, &scaledPath);

    int ix = static_cast<int>(floor(0.5 + x * scale));
    int iy = static_cast<int>(floor(0.5 + y * scale));
    clip.setRect(ix - 1, iy - 1, ix + 1, iy + 1);

    bool contains = rgn.setPath(scaledPath, clip);
    currentPath->setFillType(originalFillType);

    return contains;
}

void NativeSkia::itransform(double scalex, double skewy, double skewx,
            double scaley, double translatex, double translatey)
{
    SkMatrix m;

    m.setAll(SkDoubleToScalar(scalex), SkDoubleToScalar(skewx),
        SkDoubleToScalar(translatex), SkDoubleToScalar(skewy),
        SkDoubleToScalar(scaley), SkDoubleToScalar(translatey), 0, 0, 0);
    
    SkMatrix im;
    if (m.invert(&im)) {
        printf("transformed\n");
        m_Canvas->concat(im);
    } else {
        printf("Cant revert Matrix\n");
    }
}

void NativeSkia::transform(double scalex, double skewy, double skewx,
            double scaley, double translatex, double translatey, int set)
{
    SkMatrix m;

    float ratio = 1.0f;
    if (set) {
        ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();
    }

    m.setScaleX(SkDoubleToScalar(scalex*ratio));
    m.setSkewX(SkDoubleToScalar(skewx*ratio));
    m.setTranslateX(SkDoubleToScalar(translatex*ratio));

    m.setScaleY(SkDoubleToScalar(scaley*ratio));
    m.setSkewY(SkDoubleToScalar(skewy*ratio));
    m.setTranslateY(SkDoubleToScalar(translatey*ratio));

    m.setPerspX(0);
    m.setPerspY(0);

    m.set(SkMatrix::kMPersp2, SK_Scalar1);

    if (set) {
        m_Canvas->setMatrix(m);
    } else {
        m_Canvas->concat(m);
    }
}

void NativeSkia::setLineCap(const char *capStyle)
{
    if (strcasecmp(capStyle, "round") == 0) {
        PAINT_STROKE->setStrokeCap(SkPaint::kRound_Cap);
    } else if (strcasecmp(capStyle, "square") == 0) {
        PAINT_STROKE->setStrokeCap(SkPaint::kSquare_Cap);
    } else {
        PAINT_STROKE->setStrokeCap(SkPaint::kButt_Cap);
    }
}

void NativeSkia::setLineJoin(const char *joinStyle)
{
     if (strcasecmp(joinStyle, "round") == 0) {
        PAINT_STROKE->setStrokeJoin(SkPaint::kRound_Join);
    } else if (strcasecmp(joinStyle, "bevel") == 0) {
        PAINT_STROKE->setStrokeJoin(SkPaint::kBevel_Join);
    } else {
        PAINT_STROKE->setStrokeJoin(SkPaint::kMiter_Join);
    }
}

void NativeSkia::drawImage(NativeSkImage *image, double x, double y)
{
    SkColor old = PAINT->getColor();
    PAINT->setColor(SK_ColorBLACK);

    if (image->isCanvas) {
        m_Canvas->drawBitmap(image->canvasRef->getDevice()->accessBitmap(false),
            SkDoubleToScalar(x), SkDoubleToScalar(y),
            PAINT);

    } else if (image->img != NULL) {
        m_Canvas->drawBitmap(*image->img, SkDoubleToScalar(x), SkDoubleToScalar(y),
            PAINT);
    }

    PAINT->setColor(old);
    
    /* TODO: clear read'd pixel? */
    CANVAS_FLUSH();
}

void NativeSkia::drawImage(NativeSkImage *image, double x, double y,
    double width, double height)
{
    SkRect r;
    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));

    SkColor old = PAINT->getColor();
    PAINT->setColor(SK_ColorBLACK);

    if (image->isCanvas) {
        m_Canvas->drawBitmapRect(image->canvasRef->getDevice()->accessBitmap(false),
            NULL, r, PAINT);
    } else if (image->img != NULL) {
        m_Canvas->drawBitmapRect(*image->img, NULL, r, PAINT);
    }

    PAINT->setColor(old);

    CANVAS_FLUSH();
}

void NativeSkia::drawImage(NativeSkImage *image,
    int sx, int sy, int swidth, int sheight,
    double dx, double dy, double dwidth, double dheight)
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

    if (image->isCanvas) {
        SkBitmap bitmapImage;

        image->canvasRef->readPixels(src, &bitmapImage);
        bitmapImage.setIsVolatile(true);

        m_Canvas->drawBitmapRect(bitmapImage,
            NULL, dst, PAINT);        
    } else if (image->img != NULL) {
        m_Canvas->drawBitmapRect(*image->img,
            &src, dst, PAINT);
    }

    PAINT->setColor(old);

    CANVAS_FLUSH();
}

void NativeSkia::redrawScreen()
{
    m_Canvas->readPixels(SkIRect::MakeSize(m_Canvas->getDeviceSize()),
        screen);
    m_Canvas->writePixels(*screen, 0, 0);
    CANVAS_FLUSH();  
}

#if 0
void NativeSkia::drawPixelsGL(uint8_t *pixels, int width, int height,
    int x, int y)
{
    m_Canvas->flush();
    glDisable(GL_ALPHA_TEST);

    glWindowPos2i(x, y);

    if (glGetError() != GL_NO_ERROR) {
        printf("got an error\n");
    }
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
    if (glGetError() != GL_NO_ERROR) {
        printf("got an error\n");
    }
    context->resetContext();
}
#endif


void NativeSkia::drawPixels(uint8_t *pixels, int width, int height,
    int x, int y)
{
    SkBitmap bt;
    SkPaint pt;

    bt.setConfig(SkBitmap::kARGB_8888_Config, width, height, width*4);

    bt.setIsVolatile(true);
    bt.installPixels(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
        kUnpremul_SkAlphaType), pixels, width*4);

    pt.setFilterLevel(PAINT->getFilterLevel());

    pt.setXfermodeMode(SkXfermode::kSrc_Mode);
    m_Canvas->drawBitmap(bt, x, y, &pt);
}

int NativeSkia::readPixels(int top, int left, int width, int height,
    uint8_t *pixels)
{
    const SkImageInfo &info = SkImageInfo::Make(width, height,
        kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    if (!m_Canvas->readPixels(info, pixels, width * 4, left, top)) {
        printf("Failed to read pixels\n");
        return 0;
    }

    return 1;
}

static inline bool isBreakable(const unsigned char c)
{
    return (c == ' ' || c == '.' || c == ',' || c == '-' /*|| c == 0xAD*/);
}

double NativeSkia::breakText(const char *str, size_t len,
    struct _NativeLine lines[], double maxWidth, int *length)
{
    struct {
        SkScalar curWordWidth;
        SkScalar curLineWidth;
        size_t curWordLen;
        const char *ptr;
        int curLine;
    } curState;

    int i;

    SkScalar widths[len];

    PAINT->getTextWidths(str, len, widths);
    curState.ptr = str;
    curState.curWordWidth = SkIntToScalar(0);
    curState.curLineWidth = SkIntToScalar(0);
    curState.curWordLen   = 0;
    curState.curLine = 0;

    lines[0].line = str;
    lines[0].len  = 0;

    for (i = 0; i < len; i++) {

        lines[curState.curLine].len++;

        if (isBreakable((unsigned char)str[i])) {
            curState.ptr = &str[i+1];
            curState.curWordWidth = SkIntToScalar(0);
            curState.curWordLen   = 0;
            if (curState.ptr == NULL) {
                break;
            }
        } else {
            curState.curWordWidth += widths[i];
            curState.curWordLen++;
        }

        curState.curLineWidth += widths[i];

        if (curState.curLineWidth > maxWidth) {
            lines[curState.curLine].len = curState.ptr - lines[curState.curLine].line;
            curState.curLine++;

            lines[curState.curLine].line = curState.ptr;
            lines[curState.curLine].len = 0;
            curState.curLineWidth = curState.curWordWidth;
        }
    }

    lines[curState.curLine].len = &str[i] - lines[curState.curLine].line;
    if (length) {
        *length = curState.curLine+1;
    }
    return (curState.curLine+1)*PAINT->getFontSpacing();
}

/*
static SkBitmap load_bitmap() {
    SkStream* stream = new SkFILEStream("/skimages/sesame_street_ensemble-hp.jpg");
    SkAutoUnref aur(stream);
    
    SkBitmap bm;
    if (SkImageDecoder::DecodeStream(stream, &bm, SkBitmap::kNo_Config,
                                     SkImageDecoder::kDecodeBounds_Mode)) {
        SkPixelRef* pr = new SkImageRef_GlobalPool(stream, bm.config(), 1);
        bm.setPixelRef(pr)->unref();
    }
    return bm;
}
*/

void NativeSkia::flush()
{
    m_Canvas->flush();
}


void NativeSkia::setCanvas(SkCanvas *canvas)
{
    SkRefCnt_SafeAssign(m_Canvas, canvas);
}

