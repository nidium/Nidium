/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Frontend/Context.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


#include "Binding/JSCanvas2DContext.h"
#include "Binding/JSDocument.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSWindow.h"
#include "Binding/JSProcess.h"
#include "Binding/JSImageData.h"
#include "Binding/JSNML.h"

#include "Graphics/CanvasHandler.h"
#include "Graphics/SkiaContext.h"
#include "Graphics/GLHeader.h"


#ifdef DEBUG
#include "Binding/JSDebug.h"
#endif

#ifdef NIDIUM_AUDIO_ENABLED
#include "Binding/JSAudio.h"
#include "Binding/JSAudioContext.h"
#include "Binding/JSAudioNode.h"
#include "Binding/JSVideo.h"
#endif

#ifdef NIDIUM_WEBGL_ENABLED
#include "Graphics/GLHeader.h"
#include "Graphics/GLState.h"
#include "Binding/JSWebGL.h"
#endif

#include "IO/EmbedStream.h"
#include "IO/SystemStream.h"
#include "Interface/UIInterface.h"

using Nidium::Core::SharedMessages;
using Nidium::Core::Path;
using Nidium::Core::Utils;

using Nidium::Interface::UIInterface;
using Nidium::Graphics::GLState;

using namespace Nidium::Graphics;
using namespace Nidium::Binding;
using namespace Nidium::IO;

