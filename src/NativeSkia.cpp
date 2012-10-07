

#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
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
    return canvas->getDeviceSize().fWidth;
}

int NativeSkia::getHeight()
{
    return canvas->getDeviceSize().fHeight;
}

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


SkPMColor NativeSkia::HSLToSKColor(U8CPU alpha, float hsl[3])
{
  double hue = SkScalarToDouble(hsl[0]);
  double saturation = SkScalarToDouble(hsl[1]);
  double lightness = SkScalarToDouble(hsl[2]);
  double scaleFactor = 256.0;
  
  // If there's no color, we don't care about hue and can do everything based
  // on brightness.
  if (!saturation) {
    U8CPU lightness;

    if (hsl[2] < 0)
      lightness = 0;
    else if (hsl[2] >= SK_Scalar1)
      lightness = 255;
    else
      lightness = SkScalarToFixed(hsl[2]) >> 8;

    unsigned greyValue = SkAlphaMul(lightness, alpha);
    return SkColorSetARGB(alpha, greyValue, greyValue, greyValue);
  }

  double temp2 = (lightness < 0.5) ?
      lightness * (1.0 + saturation) :
      lightness + saturation - (lightness * saturation);
  double temp1 = 2.0 * lightness - temp2;

  double rh = calcHue(temp1, temp2, hue + 1.0 / 3.0);
  double gh = calcHue(temp1, temp2, hue);
  double bh = calcHue(temp1, temp2, hue - 1.0 / 3.0);

  return SkColorSetARGB(alpha,
      SkAlphaMul(static_cast<int>(rh * scaleFactor), alpha),
      SkAlphaMul(static_cast<int>(gh * scaleFactor), alpha),
      SkAlphaMul(static_cast<int>(bh * scaleFactor), alpha));
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
            SkScalar final[4];

            /* TODO: limits? */
            final[0] = SkScalarDiv(array[0], SkIntToScalar(360));
            final[1] = SkScalarDiv(array[1], SkIntToScalar(100));
            final[2] = SkScalarDiv(array[2], SkIntToScalar(100));
            final[3] = SkScalarMul(array[3], SkIntToScalar(255));

            SkPMColor hsl = HSLToSKColor(final[3], final);
            //printf("Got an HSL : %f %f %f %d\n", array[0], array[1], array[2], hsl);
            return SkPMColorToColor(hsl);

        }

    } else {
        SkParse::FindColor(str, &color);
    }

    return color;
}

int NativeSkia::bindOffScreen(int width, int height)
{
    SkBitmap bitmap;

    bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap.allocPixels();

    canvas = new SkCanvas(bitmap);

    /* TODO: Move the following in a common methode (init) */
    globalAlpha = 255;
    currentPath = NULL;

    state = new struct _nativeState;
    state->next = NULL;

    PAINT = new SkPaint;

    memset(&currentShadow, 0, sizeof(NativeShadow_t));
    currentShadow.color = SkColorSetARGB(255, 0, 0, 0);

    PAINT->setARGB(255, 0, 0, 0);
    PAINT->setAntiAlias(true);

    PAINT->setStyle(SkPaint::kFill_Style);
    PAINT->setFilterBitmap(true);
 
    PAINT->setSubpixelText(true);
    PAINT->setAutohinted(true);

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
    
    this->setLineWidth(1);

    asComposite = 0;

    return 0;
}

