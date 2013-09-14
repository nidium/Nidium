#include "NativeContext.h"
#include "NativeUtils.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"
#include "NativeSkia.h"
#include "NativeJSNative.h"
#include "NativeJS.h"
#include "NativeJSAV.h"
#include "NativeJSCanvas.h"
#include "NativeJSDocument.h"
#include "NativeJSWindow.h"
#include "NativeJSConsole.h"
#include "NativeJS_preload.h"
#include "NativeUIInterface.h"
#ifdef __linux__
#include "SkImageDecoder.h"
#endif
#include <stdlib.h>
#include <string.h>

jsval gfunc  = JSVAL_VOID;

NativeContext::NativeContext(NativeUIInterface *nui,
    int width, int height, ape_global *net) :
    UI(nui)
{
    gfunc = JSVAL_VOID;

    currentFPS = 0;
    
    this->stats.nframe = 0;
    this->stats.starttime = NativeUtils::getTick();
    this->stats.lastmeasuredtime = this->stats.starttime;
    this->stats.lastdifftime = 0;
    this->stats.cumulframe = 0;
    this->stats.cumultimems = 0.f;
    this->stats.fps = 0.f;
    this->stats.minfps = UINT32_MAX;
    this->stats.sampleminfps = 0.f;

    memset(this->stats.samples, 0, sizeof(this->stats.samples));

    this->initHandlers(width, height);

    this->njs = new NativeJS(net);
    this->njs->setPrivate(this);

    this->njs->loadGlobalObjects();
    this->loadNativeObjects(width, height);

    this->njs->LoadScriptContent(preload_js,
        strlen(preload_js), "private://__builtin_preload.js");
}

void NativeContext::loadNativeObjects(int width, int height)
{
    JSContext *cx = this->njs->cx;

   /* CanvasRenderingContext2D object */
    NativeCanvas2DContext::registerObject(cx);
    /* Canvas() object */
    NativeJSCanvas::registerObject(cx);
    /* Image() object */
    NativeJSImage::registerObject(cx);
    /* Audio() object */
#ifdef NATIVE_AUDIO_ENABLED
    NativeJSAudio::registerObject(cx);
    NativeJSAudioNode::registerObject(cx);
    NativeJSVideo::registerObject(cx);
#endif
    /* WebGL*() object */
#ifdef NATIVE_WEBGL_ENABLED
    NativeJSNativeGL::registerObject(cx);
    NativeJSWebGLRenderingContext::registerObject(cx);
    NativeJSWebGLObject::registerObject(cx);
    NativeJSWebGLBuffer::registerObject(cx);
    NativeJSWebGLFrameBuffer::registerObject(cx);
    NativeJSWebGLProgram::registerObject(cx);
    NativeJSWebGLRenderbuffer::registerObject(cx);
    NativeJSWebGLShader::registerObject(cx);
    NativeJSWebGLTexture::registerObject(cx);
    NativeJSWebGLUniformLocation::registerObject(cx);
#endif
    /* Native() object */
    NativeJSNative::registerObject(cx, width, height);
    /* window() object */
    NativeJSwindow::registerObject(cx);
    /* console() object */
    NativeJSconsole::registerObject(cx);
    /* document() object */
    NativeJSdocument::registerObject(cx);

    //NativeJSDebug::registerObject(cx);    
}

void NativeContext::setWindowSize(int w, int h)
{
    /* OS window */
    this->getUI()->setWindowSize((int)w, (int)h);

    this->sizeChanged(w, h);
}

void NativeContext::sizeChanged(int w, int h)
{
    NativeJSNative *jsnative = NativeJSNative::getNativeClass(this->getNJS());

    printf("Window size changed %d %d\n", w, h);
    /* Skia GL */
    this->getRootHandler()->setSize((int)w, (int)h);

    /* Native Canvas */
    jsnative->getCanvasHandler()->setSize((int)w, (int)h);
}

void NativeContext::createDebugCanvas()
{
    static const int DEBUG_HEIGHT = 60;
    debugHandler = new NativeCanvasHandler(rootHandler->getContext()->getSurface()->getWidth(), DEBUG_HEIGHT);
    debugHandler->context = new NativeCanvas2DContext(debugHandler, rootHandler->getContext()->getSurface()->getWidth(), DEBUG_HEIGHT, false);
    debugHandler->context->commonDraw = true;
    rootHandler->addChild(debugHandler);
    debugHandler->setRight(0);
    debugHandler->setOpacity(0.6);
}

