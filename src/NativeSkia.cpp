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
    printf("Interface ok\n");
    GrContext *context = GrContext::Create(kOpenGL_Shaders_GrEngine, (GrPlatform3DContext)interface);
    printf("Okkk\n");
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
    //canvas = new SkCanvas(dev);
    
    //dev->unref();

    return 1;
}

