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


int NativeSkia::bindGL(int width, int height)
{
    const GrGLInterface *interface = GrGLCreateNativeInterface();
    if (interface == NULL) {
        printf("Cant get interface\n");
        return 0;
    }

    GrContext *context = GrContext::Create(kOpenGL_Shaders_GrEngine, (GrPlatform3DContext)interface);
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
    canvas = new SkCanvas(dev);
    
    SkSafeUnref(dev);

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

void NativeSkia::drawRect(int x, int y, int width, int height, int stroke)
{
    SkIRect rect;

    /* TODO: replace with drawRectCoord */
    rect.set(x, y, width, height);

    canvas->drawIRect(rect, (stroke ? *paint_stroke : *paint));

    canvas->flush();
}

/* TODO: check if there is a best way to do this; */
void NativeSkia::clearRect(int x, int y, int width, int height)
{
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

void NativeSkia::setFillColor(const char *str)
{   
    SkColor color = SK_ColorBLACK;;

    SkParse::FindColor(str, &color);
    paint->setColor(color);
}

void NativeSkia::setStrokeColor(const char *str)
{   
    SkColor color = SK_ColorBLACK;;

    SkParse::FindColor(str, &color);
    paint_stroke->setColor(color);
}

void NativeSkia::setLineWidth(int size)
{
    paint_stroke->setStrokeWidth(SkIntToScalar(size));
}
