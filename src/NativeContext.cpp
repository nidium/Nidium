#include "NativeContext.h"
#include "NativeUtils.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"
#include "NativeCanvas3DContext.h"
#ifdef NATIVE_WEBGL_ENABLED
  #include "NativeJSWebGL.h"
#endif
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

#include "NativeMacros.h"
#include "NativeNML.h"

#include <NativeWebSocket.h>
#include <NativeSystemInterface.h>

#include <gl/GrGLInterface.h>

#include <NativeOpenGLHeader.h>

enum {
    NATIVE_SCTAG_IMAGEDATA = NATIVE_SCTAG_MAX,
};

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

void NativeContext::initStats()
{
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
}

void NativeContext::CreateAndAssemble(NativeUIInterface *ui, ape_global *gnet)
{
    new NativeContext(ui, ui->nml, ui->getWidth(), ui->getHeight(), gnet);
}

NativeContext::NativeContext(NativeUIInterface *nui, NativeNML *nml,
    int width, int height, ape_global *net) :
    m_DebugHandler(NULL), m_UI(nui), m_NML(nml),
    m_GLState(NULL), m_SizeDirty(false), m_currentClickedHandler(NULL)
{

    this->resetInputEvents();

    ape_init_pool_list(&m_CanvasEventsCanvas, 0, 8);

    m_UI->NativeCtx = this;

    NativeGLState::CreateForContext(this);

    this->initStats();
    this->initShaderLang();
    this->initHandlers(width, height);

    m_JS = new NativeJS(net);

    m_JS->setStructuredCloneAddition(NativeContext::writeStructuredCloneOp, NativeContext::readStructuredCloneOp);
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
    NativeJSWebGLRenderingContext::registerObject(cx);
    NativeJSWebGLBuffer::registerObject(cx);
    NativeJSWebGLFramebuffer::registerObject(cx);
    NativeJSWebGLProgram::registerObject(cx);
    NativeJSWebGLRenderbuffer::registerObject(cx);
    NativeJSWebGLShader::registerObject(cx);
    NativeJSWebGLTexture::registerObject(cx);
    NativeJSWebGLUniformLocation::registerObject(cx);
    NativeJSWebGLShaderPrecisionFormat::registerObject(cx);
#endif
    /* Native() object */
    NativeJSNative::registerObject(cx);
    /* document() object */
    JSObject *jsdoc = NativeJSdocument::registerObject(cx);
    /* window() object */
    m_JSWindow = NativeJSwindow::registerObject(cx, width, height, jsdoc);

#if DEBUG
    createDebug2Canvas();
#endif
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

    /* Skia GL */
    this->getRootHandler()->setSize((int)w, (int)h);
    /* Native Canvas */
    m_JSWindow->getCanvasHandler()->setSize((int)w, (int)h);
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

#if DEBUG
void NativeContext::createDebug2Canvas()
{
    NativeCanvas2DContext *context = (NativeCanvas2DContext *)m_RootHandler->getContext();
    static const int DEBUG_HEIGHT = 60;
    m_Debug2Handler = new NativeCanvasHandler(context->getSurface()->getWidth(), DEBUG_HEIGHT, this);
    NativeCanvas2DContext *ctx2d =  new NativeCanvas2DContext(m_Debug2Handler, context->getSurface()->getWidth(), DEBUG_HEIGHT, NULL, false);
    m_Debug2Handler->setContext(ctx2d);
    ctx2d->setGLState(this->getGLState());

    m_RootHandler->addChild(m_Debug2Handler);
    m_Debug2Handler->unsetTop();
    m_Debug2Handler->setRight(0);
    m_Debug2Handler->setBottom(0);
}
#endif

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
#if DEBUG
    m_Debug2Handler->bringToFront();
    m_Debug2Handler->getContext()->clear();
    NativeSkia *rootctx = ((NativeCanvas2DContext *)m_Debug2Handler->getContext())->getSurface();
    rootctx->save();

    rootctx->setFillColor("black");
    rootctx->drawText("DEBUG build", 10, 30);
    rootctx->restore();
    rootctx->flush();
#endif
}

/* TODO, move out */
void NativeContext::callFrame()
{
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

    m_JSWindow->callFrameCallbacks(tmptime);

}

NativeContext::~NativeContext()
{
    if (m_DebugHandler != NULL) {
        delete m_DebugHandler->getContext();
        delete m_DebugHandler;
    }

    if (m_RootHandler != NULL) {
        delete m_RootHandler->getContext();
        delete m_RootHandler;
    }

    m_JSWindow->callFrameCallbacks(0, true);

    delete m_JS;
    delete m_GLState;
    delete m_WS;
    delete m_JSWindow;

    NativeSkia::glcontext = NULL;

    ape_destroy_pool_ordered(m_CanvasEventsCanvas.head, NULL, NULL);
    this->clearInputEvents();

    ShFinalize();
}