int NativeSkia::bindGL(int width, int height)
{
    const GrGLInterface *interface =  GrGLCreateNativeInterface();
    
    if (interface == NULL) {
        printf("Cant get interface\n");
        return 0;
    }
    
    context = GrContext::Create(kOpenGL_Shaders_GrEngine,
        (GrPlatform3DContext)interface);

    if (context == NULL) {
        printf("Cant get context\n");
    }
    

    GrPlatformRenderTargetDesc desc;
    //GrGLRenderTarget *t = new GrGLRenderTarget();
    
    desc.fWidth = SkScalarRound(width);
    desc.fHeight = SkScalarRound(height);

    desc.fConfig = kSkia8888_PM_GrPixelConfig;

    GR_GL_GetIntegerv(interface, GR_GL_SAMPLES, &desc.fSampleCnt);
    GR_GL_GetIntegerv(interface, GR_GL_STENCIL_BITS, &desc.fStencilBits);

    GrGLint buffer = 0;
    GR_GL_GetIntegerv(interface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

    printf("Samples : %d\n", desc.fSampleCnt);
 
    GrRenderTarget * target = context->createPlatformRenderTarget(desc);
    if (target == NULL) {
        printf("Failed to init Skia\n");
        return 0;
    }
    SkGpuDevice *dev = new SkGpuDevice(context, target);
    if (dev == NULL) {
        printf("Failed to init Skia (2)\n");
        return 0;
    }

    globalAlpha = 255;

    canvas = new SkCanvas(dev);
    
    SkSafeUnref(dev);

    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    currentPath = NULL;

    state = new struct _nativeState;
    state->next = NULL;

    PAINT = new SkPaint;

    memset(&currentShadow, 0, sizeof(NativeShadow_t));
    currentShadow.color = SkColorSetARGB(255, 0, 0, 0);

    PAINT->setARGB(255, 0, 0, 0);
    PAINT->setAntiAlias(true);
    
    //PAINT->setLCDRenderText(true);
    PAINT->setStyle(SkPaint::kFill_Style);
    //PAINT->setFilterBitmap(true);
    //PAINT->setXfermodeMode(SkXfermode::kSrcOver_Mode);
    PAINT->setSubpixelText(true);
    PAINT->setAutohinted(true);

    paint_system = new SkPaint;

    paint_system->setARGB(255, 255, 0, 0);
    paint_system->setAntiAlias(true);
    //paint_system->setLCDRenderText(true);
    paint_system->setStyle(SkPaint::kFill_Style);
   
    PAINT_STROKE = new SkPaint;

    PAINT_STROKE->setARGB(255, 0, 0, 0);
    PAINT_STROKE->setAntiAlias(true);
    //PAINT_STROKE->setLCDRenderText(true);
    PAINT_STROKE->setStyle(SkPaint::kStroke_Style);
    
    this->setLineWidth(1);
    /*SkRect r;
    r.setXYWH(SkIntToScalar(0), SkIntToScalar(0),
        SkIntToScalar(640), SkIntToScalar(480));
    canvas->drawRect(r, cleared);*/

    asComposite = 0;

    //canvas->drawColor(SkColorSetARGB(0, 0, 0, 0));

    //canvas->drawARGB(0, 0, 0, 0, SkXfermode::kClear_Mode);

#if 0
    SkIRect rr;
    SkPaint p;
    p.setColor(SK_ColorRED);
    canvas->saveLayer(NULL, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
    rr.setXYWH(100, 0, 100, 100);
    canvas->drawIRect(rr, p);
    //canvas->flush();
    //canvas->restore();

    //canvas->saveLayer(NULL, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
    SkPaint p2;
    p.setColor(SK_ColorBLUE);
    p.setXfermodeMode(SkXfermode::kDstOver_Mode);
    rr.setXYWH(150, 50, 100, 100);
    canvas->drawIRect(rr, p); 
    //canvas->flush();
    canvas->restore();
#endif
    //canvas->drawColor(SkColorSetRGB(255, 0, 0));
    /* TODO: stroke miter? */

    #if 0
    SkString path("skhello.png");
    //Set Text To Draw
    SkString text("Native Studio");
    
    SkPaint paint;
    
    //Set Text ARGB Color
    paint.setARGB(255, 255, 255, 255);
    
    //Turn AntiAliasing On
    
    paint.setAntiAlias(true);
    paint.setLCDRenderText(true);
    paint.setTypeface(SkTypeface::CreateFromName("Courier new bold", SkTypeface::kNormal));
    
    //Set Text Size
    paint.setTextSize(SkIntToScalar(40));

    canvas->drawARGB(255, 255, 255, 255);
    
    //Text X, Y Position Varibles
    int x = 80;
    int y = 60;
    
    canvas->drawText(text.c_str(), text.size(), x, y, paint);
    
    //Set Style and Stroke Width
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(3);
    
    //Draw A Rectangle
    SkRect rect;
    paint.setARGB(255, 255, 255, 255);
    //Left, Top, Right, Bottom
    rect.set(50, 100, 200, 200);
    canvas->drawRoundRect(rect, 20, 20, paint);
    
    canvas->drawOval(rect, paint);
    
    //Draw A Line
    canvas->drawLine(10, 300, 300, 300, paint);
    
    //Draw Circle (X, Y, Size, Paint)
    canvas->drawCircle(100, 400, 50, paint);
    CANVAS_FLUSH();

PAINT->setImageFilter(new SkBlurImageFilter(10.0f, 10.0f))->unref();
    /*PAINT->setMaskFilter(SkBlurMaskFilter::Create(10,
                             SkBlurMaskFilter::kInner_BlurStyle,
                             SkBlurMaskFilter::kNone_BlurFlag));*/
#endif
    canvas->drawColor(SkColorSetARGB(0, 1, 1, 1));
    return 1;
}

void NativeSkia::drawRect(double x, double y, double width,
    double height, int stroke)
{
    SkRect r;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));
#if 0
    if (asComposite) {
        canvas->saveLayer(&r, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
        //PAINT->setXfermodeMode(SkXfermode::kDstOver_Mode);
        //canvas->drawColor(SK_ColorGRAY);
    }
#endif
    canvas->drawRect(r, (stroke ? *PAINT_STROKE : *PAINT));
#if 0
    if (asComposite) {
        canvas->restore();
    }
#endif
    CANVAS_FLUSH();

}

void NativeSkia::drawRect(double x, double y, double width,
    double height, double rx, double ry, int stroke)
{
    SkRect r;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));

    canvas->drawRoundRect(r, SkDoubleToScalar(rx), SkDoubleToScalar(ry),
        (stroke ? *PAINT_STROKE : *PAINT));
}

