#include "NativeContext.h"
#include "NativeUtils.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"
#include "NativeCanvasContext.h"
#include "NativeGLState.h"
#include "NativeSkia.h"
#include "NativeJSNative.h"
#include "NativeJSDocument.h"
#include "NativeJS.h"
#include "NativeJSAV.h"
#include "NativeJSCanvas.h"
#include "NativeJSDocument.h"
#include "NativeJSWindow.h"
#include "NativeUIInterface.h"
#ifdef __linux__
#include "SkImageDecoder.h"
#endif
#include <stdlib.h>
#include <string.h>
#include "NativeMacros.h"

#include "GLSLANG/ShaderLang.h"
#include "NativeMacros.h"
#include "NativeNML.h"

#include <NativeWebSocket.h>

#include <NativeSystemInterface.h>

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

jsval gfunc  = JSVAL_VOID;

int NativeContext_Logger(const char *format)
{
    __NativeUI->log(format);

    return 0;
}

int NativeContext_vLogger(const char *format, va_list ap)
{
    __NativeUI->vlog(format, ap);

    return 0;
}

int NativeContext_LogClear()
{
    __NativeUI->logclear();

    return 0;
}

NativeContext::NativeContext(NativeUIInterface *nui, NativeNML *nml,
    int width, int height, ape_global *net) :
    m_DebugHandler(NULL), m_UI(nui), m_NML(nml)
{
    gfunc = JSVAL_VOID;
    //nui->useOffScreenRendering(true);

    ShInitialize();

    m_SizeDirty = false;
    m_Stats.nframe = 0;
    m_Stats.starttime = NativeUtils::getTick();
    m_Stats.lastmeasuredtime = m_Stats.starttime;
    m_Stats.lastdifftime = 0;
    m_Stats.cumulframe = 0;
    m_Stats.cumultimems = 0.f;
    m_Stats.fps = 0.f;
    m_Stats.minfps = UINT32_MAX;
    m_Stats.sampleminfps = 0.f;

    memset(m_Stats.samples, 0, sizeof(m_Stats.samples));

    this->m_GLState = new NativeGLState(nui);

    this->initHandlers(width, height);

    m_JS = new NativeJS(net);
    m_JS->setPrivate(this);

    m_JS->loadGlobalObjects();
    this->loadNativeObjects(width, height);

    m_JS->setLogger(NativeContext_Logger);
    m_JS->setLogger(NativeContext_vLogger);
    m_JS->setLogger(NativeContext_LogClear);

    if (m_NML) {
        m_NML->setNJS(m_JS);
    }
    /*
        Set path for modules
    */
    m_JS->setPath(NativePath::getPwd());

    m_WSClient = NULL;
    m_WS = NULL;
    /*m_WS = new NativeWebSocketListener(4000, "127.0.0.1");
    m_WS->addListener(this);
    m_WS->start();*/

    m_Jobs.head = NULL;
    m_Jobs.queue = NULL;
}

void NativeContext::loadNativeObjects(int width, int height)
{
    JSContext *cx = m_JS->cx;

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
    NativeJSNative::registerObject(cx);
    /* window() object */
    NativeJSwindow::registerObject(cx, width, height);
    /* document() object */
    NativeJSdocument::registerObject(cx);

    //NativeJSDebug::registerObject(cx);    
}

void NativeContext::setWindowSize(int w, int h)
{
    m_SizeDirty = true;
    /* OS window */
    m_UI->setWindowSize((int)w, (int)h);
    /* Root canvases */
    this->sizeChanged(w, h);
}

void NativeContext::setWindowFrame(int x, int y, int w, int h)
{
    m_SizeDirty = true;
    /* OS window */
    m_UI->setWindowFrame((int)x, (int)y, (int)w, (int)h);
    /* Root canvases */
    this->sizeChanged(w, h);
}