void NativeContext::rendered(uint8_t *pdata, int width, int height)
{
    if (m_WSClient) {
        m_WSClient->write((unsigned char *)pdata, width*height*4, true);
    }
}

void NativeContext::frame(bool draw)
{
    //this->execJobs();
    /*
        Pending canvas events.
        (e.g. resize events requested between frames,
        Canvas that need to be resized because of a fluidheight)
    */
    this->execPendingCanvasChanges();

    /* Call requestAnimationFrame */
    this->callFrame();
    if (draw) {
        this->postDraw();
    }

    /*
        Exec the pending events a second time in case
        there are resize in the requestAnimationFrame
    */
    this->execPendingCanvasChanges();

    m_UI->makeMainGLCurrent();

    m_RootHandler->getContext()->flush();
    m_RootHandler->getContext()->resetGLContext();

    /* We draw on the UI fbo */
    glBindFramebuffer(GL_FRAMEBUFFER, m_UI->getFBO());
    NativeLayerizeContext ctx;
    ctx.reset();
    NativeLayerSiblingContext sctx;
    ctx.siblingCtx = &sctx;

    m_CanvasOrderedEvents.clear();

    /*
        Compose canvas eachother on the main framebuffer
    */
    m_RootHandler->layerize(ctx, draw);

    this->triggerEvents();
    this->clearInputEvents();

    m_UI->makeMainGLCurrent();
    /* Skia context is dirty after a call to layerize */
    ((NativeCanvas2DContext *)m_RootHandler->getContext())->resetSkiaContext();
}

void NativeContext_destroy_and_handle_events(ape_pool_t *pool, void *ctx)
{
    if (!pool->ptr.data) {
        return;
    }
    NativeInputEvent *ev = (NativeInputEvent *)pool->ptr.data;

    /* top-most element */
    if (ev->getDepth() == ev->m_Origin->getDepth()) {
        ev->m_Handler->_handleEvent(ev);
    }

    delete ev;
}

void NativeContext::triggerEvents()
{
    void *val;

    APE_P_FOREACH_REVERSE((&m_CanvasEventsCanvas), val) {
        /* process through the cleaner callback avoiding a complete iteration */
        ape_destroy_pool_list_ordered((ape_pool_list_t *)val,
                NativeContext_destroy_and_handle_events, NULL);
        __pool_item->ptr.data = NULL;
    }

    /*
        Reset the 'push' pointer.
    */
    ape_pool_rewind(&m_CanvasEventsCanvas);
}

// From third-party/mozilla-central/content/canvas/src/WebGLContextValidate.cpp
// TODO : Handle OpenGL ESJSVAL_
bool NativeContext::initShaderLang()
{
    GLint maxVertexAttribs;
    GLint maxTextureUnits;
    GLint maxTextureSize;
    GLint maxCubeMapTextureSize;
    GLint maxRenderbufferSize;
    GLint maxTextureImageUnits;
    GLint maxVertexTextureImageUnits;
    GLint maxFragmentUniformVectors;
    GLint maxVertexUniformVectors;

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxCubeMapTextureSize);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertexTextureImageUnits);

    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniformVectors);
    maxFragmentUniformVectors /= 4;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniformVectors);
    maxVertexUniformVectors /= 4;

#if 0
    GLint maxVaryingVectors;
    GLenum error;

    // we are now going to try to read GL_MAX_VERTEX_OUTPUT_COMPONENTS and GL_MAX_FRAGMENT_INPUT_COMPONENTS,
    // however these constants only entered the OpenGL standard at OpenGL 3.2. So we will try reading,
    // and check OpenGL error for INVALID_ENUM.

    // before we start, we check that no error already occurred, to prevent hiding it in our subsequent error handling
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("GL error 0x%x occurred during initShaderLang context initialization!\n", error);
        return false;
    }

    // On the public_webgl list, "problematic GetParameter pnames" thread, the following formula was given:
    //   mGLMaxVaryingVectors = min (GL_MAX_VERTEX_OUTPUT_COMPONENTS, GL_MAX_FRAGMENT_INPUT_COMPONENTS) / 4
    GLint maxVertexOutputComponents,
          minFragmentInputComponents;
    glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxVertexOutputComponents);
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &minFragmentInputComponents);

    error = glGetError();
    switch (error) {
        case GL_NO_ERROR:
            maxVaryingVectors = std::min(maxVertexOutputComponents, minFragmentInputComponents) / 4;
            break;
        case GL_INVALID_ENUM:
            maxVaryingVectors = 16; // = 64/4, 64 is the min value for maxVertexOutputComponents in OpenGL 3.2 spec
            break;
        default:
            printf("GL error 0x%x occurred during WebGL context initialization!\n", error);
            return false;
    }
