#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SkDevice.h"
#include "SkCanvas.h"
#include "SkCGUtils.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"
#include "SkGpuDevice.h"
#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkTypeface.h"
#include "jsapi.h"

int setupSkiaGL(int width, int height)
{

 	const GrGLInterface *interface = GrGLCreateNativeInterface();
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

    SkGpuDevice *dev = new SkGpuDevice(context, target);

    SkCanvas *canvas = new SkCanvas(dev);

    dev->unref();

    return 0;
}