void NativeContext::sizeChanged(int w, int h)
{
    if (!m_SizeDirty) {
        return;
    }

    m_SizeDirty = false;

    NativeJSwindow *jswindow = NativeJSwindow::getNativeClass(m_JS);

    /* Skia GL */
    this->getRootHandler()->setSize((int)w, (int)h);
    /* Native Canvas */
    jswindow->getCanvasHandler()->setSize((int)w, (int)h);
    /* Redraw */
    m_UI->refresh();
}

void NativeContext::createDebugCanvas()
{
    NativeCanvas2DContext *context = (NativeCanvas2DContext *)m_RootHandler->getContext();
    static const int DEBUG_HEIGHT = 60;
    m_DebugHandler = new NativeCanvasHandler(context->getSurface()->getWidth(), DEBUG_HEIGHT, this);
    NativeCanvas2DContext *ctx2d =  new NativeCanvas2DContext(m_DebugHandler, context->getSurface()->getWidth(), DEBUG_HEIGHT, NULL, false);
    m_DebugHandler->setContext(ctx2d);
    ctx2d->setGLState(this->getGLState());

    m_RootHandler->addChild(m_DebugHandler);
    m_DebugHandler->setRight(0);
    m_DebugHandler->setOpacity(0.6);
}

void NativeContext::postDraw()
{
    if (NativeJSdocument::showFPS && m_DebugHandler) {

        NativeSkia *s = ((NativeCanvas2DContext *)m_DebugHandler->getContext())->getSurface();
        m_DebugHandler->bringToFront();

        s->setFillColor(0xFF000000u);
        s->drawRect(0, 0, m_DebugHandler->getWidth(), m_DebugHandler->getHeight(), 0);
        s->setFillColor(0xFFEEEEEEu);

        s->setFontType((char *)"monospace");
        s->drawTextf(5, 12, "NATiVE build %s %s", __DATE__, __TIME__);
        s->drawTextf(5, 25, "Frame: %lld (%lldms)\n", m_Stats.nframe, m_Stats.lastdifftime/1000000LL);
        s->drawTextf(5, 38, "Time : %lldns\n", m_Stats.lastmeasuredtime-m_Stats.starttime);
        s->drawTextf(5, 51, "FPS  : %.2f (%.2f)", m_Stats.fps, m_Stats.sampleminfps);

        s->setLineWidth(0.0);
        for (int i = 0; i < sizeof(m_Stats.samples)/sizeof(float); i++) {
            //s->drawLine(300+i*3, 55, 300+i*3, (40/60)*m_Stats.samples[i]);
            s->setStrokeColor(0xFF004400u);
            s->drawLine(m_DebugHandler->getWidth()-20-i*3, 55, m_DebugHandler->getWidth()-20-i*3, 20.f);
            s->setStrokeColor(0xFF00BB00u);   
            s->drawLine(m_DebugHandler->getWidth()-20-i*3, 55, m_DebugHandler->getWidth()-20-i*3, native_min(60-((40.f/62.f)*(float)m_Stats.samples[i]), 55));
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
    m_Stats.nframe++;

    m_Stats.lastdifftime = tmptime - m_Stats.lastmeasuredtime;
    m_Stats.lastmeasuredtime = tmptime;

    /* convert to ms */
    m_Stats.cumultimems += (float)m_Stats.lastdifftime / 1000000.f;
    m_Stats.cumulframe++;

    m_Stats.minfps = native_min(m_Stats.minfps, 1000.f/(m_Stats.lastdifftime/1000000.f));
    //printf("FPS : %f\n", 1000.f/(m_Stats.lastdifftime/1000000.f));

    //printf("Last diff : %f\n", (float)(m_Stats.lastdifftime/1000000.f));

    /* Sample every 1000ms */
    if (m_Stats.cumultimems >= 1000.f) {
        m_Stats.fps = 1000.f/(float)(m_Stats.cumultimems/(float)m_Stats.cumulframe);
        m_Stats.cumulframe = 0;
        m_Stats.cumultimems = 0.f;
        m_Stats.sampleminfps = m_Stats.minfps;
        m_Stats.minfps = UINT32_MAX;

        memmove(&m_Stats.samples[1], m_Stats.samples, sizeof(m_Stats.samples)-sizeof(float));

        m_Stats.samples[0] = m_Stats.fps;
    }

    NativeJSwindow::getNativeClass(m_JS)->callFrameCallbacks(tmptime);

    if (gfunc != JSVAL_VOID) {
        JSAutoRequest ar(m_JS->cx);
        JS_CallFunctionValue(m_JS->cx, JS_GetGlobalObject(m_JS->cx), gfunc, 0, NULL, &rval);
    }
}

NativeContext::~NativeContext()
{
    JS_RemoveValueRoot(m_JS->cx, &gfunc);

    if (m_DebugHandler != NULL) {
        delete m_DebugHandler->getContext();
        delete m_DebugHandler;
    }

    if (m_RootHandler != NULL) {
        delete m_RootHandler->getContext();
        delete m_RootHandler;
    }

    NativeJSwindow *jswindow = NativeJSwindow::getNativeClass(m_JS);
    jswindow->callFrameCallbacks(0, true);

    delete m_JS;
    delete m_GLState;
    printf("destroy reader\n");
    delete m_WS;
    
    NativeSkia::glcontext = NULL;

    ShFinalize();
}

void NativeContext::rendered(uint8_t *pdata, int width, int height)
{
    if (m_WSClient) {
        m_WSClient->write((const char *)pdata, width*height*4, true);
    }
}

void NativeContext::frame()
{
    this->callFrame();
    this->postDraw();

    this->execJobs();
    
    m_RootHandler->getContext()->flush();
    m_RootHandler->getContext()->resetGLContext();

    /* We draw on the UI fbo */
    glBindFramebuffer(GL_FRAMEBUFFER, m_UI->getFBO());
    NativeLayerizeContext ctx;
    ctx.reset();
    NativeLayerSiblingContext sctx;
    ctx.siblingCtx = &sctx;
    
    m_RootHandler->layerize(ctx);
    /* Skia context is dirty after a call to layerize */
    ((NativeCanvas2DContext *)m_RootHandler->getContext())->resetSkiaContext();
}

void NativeContext::initHandlers(int width, int height)
{
    m_RootHandler = new NativeCanvasHandler(width, height, this);

    m_RootHandler->setContext(new NativeCanvas2DContext(m_RootHandler, width, height, m_UI));
    m_RootHandler->getContext()->setGLState(this->getGLState());
}

void NativeContext::addJob(void (*job)(void *arg), void *arg)
{
    struct NativeJobQueue *obj = (struct NativeJobQueue *)malloc(sizeof(struct NativeJobQueue));

    obj->job = job;
    obj->arg = arg;
    obj->next = NULL;

    if (m_Jobs.head == NULL) {
        m_Jobs.head = obj;
    }
    if (m_Jobs.queue == NULL) {
        m_Jobs.queue = obj;
    } else {
        m_Jobs.queue->next = obj;
    }
}

void NativeContext::execJobs()
{
    if (m_Jobs.head == NULL) {
        return;
    }

    struct NativeJobQueue *obj, *tObj;

    for (obj = m_Jobs.head; obj != NULL; obj = tObj) {
        tObj = obj->next;

        obj->job(obj->arg);

        free(obj);
    }

    m_Jobs.head = NULL;
    m_Jobs.queue = NULL;
}

void NativeContext::onMessage(const NativeSharedMessages::Message &msg)
{
    switch (msg.event()) {
        case NATIVEWEBSOCKET_SERVER_CONNECT:
            m_WSClient = (NativeWebSocketClientConnection *)msg.args[0].toPtr();
            printf("New WS client for render :)\n");
            break;
    }
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
