/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/


#include "NativeSkia.h"

#include "SkData.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTime.h"
#include "SkRefCnt.h"
#include "SkParse.h"

#include "GrContext.h"
#include "SkTypeface.h"

#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"

#include "SkOSFile.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkStream.h"

#include "SkGPipe.h"


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


int NativeSkia::bindGL(int width, int height)
{
    const GrGLInterface *interface = GrGLCreateNativeInterface();
    if (interface == NULL) {
        printf("Cant get interface\n");
        return 0;
    }

    GrContext *context = GrContext::Create(kOpenGL_Shaders_GrEngine,
        (GrPlatform3DContext)interface);

    GrPlatformRenderTargetDesc desc;
    
    desc.fWidth = SkScalarRound(width);
    desc.fHeight = SkScalarRound(height);

    desc.fConfig = kSkia8888_PM_GrPixelConfig;
    
    GR_GL_GetIntegerv(interface, GR_GL_SAMPLES, &desc.fSampleCnt);
    GR_GL_GetIntegerv(interface, GR_GL_STENCIL_BITS, &desc.fStencilBits);

    GrGLint buffer;
    GR_GL_GetIntegerv(interface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;

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
    
    printf("Skia init !\n");

    globalAlpha = 255;

    canvas = new SkCanvas(dev);
    
    SkSafeUnref(dev);

    currentPath = NULL;

    paint = new SkPaint;

    paint->setARGB(255, 0, 0, 0);
    paint->setAntiAlias(true);
    paint->setLCDRenderText(true);
    paint->setStyle(SkPaint::kFill_Style);

    paint_stroke = new SkPaint;

    paint_stroke->setARGB(255, 0, 0, 0);
    paint_stroke->setAntiAlias(true);
    paint_stroke->setLCDRenderText(true);
    paint_stroke->setStyle(SkPaint::kStroke_Style);


    this->setLineWidth(1);

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
    canvas->flush();
#endif
    return 1;
}

void NativeSkia::drawRect(double x, double y, double width,
    double height, double stroke)
{
    canvas->drawRectCoords(SkDoubleToScalar(x), SkDoubleToScalar(y),
        SkDoubleToScalar(width), SkDoubleToScalar(height),
        (stroke ? *paint_stroke : *paint));

    canvas->flush();

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
    SkPaint clearPaint;
    clearPaint.setColor(SK_ColorWHITE);
    clearPaint.setStyle(SkPaint::kFill_Style);

    canvas->drawRectCoords(SkIntToScalar(x), SkIntToScalar(y),
        SkIntToScalar(width), SkIntToScalar(height), clearPaint);

    canvas->flush();

}

void NativeSkia::drawText(const char *text, int x, int y)
{
    canvas->drawText(text, strlen(text),
        SkIntToScalar(x), SkIntToScalar(y), *paint);

    canvas->flush();
}

/* TODO : move color logic to a separate function */
void NativeSkia::setFillColor(const char *str)
{   
    SkColor color = SK_ColorBLACK;
    if (strncasecmp(str, "rgb", 3) == 0) {
        SkScalar array[4];

        int count = count_separators(str, ",") + 1;
        
        if (count == 4) {
            if (str[3] != 'a') {
                count = 3;
            }
        } else if (count != 3) {
            return;
        } 

        array[3] = SK_Scalar1;
        
        const char* end = SkParse::FindScalars(&str[(str[3] == 'a' ? 5 : 4)],
            array, count);

        if (end == NULL) {
            return;
        }

        array[3] *= 255;

        color = SkColorSetARGB(SkScalarRound(array[3]), SkScalarRound(array[0]),
        SkScalarRound(array[1]), SkScalarRound(array[2]));

    } else {
        SkParse::FindColor(str, &color);
    }

    paint->setColor(color);

    paint->setAlpha(SkAlphaMul(paint->getAlpha(),
        SkAlpha255To256(globalAlpha)));
}

void NativeSkia::setStrokeColor(const char *str)
{   
    SkColor color = SK_ColorBLACK;
    if (strncasecmp(str, "rgb", 3) == 0) {
        SkScalar array[4];

        int count = count_separators(str, ",") + 1;
        
        if (count == 4) {
            if (str[3] != 'a') {
                count = 3;
            }
        } else if (count != 3) {
            return;
        } 

        array[3] = SK_Scalar1;
        
        const char* end = SkParse::FindScalars(&str[(str[3] == 'a' ? 5 : 4)],
            array, count);

        if (end == NULL) {
            return;
        }

        array[3] *= 255;

        color = SkColorSetARGB(SkScalarRound(array[3]), SkScalarRound(array[0]),
        SkScalarRound(array[1]), SkScalarRound(array[2]));

    } else {
        SkParse::FindColor(str, &color);
    }

    paint_stroke->setColor(color);

    paint_stroke->setAlpha(SkAlphaMul(paint_stroke->getAlpha(),
        SkAlpha255To256(globalAlpha)));

}

void NativeSkia::setGlobalAlpha(double value)
{

    if (value < 0) return;

    SkScalar maxuint = SkIntToScalar(255);
    globalAlpha = SkMinScalar(SkDoubleToScalar(value) * maxuint, maxuint);

    paint->setAlpha(globalAlpha);
    paint_stroke->setAlpha(globalAlpha);
}

void NativeSkia::setLineWidth(double size)
{
    paint_stroke->setStrokeWidth(SkDoubleToScalar(size));
}

void NativeSkia::beginPath()
{
    if (currentPath) {
        delete currentPath;
    }

    currentPath = new SkPath();
    //currentPath->moveTo(SkIntToScalar(0), SkIntToScalar(0));
}

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

    canvas->drawPath(*currentPath, *paint);
    canvas->flush();
}

void NativeSkia::stroke()
{
    if (!currentPath) {
        return;
    }

    canvas->drawPath(*currentPath, *paint_stroke);
    canvas->flush();   
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
    canvas->flush();
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
    canvas->save();
}

void NativeSkia::restore()
{
    canvas->restore();
}

void NativeSkia::skew(double x, double y)
{
    canvas->skew(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

/*
    composite :
    http://code.google.com/p/webkit-mirror/source/browse/Source/WebCore/platform/graphics/skia/SkiaUtils.cpp
*/

void NativeSkia::transform(double scalex, double skewy, double skewx,
            double scaley, double translatex, double translatey, int set)
{
    SkMatrix m;
    SkMatrix original = canvas->getTotalMatrix();

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
        paint_stroke->setStrokeCap(SkPaint::kRound_Cap);
    } else if (strcasecmp(capStyle, "square") == 0) {
        paint_stroke->setStrokeCap(SkPaint::kSquare_Cap);
    } else {
        paint_stroke->setStrokeCap(SkPaint::kButt_Cap);
    }
}
