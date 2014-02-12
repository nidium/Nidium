#include "NativeContext.h"
#include "NativeUtils.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"
#include "NativeCanvasContext.h"
#include "NativeGLState.h"
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
#include "SkForceLinking.h"

#include "SkImageDecoder.h"

#include <stdlib.h>
#include <string.h>
#include "NativeMacros.h"

#include "GLSLANG/ShaderLang.h"
#include "NativeMacros.h"
#include "NativeNML.h"

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

jsval gfunc  = JSVAL_VOID;

void NativeContext_Logger(const char *format)
{
    __NativeUI->log(format);
}

void NativeContext_vLogger(const char *format, va_list ap)
{
    __NativeUI->vlog(format, ap);
}


NativeContext::NativeContext(NativeUIInterface *nui, NativeNML *nml,
    int width, int height, ape_global *net) :
    m_DebugHandler(NULL), m_UI(nui), m_NML(nml)
{
    gfunc = JSVAL_VOID;

    ShInitialize();

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
    m_JS->setDelegate(this);

    m_NML->setNJS(m_JS);

    __preloadScripts(&this->preload);

    /*
        TODO: Why is "native.js" hardcoded???
    */
    const char *loadPreload[] = {
        "falcon/native.js",
        "../scripts/preload.js"
    };

    for (int i = 0; i < (sizeof(loadPreload) / sizeof(intptr_t)); i++) {
        NativeBytecodeScript *script = this->preload.get(loadPreload[i]);
        if (script) {
            m_JS->LoadBytecode(script);
        }
    }
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
    /* console() object */
    NativeJSconsole::registerObject(cx);
    /* document() object */
    NativeJSdocument::registerObject(cx);

    //NativeJSDebug::registerObject(cx);    
}

void NativeContext::setWindowSize(int w, int h)
{
    /* OS window */
    m_UI->setWindowSize((int)w, (int)h);
    /* Root canvases */
    this->sizeChanged(w, h);
    /* Redraw */
    m_UI->refresh();
}

void NativeContext::sizeChanged(int w, int h)
{
    NativeJSwindow *jswindow = NativeJSwindow::getNativeClass(m_JS);

    /* Skia GL */
    this->getRootHandler()->setSize((int)w, (int)h);
    /* Native Canvas */
    jswindow->getCanvasHandler()->setSize((int)w, (int)h);
}

void NativeContext::createDebugCanvas()
{
    NativeCanvas2DContext *context = (NativeCanvas2DContext *)m_RootHandler->getContext();
    static const int DEBUG_HEIGHT = 60;
    m_DebugHandler = new NativeCanvasHandler(context->getSurface()->getWidth(), DEBUG_HEIGHT);
    NativeCanvas2DContext *ctx2d =  new NativeCanvas2DContext(m_DebugHandler, context->getSurface()->getWidth(), DEBUG_HEIGHT, NULL, false);
    m_DebugHandler->setContext(ctx2d);
    ctx2d->setGLState(this->getGLState());

    m_RootHandler->addChild(m_DebugHandler);
    m_DebugHandler->setRight(0);
    m_DebugHandler->setOpacity(0.6);
}

void NativeContext::postDraw()
{
    if (NativeJSNative::showFPS && m_DebugHandler) {

        NativeSkia *s = ((NativeCanvas2DContext *)m_DebugHandler->getContext())->getSurface();
        m_DebugHandler->bringToFront();

        s->setFillColor(0xFF000000u);
        s->drawRect(0, 0, m_DebugHandler->getWidth(), m_DebugHandler->getHeight(), 0);
        s->setFillColor(0xFFEEEEEEu);

        s->setFontType("monospace");
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

    NativeSkia::glcontext = NULL;

    ShFinalize();
}

void NativeContext::frame()
{
    this->callFrame();
    this->postDraw();

    m_RootHandler->getContext()->flush();
    m_RootHandler->getContext()->resetGLContext();

    /* We draw on the screen */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_RootHandler->layerize(NULL, 0, 0, 1.0, 1.0, NULL);
    /* Skia context is dirty after a call to layerize */
    ((NativeCanvas2DContext *)m_RootHandler->getContext())->resetSkiaContext();
}

void NativeContext::initHandlers(int width, int height)
{
    m_RootHandler = new NativeCanvasHandler(width, height);
    m_RootHandler->setContext(new NativeCanvas2DContext(m_RootHandler, width, height, m_UI));
    m_RootHandler->getContext()->setGLState(this->getGLState());
}

bool NativeContext::onLoad(NativeJS *njs, char *filename, int argc, jsval *vp)
{
    JSContext *cx = njs->cx;
    int interfaceLen = 0;
    NativeStream::StreamInterfaces interface = 
        NativeStream::typeInterface(filename, &interfaceLen);
    char *file = NativeStream::resolvePath(filename, NativeStream::STREAM_RESOLVE_FILE);
    const char *prefix = njs->getPath();
    char *finalfile;
    
    if (interface == NativeStream::INTERFACE_PRIVATE) {
        finalfile = strdup(file);
    } else {
        finalfile = (char *)malloc(sizeof(char) *
            (1 + strlen(prefix) + strlen(file)));
        sprintf(finalfile, "%s%s", prefix, file);
    }

    // More than 1 argument, assume it's nss loading
    if (argc > 1) {
        JS::RootedValue type(cx, JS_ARGV(cx, vp)[1]);
        if (type.isString()) {
            JS::RootedValue ret(cx, JSVAL_NULL);
            if (!NativeJS::LoadScriptReturn(cx, finalfile, ret.address())) {
                JS_ReportError(cx, "Failed to load %s\n", finalfile);
            }

            JS_SET_RVAL(cx, vp, ret);
        }

        free(file);
        free(finalfile);

        return true;
    }

#ifdef NATIVE_EMBED_PRIVATE
    if (interface == NativeStream::INTERFACE_PRIVATE) {
        // XXX : This is a temporary hack until we got VFS working
        // If falcon framework is embeded inside the binary so all call to load()
        // with private:// have to load data from the binary 
        njs->LoadBytecode(this->preload->get(&filename[interfaceLen]));

        free(file);
        free(finalfile);

        return true;
    } 
#endif

    if (!njs->LoadScript(finalfile)) {
        JS_ReportError(cx, "Failed to load %s\n", finalfile);
    }
    
    free(file);
    free(finalfile);
    return true;
}