void NativeContext::postDraw()
{
    if (NativeJSNative::showFPS && debugHandler) {

        NativeSkia *s = debugHandler->getContext()->getSurface();
        debugHandler->bringToFront();

        s->setFillColor(0xFF000000u);
        s->drawRect(0, 0, debugHandler->getWidth(), debugHandler->getHeight(), 0);
        s->setFillColor(0xFFEEEEEEu);

        s->setFontType("monospace");
        s->drawTextf(5, 12, "NATiVE build %s %s", __DATE__, __TIME__);
        s->drawTextf(5, 25, "Frame: %lld (%lldms)\n", this->stats.nframe, stats.lastdifftime/1000000LL);
        s->drawTextf(5, 38, "Time : %lldns\n", stats.lastmeasuredtime-stats.starttime);
        s->drawTextf(5, 51, "FPS  : %.2f (%.2f)", stats.fps, stats.sampleminfps);

        s->setLineWidth(0.0);
        for (int i = 0; i < sizeof(stats.samples)/sizeof(float); i++) {
            //s->drawLine(300+i*3, 55, 300+i*3, (40/60)*stats.samples[i]);
            s->setStrokeColor(0xFF004400u);
            s->drawLine(debugHandler->getWidth()-20-i*3, 55, debugHandler->getWidth()-20-i*3, 20.f);
            s->setStrokeColor(0xFF00BB00u);   
            s->drawLine(debugHandler->getWidth()-20-i*3, 55, debugHandler->getWidth()-20-i*3, native_min(60-((40.f/62.f)*(float)stats.samples[i]), 55));
        }
        //s->setLineWidth(1.0);
        
        //s->translate(10, 10);
        //sprintf(fps, "%d fps", currentFPS);
        //s->system(fps, 5, 10);
        s->flush();
    }
}

/* TODO, move out */
void NativeContext::callFrame()
{
    jsval rval;
    uint64_t tmptime = NativeUtils::getTick();
    stats.nframe++;

    stats.lastdifftime = tmptime - stats.lastmeasuredtime;
    stats.lastmeasuredtime = tmptime;

    /* convert to ms */
    stats.cumultimems += (float)stats.lastdifftime / 1000000.f;
    stats.cumulframe++;

    stats.minfps = native_min(stats.minfps, 1000.f/(stats.lastdifftime/1000000.f));
    //printf("FPS : %f\n", 1000.f/(stats.lastdifftime/1000000.f));

    //printf("Last diff : %f\n", (float)(stats.lastdifftime/1000000.f));

    /* Sample every 1000ms */
    if (stats.cumultimems >= 1000.f) {
        stats.fps = 1000.f/(float)(stats.cumultimems/(float)stats.cumulframe);
        stats.cumulframe = 0;
        stats.cumultimems = 0.f;
        stats.sampleminfps = stats.minfps;
        stats.minfps = UINT32_MAX;

        memmove(&stats.samples[1], stats.samples, sizeof(stats.samples)-sizeof(float));

        stats.samples[0] = stats.fps;
    }

    NativeJSwindow::getNativeClass(this->getNJS())->callFrameCallbacks(tmptime);

    if (gfunc != JSVAL_VOID) {
        JSAutoRequest ar(njs->cx);
        JS_CallFunctionValue(njs->cx, JS_GetGlobalObject(njs->cx), gfunc, 0, NULL, &rval);
    }
}

NativeContext::~NativeContext()
{
    JS_RemoveValueRoot(njs->cx, &gfunc);

    delete njs;

    if (rootHandler != NULL) {
        delete rootHandler->getContext();
        delete rootHandler;
    }

    NativeSkia::glcontext = NULL;
}

void NativeContext::initHandlers(int width, int height)
{
    rootHandler = new NativeCanvasHandler(width, height);
    rootHandler->context = new NativeCanvas2DContext(rootHandler, width, height);
}

void NativeContext::forceLinking()
{
#ifdef __linux__
    CreateJPEGImageDecoder();
    CreatePNGImageDecoder();
    //CreateGIFImageDecoder();
    CreateBMPImageDecoder();
    CreateICOImageDecoder();
    CreateWBMPImageDecoder();
#endif
}