namespace Nidium {
namespace Frontend {

enum
{
    NIDIUM_SCTAG_IMAGEDATA = NidiumJS::kSctag_Max,
};

// {{{ Logging
void Context::log(const char *str)
{
    UIInterface::UIConsole *console = Interface::__NidiumUI->getConsole(true);
    if (console && !console->hidden()) {
        console->log(str);
    } else {
        Core::Context::log(str);
    }
}

void Context::logClear()
{
    UIInterface::UIConsole *console = Interface::__NidiumUI->getConsole(true);
    if (console && !console->hidden()) {
        console->clear();
    }
}

void Context::logShow()
{
    UIInterface::UIConsole *console = Interface::__NidiumUI->getConsole(true);
    if (console && console->hidden()) {
        console->show();
    }
}

void Context::logHide()
{
    UIInterface::UIConsole *console = Interface::__NidiumUI->getConsole(true);
    if (console && !console->hidden()) {
        console->hide();
    }
}
// }}}

void Context::initStats()
{
    m_Stats.nframe           = 0;
    m_Stats.starttime        = Utils::GetTick();
    m_Stats.lastmeasuredtime = m_Stats.starttime;
    m_Stats.lastdifftime     = 0;
    m_Stats.cumulframe       = 0;
    m_Stats.cumultimems      = 0.f;
    m_Stats.fps              = 0.f;
    m_Stats.minfps           = UINT32_MAX;
    m_Stats.sampleminfps     = 0.f;

    memset(m_Stats.samples, 0, sizeof(m_Stats.samples));
}


Context::Context(ape_global *net)
    : Core::Context(net), m_RootHandler(NULL), m_DebugHandler(NULL),
#if DEBUG
      m_Debug2Handler(NULL),
#endif
      m_UI(NULL), m_NML(NULL), m_GLState(NULL),
      m_JSWindow(NULL), m_SizeDirty(false)
{

    Binding::NidiumLocalContext *nlc = Binding::NidiumLocalContext::Get();
    nlc->ptr = (void *)new LocalContext();

#ifdef NIDIUM_PACKAGE_EMBED
    // When nidium is packaged with the embedded resources, the embed://
    // prefix should be kept for correctly resolving the path of the files.
    Path::RegisterScheme(SCHEME_DEFINE("embed://", EmbedStream, true));
#else
    Path::RegisterScheme(SCHEME_DEFINE("embed://", EmbedStream, false));
#endif

    Path::RegisterScheme(SCHEME_DEFINE("system://", SystemStream, false));
    Path::RegisterScheme(SCHEME_DEFINE("user://", UserStream, false));
    Path::RegisterScheme(SCHEME_DEFINE("private://", PrivateStream, false));

    ape_init_pool_list(&m_CanvasEventsCanvas, 0, 8);

    m_JS->setStructuredCloneAddition(Context::WriteStructuredCloneOp,
                                     Context::ReadStructuredCloneOp);

    JS::RootedObject globalObj(m_JS->m_Cx, JS::CurrentGlobalOrNull(m_JS->m_Cx));
    //JS_InitReflect(m_JS->m_Cx, globalObj);

    m_Jobs.head  = NULL;
    m_Jobs.queue = NULL;
}


void Context::setUIObject(Interface::UIInterface *ui)
{
    m_UI  = ui;
    m_NML = m_UI->m_Nml;

    GLState::CreateForContext(this);

    this->initStats();
    this->initShaderLang();
    this->initHandlers(ui->getWidth(), ui->getHeight());
    this->loadNativeObjects(ui->getWidth(), ui->getHeight());
}


void Context::loadNativeObjects(int width, int height)
{
    assert(m_UI != NULL);

    JSContext *cx = m_JS->m_Cx;

    Canvas2DContext::RegisterObject(cx);
    JSCanvas::RegisterObject(cx);
    JSImage::RegisterObject(cx);
    JSImageData::RegisterObject(cx);
    JSNML::RegisterObject(cx);
#ifdef NIDIUM_AUDIO_ENABLED
    JSAudio::RegisterAllObjects(cx);
    JSVideo::RegisterAllObjects(cx);
#endif
#ifdef NIDIUM_WEBGL_ENABLED
    JSWebGLRenderingContext::RegisterAllObjects(cx);
#endif
    JS::RootedObject docObj(cx, JSDocument::RegisterObject(cx));
    m_JSWindow = JSWindow::RegisterObject(cx, width, height, docObj);

    JSProcess::RegisterObject(cx, m_UI->m_Argv, m_UI->m_Argc, 0);


#if DEBUG
    createDebug2Canvas();
#endif
}

void Context::setWindowSize(int w, int h)
{
    assert(m_UI != NULL);

    m_SizeDirty = true;
    /* OS window */
    m_UI->setWindowSize((int)w, (int)h);
    /* Root canvases */
    this->sizeChanged(w, h);
}

void Context::setWindowFrame(int x, int y, int w, int h)
{
    assert(m_UI != NULL);

    m_SizeDirty = true;
    /* OS window */
    m_UI->setWindowFrame(static_cast<int>(x), static_cast<int>(y),
                         static_cast<int>(w), static_cast<int>(h));
    /* Root canvases */
    this->sizeChanged(w, h);
}

void Context::sizeChanged(int w, int h)
{
    assert(m_UI != NULL);

    if (!m_SizeDirty) {
        return;
    }

    m_SizeDirty = false;

    /* Skia GL */
    this->getRootHandler()->setSize(static_cast<int>(w), static_cast<int>(h));
    /* Nidium Canvas */
    m_JSWindow->getCanvasHandler()->setSize(static_cast<int>(w),
                                            static_cast<int>(h));
    /* Redraw */
    m_UI->refresh();
}

void Context::createDebugCanvas()
{
    Canvas2DContext *context
        = static_cast<Canvas2DContext *>(m_RootHandler->getContext());
    static const int DEBUG_HEIGHT = 60;
    m_DebugHandler = new CanvasHandler(context->getSkiaContext()->getWidth(),
                                       DEBUG_HEIGHT, this);
    Canvas2DContext *ctx2d
        = new Canvas2DContext(m_DebugHandler, context->getSkiaContext()->getWidth(),
                              DEBUG_HEIGHT, NULL, false);
    m_DebugHandler->setContext(ctx2d);
    ctx2d->setGLState(this->getGLState());

    m_RootHandler->addChild(m_DebugHandler);

    m_DebugHandler->setRight(0);
    m_DebugHandler->setOpacity(0.6);
    ctx2d->getSkiaContext()->setFontType("monospace");
}

#if DEBUG
void Context::createDebug2Canvas()
{
    Canvas2DContext *context
        = static_cast<Canvas2DContext *>(m_RootHandler->getContext());
    static const int DEBUG_HEIGHT = 60;
    m_Debug2Handler = new CanvasHandler(context->getSkiaContext()->getWidth(),
                                        DEBUG_HEIGHT, this);
    Canvas2DContext *ctx2d = new Canvas2DContext(
        m_Debug2Handler, context->getSkiaContext()->getWidth(), DEBUG_HEIGHT, NULL,
        false);
    m_Debug2Handler->setContext(ctx2d);
    ctx2d->setGLState(this->getGLState());

    m_RootHandler->addChild(m_Debug2Handler);
    m_Debug2Handler->unsetTop();
    m_Debug2Handler->setRight(0);
    m_Debug2Handler->setBottom(0);
}
#endif

void Context::postDraw()
{
    if (JSDocument::m_ShowFPS && m_DebugHandler) {

        SkiaContext *s
            = (static_cast<Canvas2DContext *>(m_DebugHandler->getContext())
                   ->getSkiaContext());
        m_DebugHandler->bringToFront();

        s->setFillColor(0xFF000000u);
        s->drawRect(0, 0, m_DebugHandler->getWidth(),
                    m_DebugHandler->getHeight(), 0);
        s->setFillColor(0xFFEEEEEEu);
        
        s->drawTextf(5, 12, "Nidium build %s %s", __DATE__, __TIME__);
        s->drawTextf(5, 25, "Frame: %lld (%lldms)", m_Stats.nframe,
                     m_Stats.lastdifftime / 1000000LL);
        s->drawTextf(5, 38, "Time : %lldns",
                     m_Stats.lastmeasuredtime - m_Stats.starttime);
        s->drawTextf(5, 51, "FPS  : %.2f (%.2f)", m_Stats.fps,
                     m_Stats.sampleminfps);

        s->setLineWidth(0.0);

        for (int i = 0; i < sizeof(m_Stats.samples) / sizeof(float); i++) {
            // s->drawLine(300 + i * 3, 55, 300 + i * 3, (40 / 60) *
            // m_Stats.samples[i]);
            s->setStrokeColor(0xFF004400u);
            s->drawLine(m_DebugHandler->getWidth() - 20 - i * 3, 55,
                        m_DebugHandler->getWidth() - 20 - i * 3, 20.f);
            s->setStrokeColor(0xFF00BB00u);
            s->drawLine(
                m_DebugHandler->getWidth() - 20 - i * 3, 55,
                m_DebugHandler->getWidth() - 20 - i * 3,
                nidium_min(60 - ((40.f / 62.f)
                                 * static_cast<float>(m_Stats.samples[i])),
                           55));
        }
        // s->setLineWidth(1.0);

        // s->translate(10, 10);
        // sprintf(fps, "%d fps", currentFPS);
        // s->system(fps, 5, 10);
        s->flush();
    }
#if DEBUG
    if (m_Debug2Handler) {
        m_Debug2Handler->bringToFront();
        m_Debug2Handler->getContext()->clear();
        SkiaContext *rootctx
            = (static_cast<Canvas2DContext *>(m_Debug2Handler->getContext())
                   ->getSkiaContext());
        rootctx->save();

        rootctx->setFillColor("black");
        rootctx->drawText("DEBUG build", 10, 30);
        rootctx->setFillColor("white");
        rootctx->drawText("DEBUG build", 10, 45);
        rootctx->restore();
        rootctx->flush();
    }
#endif
}

/* TODO, move out */
void Context::callFrame()
{
    assert(m_JSWindow != NULL);

    uint64_t tmptime = Utils::GetTick();
    m_Stats.nframe++;

    m_Stats.lastdifftime     = tmptime - m_Stats.lastmeasuredtime;
    m_Stats.lastmeasuredtime = tmptime;

    /* convert to ms */
    m_Stats.cumultimems += static_cast<float>(m_Stats.lastdifftime) / 1000000.f;
    m_Stats.cumulframe++;

    m_Stats.minfps = nidium_min(m_Stats.minfps,
                                1000.f / (m_Stats.lastdifftime / 1000000.f));
    // printf("FPS : %f\n", 1000.f/(m_Stats.lastdifftime/1000000.f));

    // printf("Last diff : %f\n",
    // static_cast<float>(m_Stats.lastdifftime/1000000.f));

    /* Sample every 1000ms */
    if (m_Stats.cumultimems >= 1000.f) {
        m_Stats.fps = 1000.f / static_cast<float>(m_Stats.cumultimems)
                      / static_cast<float>(m_Stats.cumulframe);
        m_Stats.cumulframe   = 0;
        m_Stats.cumultimems  = 0.f;
        m_Stats.sampleminfps = m_Stats.minfps;
        m_Stats.minfps       = UINT32_MAX;

        memmove(&m_Stats.samples[1], m_Stats.samples,
                sizeof(m_Stats.samples) - sizeof(float));

        m_Stats.samples[0] = m_Stats.fps;
    }

    m_JSWindow->callFrameCallbacks(tmptime);
}


void Context::rendered(uint8_t *pdata, int width, int height)
{
#if 0
    if (m_WSClient) {
        m_WSClient->write(static_cast<unsigned char *>(pdata),
                          width * height * 4, true);
    }
#endif
}

void Context::frame(bool draw)
{
    LayerizeContext ctx;
    LayerSiblingContext sctx;
    Canvas2DContext *rootctx;
    std::vector<ComposeContext> compList;

    ctx.reset();
    ctx.m_SiblingCtx = &sctx;

    rootctx = (Canvas2DContext *)m_RootHandler->m_Context;

    assert(m_UI != NULL);
    // this->execJobs();
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
    m_CanvasOrderedEvents.clear();

    /* Build the composition list */
    m_RootHandler->layerize(ctx, compList, draw);

    m_UI->makeMainGLCurrent();
    rootctx->clear(0xFFFFFFFF);
    rootctx->flush();

    /*
        Compose canvas eachother on the main framebuffer
    */
    m_RootHandler->getContext()->flush();
    m_RootHandler->getContext()->resetGLContext();
    /* We draw on the UI fbo */
    glBindFramebuffer(GL_FRAMEBUFFER, m_UI->getFBO());
    for (auto &com : compList) {
        com.handler->m_Context->preComposeOn(rootctx, com.left,
            com.top, com.opacity, com.zoom, com.needClip ? &com.clip : nullptr);
    }

    this->triggerEvents();
    m_InputHandler.clear();

    m_UI->makeMainGLCurrent();
    /* Skia context is dirty after a call to layerize */
    (static_cast<Canvas2DContext *>(m_RootHandler->getContext()))->getSkiaContext()->resetGrBackendContext();
}

void NidiumContext_destroy_and_handle_events(ape_pool_t *pool, void *ctx)
{
    if (!pool->ptr.data) {
        return;
    }
    InputEvent *ev = static_cast<InputEvent *>(pool->ptr.data);

    /* top-most element */
    if (ev->getDepth() == ev->m_Origin->getDepth()) {
        ev->m_Handler->_handleEvent(ev);
    }

    delete ev;
}

void Context::triggerEvents()
{
    void *val;

    APE_P_FOREACH((&m_CanvasEventsCanvas), val)
    {
        /* process through the cleaner callback avoiding a complete iteration */
        ape_destroy_pool_list_ordered((ape_pool_list_t *)val,
                                      NidiumContext_destroy_and_handle_events,
                                      NULL);
        __pool_item->ptr.data = NULL;
    }

    /*
        Reset the 'push' pointer.
    */
    ape_pool_rewind(&m_CanvasEventsCanvas);
}

// From Mozilla gfx/gl/GLContext.cpp
static int GetGLSLVersion()
{
    /**
     * OpenGL 2.x, 3.x, 4.x specifications:
     *  The VERSION and SHADING_LANGUAGE_VERSION strings are laid out as
     * follows:
     *
     *    <version number><space><vendor-specific information>
     *
     *  The version number is either of the form major_number.minor_number or
     *  major_number.minor_number.release_number, where the numbers all have
     *  one or more digits.
     *
     * SHADING_LANGUAGE_VERSION is *almost* identical to VERSION. The
     * difference is that the minor version always has two digits and the
     * prefix has an additional 'GLSL ES'
     *
     *
     * OpenGL ES 2.0, 3.0 specifications:
     *  The VERSION string is laid out as follows:
     *
     *     "OpenGL ES N.M vendor-specific information"
     *
     *  The version number is either of the form major_number.minor_number or
     *  major_number.minor_number.release_number, where the numbers all have
     *  one or more digits.
     *
     *
     * Note:
     *  We don't care about release_number.
     */
    const unsigned char *tmp;
    const char *versionString;
    int err;

    NIDIUM_GL_CALL_RET_MAIN(GetError(), err);
    NIDIUM_GL_CALL_RET_MAIN(GetString(GL_SHADING_LANGUAGE_VERSION), tmp);

    NIDIUM_GL_CALL_RET_MAIN(GetError(), err);

    if (err != GL_NO_ERROR) {
        NUI_LOG("Failed to parse GL's major-minor version number separator.\n");
        return -1;
    }

    versionString = (const char *)tmp;

    if (!versionString) {
        // This happens on the Android emulators. We'll just return 100
        return 100;
    }

    const auto fnSkipPrefix = [&versionString](const char *prefix) {
        const auto len = strlen(prefix);
        if (strncmp(versionString, prefix, len) == 0) {
            versionString += len;
        }
    };

    const char kGLESVersionPrefix[] = "OpenGL ES GLSL ES";
    fnSkipPrefix(kGLESVersionPrefix);

    const char *itr   = versionString;
    char *end         = nullptr;
    auto majorVersion = strtol(itr, &end, 10);

    if (!end) {
        NUI_LOG("Failed to parse the GL major version number.\n");
        return -1;
    }

    if (*end != '.') {
        NUI_LOG("Failed to parse GL's major-minor version number separator.\n");
        return -1;
    }

    // we skip the '.' between the major and the minor version
    itr = end + 1;
    end = nullptr;

    auto minorVersion = strtol(itr, &end, 10);
    if (!end) {
        NUI_LOG("Failed to parse GL's minor version number.\n");
        return -1;
    }

    if (majorVersion <= 0 || majorVersion >= 100) {
        NUI_LOG("Invalid major version.\n");
        return false;
    }

    if (minorVersion < 0 || minorVersion >= 100) {
        NUI_LOG("Invalid minor version.\n");
        return false;
    }

    return (uint32_t)majorVersion * 100 + (uint32_t)minorVersion;
}

// From Mozilla dom/canvas/WebGLShaderValidator.cpp
static ShShaderOutput GetShaderOutputVersion()
{
#ifdef NIDIUM_OPENGLES2
    return SH_ESSL_OUTPUT;
#else
    int version = GetGLSLVersion();
    switch (version) {
        case 100:
            return SH_GLSL_COMPATIBILITY_OUTPUT;
        case 120:
            return SH_GLSL_COMPATIBILITY_OUTPUT;
        case 130:
            return SH_GLSL_130_OUTPUT;
        case 140:
            return SH_GLSL_140_OUTPUT;
        case 150:
            return SH_GLSL_150_CORE_OUTPUT;
        case 330:
            return SH_GLSL_330_CORE_OUTPUT;
        case 400:
            return SH_GLSL_400_CORE_OUTPUT;
        case 410:
            return SH_GLSL_410_CORE_OUTPUT;
        case 420:
            return SH_GLSL_420_CORE_OUTPUT;
        case 430:
            return SH_GLSL_430_CORE_OUTPUT;
        case 440:
            return SH_GLSL_440_CORE_OUTPUT;
        case 450:
            return SH_GLSL_450_CORE_OUTPUT;
        default:
            NUI_LOG("Unexpected GLSL version.\n");
            return SH_GLSL_COMPATIBILITY_OUTPUT;
    }
#endif

}

// From Mozilla canvas/src/WebGLContextValidate.cpp
// TODO : Handle OpenGL ESJSVAL_
bool Context::initShaderLang()
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
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
                  &maxVertexTextureImageUnits);

#ifdef NIDIUM_OPENGLES2
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxFragmentUniformVectors);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniformVectors);
#else
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
                  &maxFragmentUniformVectors);
    maxFragmentUniformVectors /= 4;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniformVectors);
    maxVertexUniformVectors /= 4;