NativeSkia::~NativeSkia()
{
    struct _nativeState *nstate = state;

    while(nstate) {
        struct _nativeState *tmp = nstate->next;

        delete nstate->paint;
        delete nstate->paint_stroke;
        delete nstate;
        nstate = tmp;
    }
    delete paint_system;
    
    if (currentPath) delete currentPath;

    delete canvas;
}

/* TODO: check if there is a best way to do this;
    context->clear() ?
*/
void NativeSkia::clearRect(int x, int y, int width, int height)
{
/*
    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    paint.setXfermodeMode(SkXfermode::kClear_Mode);
*/
    //CANVAS_FLUSH();
    //glClear(GL_COLOR_BUFFER_BIT);
    SkPaint clearPaint;
    
    clearPaint.setStyle(SkPaint::kFill_Style);
    clearPaint.setARGB(0,0,0,0);
    clearPaint.setXfermodeMode(SkXfermode::kClear_Mode);

    canvas->drawRectCoords(SkIntToScalar(x), SkIntToScalar(y),
        SkIntToScalar(width), SkIntToScalar(height), clearPaint);

    CANVAS_FLUSH();

}

void NativeSkia::setFontSize(double size)
{
    SkScalar ssize = SkDoubleToScalar(size);
    PAINT->setTextSize(ssize);
    PAINT_STROKE->setTextSize(ssize);
}

void NativeSkia::setFontType(const char *str)
{
    SkTypeface *tf = SkTypeface::CreateFromName(str,
        SkTypeface::kNormal);

    PAINT->setTypeface(tf);
    PAINT_STROKE->setTypeface(tf);

    tf->unref();
}

/* TODO: bug with alpha */
void NativeSkia::drawText(const char *text, int x, int y)
{
    canvas->drawText(text, strlen(text),
        SkIntToScalar(x), SkIntToScalar(y), *PAINT);

    CANVAS_FLUSH();
}

void NativeSkia::system(const char *text, int x, int y)
{
    canvas->drawText(text, strlen(text),
        SkIntToScalar(x), SkIntToScalar(y), *paint_system);

    CANVAS_FLUSH();
}