#endif

    ShInitialize();

    ShInitBuiltInResources(&m_ShResources);

    m_ShResources.MaxVertexAttribs = maxVertexAttribs;
    m_ShResources.MaxVertexUniformVectors = maxVertexUniformVectors;
    m_ShResources.MaxVaryingVectors = 16;
    m_ShResources.MaxVertexTextureImageUnits = maxVertexTextureImageUnits;
    m_ShResources.MaxCombinedTextureImageUnits = maxTextureImageUnits;
    m_ShResources.MaxTextureImageUnits = maxTextureImageUnits;
    m_ShResources.MaxFragmentUniformVectors = maxFragmentUniformVectors;
    m_ShResources.MaxDrawBuffers = 1;

    m_ShResources.FragmentPrecisionHigh = 1;

    // FIXME : Check if extension is supported and enable or not
    m_ShResources.OES_standard_derivatives = 0;
    m_ShResources.OES_EGL_image_external = 0;

    return true;
}

void NativeContext::initHandlers(int width, int height)
{
    NativeCanvasHandler::LastIdx = 0;

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

void NativeContext::execPendingCanvasChanges()
{
    ape_htable_item_t *item, *tmpItem;
    for (item = m_CanvasPendingJobs.accessCStruct()->first; item != NULL; item = tmpItem) {
        tmpItem = item->lnext;
        NativeCanvasHandler *handler = (NativeCanvasHandler *)item->content.addrs;
        handler->execPending();
    }
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

bool NativeContext::writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                     JS::HandleObject obj, void *closure)
{

    JS::RootedValue vobj(cx, OBJECT_TO_JSVAL(obj));
    JSType type = JS_TypeOfValue(cx, vobj);

    if (type != JSTYPE_OBJECT) {
        return false;
    }

    if (JS_GetClass(obj) == NativeCanvas2DContext::ImageData_jsclass) {
        JS::RootedValue iwidth(cx);
        JS::RootedValue iheight(cx);
        JS::RootedValue idata(cx);
        uint32_t dwidth, dheight;

        if (!JS_GetProperty(cx, obj, "width", &iwidth)) {
            return false;
        }
        if (!JS_GetProperty(cx, obj, "height", &iheight)) {
            return false;
        }
        if (!JS_GetProperty(cx, obj, "data", &idata)) {
            return false;
        }

        dwidth = iwidth.toInt32();
        dheight = iheight.toInt32();

        JS_WriteUint32Pair(w, NATIVE_SCTAG_IMAGEDATA,
            (sizeof(uint32_t) * 2) + dwidth * dheight * 4);

        JS_WriteBytes(w, &dwidth, sizeof(uint32_t));
        JS_WriteBytes(w, &dheight, sizeof(uint32_t));
        JS_WriteTypedArray(w, idata);

        return true;
    }

    return false;
}

JSObject *NativeContext::readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                       uint32_t tag, uint32_t data, void *closure)
{
    switch (tag) {
        case NATIVE_SCTAG_IMAGEDATA:
        {
            if (data < sizeof(uint32_t) * 2 + 1) {
                return JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr());
            }
            uint32_t width, height;
            JS::RootedValue arr(cx);

            JS_ReadBytes(r, &width, sizeof(uint32_t));
            JS_ReadBytes(r, &height, sizeof(uint32_t));

            JS_ReadTypedArray(r, &arr);

            JS::RootedObject dataObject(cx, JS_NewObject(cx,  NativeCanvas2DContext::ImageData_jsclass, JS::NullPtr(), JS::NullPtr()));
            JS_DefineProperty(cx, dataObject, "width", UINT_TO_JSVAL(width),
                nullptr, nullptr,
                JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

            JS_DefineProperty(cx, dataObject, "height", UINT_TO_JSVAL(height),
                nullptr, nullptr,
                JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

            JS_DefineProperty(cx, dataObject, "data", arr,
               nullptr, nullptr,
               JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

            return dataObject;
        }
        default:
            break;
    }
    return JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr());
}

void NativeContext::addInputEvent(NativeInputEvent *ev)
{
    if (m_InputEvents.head == NULL) {
        m_InputEvents.head = ev;
    }

    if (m_InputEvents.queue) {
        m_InputEvents.queue->m_Next = ev;
    }

    m_InputEvents.queue = ev;
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