#endif

    m_ShShaderOutput = GetShaderOutputVersion();

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

    m_ShResources.MaxVertexAttribs             = maxVertexAttribs;
    m_ShResources.MaxVertexUniformVectors      = maxVertexUniformVectors;
    m_ShResources.MaxVaryingVectors            = 16;
    m_ShResources.MaxVertexTextureImageUnits   = maxVertexTextureImageUnits;
    m_ShResources.MaxCombinedTextureImageUnits = maxTextureImageUnits;
    m_ShResources.MaxTextureImageUnits         = maxTextureImageUnits;
    m_ShResources.MaxFragmentUniformVectors    = maxFragmentUniformVectors;
    m_ShResources.MaxDrawBuffers               = 1;

    m_ShResources.FragmentPrecisionHigh = 1;

    // FIXME : Check if extension is supported and enable or not
    m_ShResources.OES_standard_derivatives = 1;
    m_ShResources.OES_EGL_image_external   = 0;
    m_ShResources.EXT_shader_texture_lod   = 1;

    return true;
}

void Context::initHandlers(int width, int height)
{
    assert(m_UI != NULL);

    m_RootHandler = new CanvasHandler(width, height, this);

    m_RootHandler->setContext(
        new Canvas2DContext(m_RootHandler, width, height, m_UI));
    m_RootHandler->getContext()->setGLState(this->getGLState());
}