void NativeSkia::setFillColor(NativeSkGradient *gradient)
{ 
    SkShader *shader;

    if ((shader = gradient->build()) == NULL) {
        /* Make paint invalid (no future draw) */
        //paint->setShader(NULL);
        return;
    }
    PAINT->setColor(SK_ColorBLACK);
    PAINT->setShader(shader);
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

NativeShadowLooper *NativeSkia::buildShadow()
{
    if (currentShadow.blur == 0) {
        return NULL;
    }

    return new NativeShadowLooper (SkDoubleToScalar(currentShadow.blur),
                                SkDoubleToScalar(currentShadow.x),
                                SkDoubleToScalar(currentShadow.y),
                                currentShadow.color,
                                SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                SkBlurDrawLooper::kHighQuality_BlurFlag );
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


void NativeSkia::setGlobalAlpha(double value)
{
    if (value < 0) return;

    SkScalar maxuint = SkIntToScalar(255);
    globalAlpha = SkMinScalar(SkDoubleToScalar(value) * maxuint, maxuint);
    SkColorFilter *filter = SkColorFilter::CreateModeFilter(
        SkColorSetARGB(globalAlpha, 255, 255, 255),
        SkXfermode::kMultiply_Mode);

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
    {NULL,                 SkXfermode::kSrcOver_Mode}
};

void NativeSkia::setGlobalComposite(const char *str)
{
    for (int i = 0; native_xfer_mode[i].str != NULL; i++) {
        if (strcasecmp(native_xfer_mode[i].str, str) == 0) {
            PAINT->setXfermodeMode(native_xfer_mode[i].mode);
            PAINT_STROKE->setXfermodeMode(native_xfer_mode[i].mode);
            break;
        }
    }
    
    asComposite = 1;
}

void NativeSkia::setLineWidth(double size)
{
    PAINT_STROKE->setStrokeWidth(SkDoubleToScalar(size));
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

    currentPath->moveTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::lineTo(double x, double y)
{
    /* moveTo is set? */
    if (!currentPath) {
        beginPath();
    }

    currentPath->lineTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::fill()
{
    if (!currentPath) {
        return;
    }

    canvas->drawPath(*currentPath, *PAINT);
    CANVAS_FLUSH();
}

void NativeSkia::stroke()
{
    if (!currentPath) {
        return;
    }

    canvas->drawPath(*currentPath, *PAINT_STROKE);
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

    canvas->clipPath(*currentPath);
    CANVAS_FLUSH();
}

void NativeSkia::rect(double x, double y, double width, double height)
{
    if (!currentPath) {
        beginPath();
    }

    SkRect r;

    r.setXYWH(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height));
    
    currentPath->addRect(r);
}


void NativeSkia::arc(int x, int y, int r,
    double startAngle, double endAngle, int CCW)
{
    if (!currentPath || (!startAngle && !endAngle) || !r) {
        return;
    }

    double sweep = endAngle - startAngle;

    SkRect rect;
    SkScalar cx = SkIntToScalar(x);
    SkScalar cy = SkIntToScalar(y);
    SkScalar s360 = SkIntToScalar(360);
    SkScalar radius = SkIntToScalar(r);

    SkScalar start = SkDoubleToScalar(180 * startAngle / SK_ScalarPI);
    SkScalar end = SkDoubleToScalar(180 * sweep / SK_ScalarPI);

    rect.set(cx-radius, cy-radius, cx+radius, cy+radius);

    if (end >= s360 || end <= -s360) {
        // Move to the start position (0 sweep means we add a single point).
        currentPath->arcTo(rect, start, 0, false);
        // Draw the circle.
        currentPath->addOval(rect);
        // Force a moveTo the end position.
        currentPath->arcTo(rect, start + end, 0, true);        
    } else {
        if (CCW && end > 0) {
            end -= s360;
        } else if (!CCW && end < 0) {
            end += s360;
        }

        currentPath->arcTo(rect, start, end, false);        
    }
}

void NativeSkia::quadraticCurveTo(int cpx, int cpy, int x, int y)
{
    if (!currentPath) {
        return;
    }

    currentPath->quadTo(SkIntToScalar(cpx), SkIntToScalar(cpy),
        SkIntToScalar(x), SkIntToScalar(y));
}

void NativeSkia::bezierCurveTo(double cpx, double cpy, double cpx2, double cpy2,
    double x, double y)
{
    if (!currentPath) {
        return;
    }

    currentPath->cubicTo(SkDoubleToScalar(cpx), SkDoubleToScalar(cpy),
        SkDoubleToScalar(cpx2), SkDoubleToScalar(cpy2),
        SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::rotate(double angle)
{
    canvas->rotate(SkDoubleToScalar(180 * angle / SK_ScalarPI));
}

void NativeSkia::scale(double x, double y)
{
    canvas->scale(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::translate(double x, double y)
{
    canvas->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void NativeSkia::save()
{
    struct _nativeState *nstate = new struct _nativeState;

    nstate->paint = new SkPaint(*PAINT);
    nstate->paint_stroke = new SkPaint(*PAINT_STROKE);
    nstate->next = state;

    state = nstate;

    canvas->save();
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
        printf("Shouldnt be there\n");
    }
    
    canvas->restore();
}

double NativeSkia::measureText(const char *str, size_t length)
{
    return SkScalarToDouble(PAINT->measureText(str, length));
}

void NativeSkia::skew(double x, double y)
{
    canvas->skew(SkDoubleToScalar(x), SkDoubleToScalar(y));
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


void NativeSkia::transform(double scalex, double skewy, double skewx,
            double scaley, double translatex, double translatey, int set)
{
    SkMatrix m;

    m.setScaleX(SkDoubleToScalar(scalex));
    m.setSkewX(SkDoubleToScalar(skewx));
    m.setTranslateX(SkDoubleToScalar(translatex));

    m.setScaleY(SkDoubleToScalar(scaley));
    m.setSkewY(SkDoubleToScalar(skewy));
    m.setTranslateY(SkDoubleToScalar(translatey));

    m.setPerspX(0);
    m.setPerspY(0);

    m.set(SkMatrix::kMPersp2, SK_Scalar1);

    if (set) {
        canvas->setMatrix(m);
    } else {
        canvas->concat(m);
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
        image->canvasRef->readPixels(
            SkIRect::MakeSize(image->canvasRef->getDeviceSize()),
            &image->img);
    }

    if (image->fixedImg != NULL) {
        image->fixedImg->draw(canvas, SkDoubleToScalar(x), SkDoubleToScalar(y),
            PAINT);
    } else {
        canvas->drawBitmap(image->img, SkDoubleToScalar(x), SkDoubleToScalar(y),
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
        image->canvasRef->readPixels(SkIRect::MakeSize(
            image->canvasRef->getDeviceSize()),
            &image->img);
    }

    if (!image->img.hasMipMap()) {
        printf("build mipmap\n");
        image->img.buildMipMap();
    }
    canvas->drawBitmapRect(image->img, NULL, r, PAINT);

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
    /* TODO: ->readPixels : switch to readPixels(bitmap, x, y); */
    src.setXYWH(sx, sy, swidth, sheight);

    if (image->isCanvas) {
        image->canvasRef->readPixels(src, &image->img);
    }

    dst.setXYWH(SkDoubleToScalar(dx), SkDoubleToScalar(dy),
        SkDoubleToScalar(dwidth), SkDoubleToScalar(dheight));

    if (!image->img.hasMipMap()) {
        image->img.buildMipMap();
    }

    canvas->drawBitmapRect(image->img,
        (image->isCanvas ? NULL : &src), dst, PAINT);

    PAINT->setColor(old);

    CANVAS_FLUSH();
}

void NativeSkia::redrawScreen()
{
    canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()),
        screen);
    canvas->writePixels(*screen, 0, 0);
    CANVAS_FLUSH();  
}

#if 0
void NativeSkia::drawPixelsGL(uint8_t *pixels, int width, int height,
    int x, int y)
{
    canvas->flush();
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
    //drawPixelsGL(pixels, width, height, x, y);
    //return;

    SkBitmap bt;
    SkPaint pt;
    SkRect r;

    uint32_t *PMPixels = (uint32_t *)alloca(width * height * 4);
    bt.setConfig(SkBitmap::kARGB_8888_Config, width, height, width*4);

    SkConvertConfig8888Pixels(PMPixels, width*4,
        SkCanvas::kNative_Premul_Config8888,
        (uint32_t*)pixels, width*4, SkCanvas::kRGBA_Unpremul_Config8888,
        width, height);

    bt.setIsVolatile(true);
    bt.setPixels(PMPixels);
    r.setXYWH(x, y, width, height);

    canvas->saveLayer(NULL, NULL);
        canvas->clipRect(r, SkRegion::kReplace_Op);
        canvas->drawColor(SK_ColorWHITE);
        canvas->drawBitmap(bt, x, y);
    canvas->restore();
}

int NativeSkia::readPixels(int top, int left, int width, int height,
    uint8_t *pixels)
{
    SkBitmap bt;

    bt.setConfig(SkBitmap::kARGB_8888_Config, width, height, width*4);
    memset(pixels, 0, width * height * 4);

    bt.setPixels(pixels);

    if (!canvas->readPixels(&bt, left, top, SkCanvas::kRGBA_Premul_Config8888)) {
        printf("Failed to read pixels\n");
        return 0;
    }

    return 1;
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
    canvas->flush();
}