void Context::addJob(void (*job)(void *arg), void *arg)
{
    struct JobQueue *obj
        = static_cast<struct JobQueue *>(malloc(sizeof(struct JobQueue)));

    obj->job  = job;
    obj->arg  = arg;
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

void Context::execJobs()
{
    if (m_Jobs.head == NULL) {
        return;
    }

    struct JobQueue *obj, *tObj;

    for (obj = m_Jobs.head; obj != NULL; obj = tObj) {
        tObj = obj->next;

        obj->job(obj->arg);

        free(obj);
    }

    m_Jobs.head  = NULL;
    m_Jobs.queue = NULL;
}

void Context::execPendingCanvasChanges()
{
    ape_htable_item_t *item, *tmpItem;
    for (item = m_CanvasPendingJobs.accessCStruct()->first; item != NULL;
         item = tmpItem) {
        tmpItem = item->lnext;
        CanvasHandler *handler
            = static_cast<CanvasHandler *>(item->content.addrs);
        handler->execPending();
    }
}


bool Context::WriteStructuredCloneOp(JSContext *cx,
                                     JSStructuredCloneWriter *w,
                                     JS::HandleObject obj,
                                     void *closure)
{

    JS::RootedValue vobj(cx, JS::ObjectValue(*obj));
    JSType type = JS_TypeOfValue(cx, vobj);

    if (type != JSTYPE_OBJECT) {
        return false;
    }

    if (JSImageData::InstanceOf(obj)) {
        uint32_t dwidth, dheight;

        JS::RootedValue iwidth(cx);
        if (!JS_GetProperty(cx, obj, "width", &iwidth)) {
            return false;
        }
        JS::RootedValue iheight(cx);
        if (!JS_GetProperty(cx, obj, "height", &iheight)) {
            return false;
        }
        JS::RootedValue idata(cx);
        if (!JS_GetProperty(cx, obj, "data", &idata)) {
            return false;
        }

        dwidth  = iwidth.toInt32();
        dheight = iheight.toInt32();

        JS_WriteUint32Pair(w, NIDIUM_SCTAG_IMAGEDATA,
                           (sizeof(uint32_t) * 2) + dwidth * dheight * 4);

        JS_WriteBytes(w, &dwidth, sizeof(uint32_t));
        JS_WriteBytes(w, &dheight, sizeof(uint32_t));
        JS_WriteTypedArray(w, idata);

        return true;
    }

    return false;
}

JSObject *Context::ReadStructuredCloneOp(JSContext *cx,
                                         JSStructuredCloneReader *r,
                                         uint32_t tag,
                                         uint32_t data,
                                         void *closure)
{
    switch (tag) {
        case NIDIUM_SCTAG_IMAGEDATA: {
            if (data < sizeof(uint32_t) * 2 + 1) {
                JS::RootedObject obj(
                    cx,
                    JS_NewPlainObject(cx));
                return obj;
            }
            uint32_t width, height;

            JS_ReadBytes(r, &width, sizeof(uint32_t));
            JS_ReadBytes(r, &height, sizeof(uint32_t));

            JS::RootedValue arr(cx);
            JS_ReadTypedArray(r, &arr);

            JS::RootedObject dataObject(cx,
              JSImageData::CreateObject(cx, new JSImageData()));

            JS_DefineProperty(cx, dataObject, "width", width,
                              JSPROP_PERMANENT | JSPROP_ENUMERATE
                                  | JSPROP_READONLY);
            JS_DefineProperty(cx, dataObject, "height", height,
                              JSPROP_PERMANENT | JSPROP_ENUMERATE
                                  | JSPROP_READONLY);
            JS_DefineProperty(cx, dataObject, "data", arr,
                              JSPROP_PERMANENT | JSPROP_ENUMERATE
                                  | JSPROP_READONLY);

            return dataObject;
        }
        default:
            break;
    }
    return JS_NewPlainObject(cx);
}

Context::~Context()
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

    delete m_JSWindow;

    /*
        Don't let the base class destroy the JS.
        CanvasHandler are released by the JS engine (unroot) and require
        this context to be available in their destructor.
    */
    destroyJS();

    delete m_GLState;

    SkiaContext::m_GlSurface = nullptr;

    ape_destroy_pool_ordered(m_CanvasEventsCanvas.head, NULL, NULL);
    m_InputHandler.clear();

    ShFinalize();
}

} // namespace Frontend
} // namespace Nidium
