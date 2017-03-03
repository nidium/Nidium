/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSAudioContext.h"
#include "Binding/JSAudioNode.h"
#include "Binding/JSConsole.h"

#include "Core/Utils.h"
#include "AV/Audio.h"
#include "Frontend/Context.h"

using namespace Nidium::AV;
using namespace Nidium::Core;

namespace Nidium {
namespace Binding {

JSAudioContext *JSAudioContext::m_Ctx = nullptr;

extern void
reportError(JSContext *cx, const char *message, JSErrorReport *report);

static JSClass Global_AudioThread_class = {
    "_GLOBALAudioThread",
    JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL | JSCLASS_HAS_RESERVED_SLOTS(1),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    JS_GlobalObjectTraceHook
};


JSAudioContext::JSAudioContext(JSContext *cx, Audio *audio)
    : m_Audio(audio), m_JsGlobalObj(nullptr)
{
    JSAudioContext::m_Ctx = this;

    JSAudioContext::CreateObject(cx, this);

    this->root();

    NIDIUM_PTHREAD_VAR_INIT(&m_ShutdownWait)

    m_Audio->postMessage(JSAudioContext::CtxCallback, static_cast<void *>(this),
                         true);
}

JSAudioContext *JSAudioContext::GetContext()
{
    return JSAudioContext::m_Ctx;
}

JSAudioContext *JSAudioContext::GetContext(JSContext *cx,
                                           unsigned int bufferSize,
                                           unsigned int channels,
                                           unsigned int sampleRate)
{
    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(cx));
    Audio *audio;

    try {
        audio = new Audio(net, bufferSize, channels, sampleRate);
    } catch (...) {
        return NULL;
    }

    return new JSAudioContext(cx, audio);
}

void JSAudioContext::RunCallback(void *custom)
{
    JSAudioContext *audio = JSAudioContext::GetContext();
    if (!audio) return;

    char *str = static_cast<char *>(custom);
    audio->run(str);
    JS_free(audio->getJSContext(), custom);
}

bool JSAudioContext::JS_run(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString fn(cx);
    JS::RootedFunction nfn(cx);
    if ((nfn = JS_ValueToFunction(cx, args[0])) == NULL
        || (fn = JS_DecompileFunction(cx, nfn, 0)) == NULL) {
        JS_ReportError(cx, "Failed to read callback function\n");
        return false;
    }

    char *funStr = JS_EncodeString(cx, fn);
    if (!funStr) {
        JS_ReportError(cx,
                       "Failed to convert callback function to source string");
        return false;
    }

    m_Audio->postMessage(JSAudioContext::RunCallback,
                         static_cast<void *>(funStr));

    return true;
}

bool JSAudioContext::JS_load(JSContext *cx, JS::CallArgs &args)
{
    JS_ReportError(cx, "Not implemented");
    return false;
}

bool JSAudioContext::JS_createNode(JSContext *cx, JS::CallArgs &args)
{
    int in, out;
    JS::RootedString name(cx);
    JSAudioNode *node = nullptr;
    JS::RootedObject nodeObj(cx);

    if (!JS_ConvertArguments(cx, args, "Suu", name.address(), &in, &out)) {
        return false;
    }

    if (in == 0 && out == 0) {
        JS_ReportError(cx, "Node must have at least one input or output");
        return false;
    } else if (in < 0 || out < 0) {
        JS_ReportError(cx,
                       "Wrong channel count (Must be greater or equal to 0)");
        return false;
    } else if (in > 32 || out > 32) {
        JS_ReportError(cx,
                       "Wrong channel count (Must be lower or equal to 32)");
        return false;
    }

    JSAutoByteString cname(cx, name);

    try {
        if (strcmp("source", cname.ptr()) == 0) {
            JSAudioNodeSource *tmp = new JSAudioNodeSource(cx, out);
            node                   = tmp;
            nodeObj = tmp->getJSObject();
        } else if (strcmp("custom-source", cname.ptr()) == 0) {
            JSAudioNodeCustomSource *tmp = new JSAudioNodeCustomSource(cx, out);
            node                         = tmp;
            nodeObj = tmp->getJSObject();
        } else if (strcmp("custom", cname.ptr()) == 0) {
            JSAudioNodeCustom *tmp = new JSAudioNodeCustom(cx, in, out);
            node                   = tmp;
            nodeObj = tmp->getJSObject();
        } else if (strcmp("reverb", cname.ptr()) == 0) {
            JSAudioNodeReverb *tmp = new JSAudioNodeReverb(cx, in, out);
            node                   = tmp;
            nodeObj = tmp->getJSObject();
        } else if (strcmp("delay", cname.ptr()) == 0) {
            JSAudioNodeDelay *tmp = new JSAudioNodeDelay(cx, in, out);
            node                  = tmp;
            nodeObj = tmp->getJSObject();
        } else if (strcmp("gain", cname.ptr()) == 0) {
            JSAudioNodeGain *tmp = new JSAudioNodeGain(cx, in, out);
            node                 = tmp;
            nodeObj = tmp->getJSObject();
        } else if (strcmp("target", cname.ptr()) == 0) {
            if (m_Target != nullptr) {
                node    = m_Target;
                nodeObj = m_Target->getJSObject();
            } else {
                JSAudioNodeTarget *tmp = new JSAudioNodeTarget(cx, in);
                node                   = tmp;
                nodeObj                = tmp->getJSObject();
                m_Target               = tmp;
            }
        } else if (strcmp("stereo-enhancer", cname.ptr()) == 0) {
            JSAudioNodeStereoEnhancer *tmp
                = new JSAudioNodeStereoEnhancer(cx, in, out);
            node    = tmp;
            nodeObj = tmp->getJSObject();
        } else {
            JS_ReportError(cx, "Unknown node : %s\n", cname.ptr());
            return false;
        }
    } catch (AudioNodeException *e) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", e->what());
        return false;
    }

    args.rval().setObjectOrNull(nodeObj);

    return true;
}

bool JSAudioContext::JS_connect(JSContext *cx, JS::CallArgs &args)
{
    JSAudioNodeLink *jlink1 = nullptr;
    JSAudioNodeLink *jlink2 = nullptr;

    NodeLink *link1 = nullptr;
    NodeLink *link2 = nullptr;

    Audio *audio = this->m_Audio;

    JS::RootedObject arg0(cx, args[0].isObject() ? args[0].toObjectOrNull()
                                                 : nullptr);
    JS::RootedObject arg1(cx, args[1].isObject() ? args[1].toObjectOrNull()
                                                 : nullptr);

    if (!arg0 || !(jlink1 = JSAudioNodeLink::GetInstance(arg0))) {
        JS_ReportError(cx, "First argument must be an AudioNodeLink");
        return false;
    }

    if (!arg1 || !(jlink2 = JSAudioNodeLink::GetInstance(arg1))) {
        JS_ReportError(cx, "Second argument must be an AudioNodeLink");
        return false;
    }

    link1 = jlink1->get();
    link2 = jlink2->get();

    if (!link1 || !link2) {
        JS_ReportError(cx, "Invalid AudioNodeLink");
        return false;
    }

    if (link1->isInput() && link2->isOutput()) {
        if (!audio->connect(link2, link1)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return false;
        }
    } else if (link1->isOutput() && link2->isInput()) {
        if (!audio->connect(link1, link2)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return false;
        }
    } else {
        JS_ReportError(cx, "connect() expect one input and one output.\n");
        return false;
    }

    return true;
}

bool JSAudioContext::JS_disconnect(JSContext *cx, JS::CallArgs &args)
{
    JSAudioNodeLink *jlink1;
    JSAudioNodeLink *jlink2;

    NodeLink *link1;
    NodeLink *link2;

    Audio *audio = this->m_Audio;

    JS::RootedObject arg0(cx, args[0].isObject() ? args[0].toObjectOrNull()
                                                 : nullptr);
    JS::RootedObject arg1(cx, args[1].isObject() ? args[1].toObjectOrNull()
                                                 : nullptr);

    if (!(arg0 || (jlink1 = JSAudioNodeLink::GetInstance(arg0)))) {
        JS_ReportError(cx, "First argument must be an AudioNodeLink");
        return false;
    }

    if (!(arg1 || (jlink2 = JSAudioNodeLink::GetInstance(arg1)))) {
        JS_ReportError(cx, "Second argument must be an AudioNodeLink");
        return false;
    }

    link1 = jlink1->get();
    link2 = jlink2->get();

    if (!link1 || !link2) {
        JS_ReportError(cx, "Invalid AudioNodeLink");
        return false;
    }

    if (link1->isInput() && link2->isOutput()) {
        audio->disconnect(link2, link1);
    } else if (link1->isOutput() && link2->isInput()) {
        audio->disconnect(link1, link2);
    } else {
        JS_ReportError(cx, "disconnect() expect one input and one output\n");
        return false;
    }

    return true;
}

bool JSAudioContext::JS_pFFT(JSContext *cx, JS::CallArgs &args)
{
    int dir, n;
    double *dx, *dy;
    uint32_t dlenx, dleny;

    JS::RootedObject x(cx);
    JS::RootedObject y(cx);
    if (!JS_ConvertArguments(cx, args, "ooii", x.address(), y.address(), &n,
                             &dir)) {
        return false;
    }

    if (!JS_IsTypedArrayObject(x) || !JS_IsTypedArrayObject(y)) {
        JS_ReportError(cx, "Bad argument");
        return false;
    }

    bool shared;

    if (JS_GetObjectAsFloat64Array(x, &dlenx, &shared, &dx) == NULL) {
        JS_ReportError(cx, "Cannot convert typed array (expected Float64Array)");
        return false;
    }
    if (JS_GetObjectAsFloat64Array(y, &dleny, &shared, &dy) == NULL) {
        JS_ReportError(cx, "Cannot convert typed array (expected Float64Array)");
        return false;
    }

    if (dlenx != dleny) {
        JS_ReportError(cx, "Buffers size must match");
        return false;
    }

    if ((n & (n - 1)) != 0 || n < 32 || n > 4096) {
        JS_ReportError(cx, "Invalid frame size");
        return false;
    }

    if (n > dlenx) {
        JS_ReportError(cx, "Buffer is too small");
        return false;
    }

    Utils::FFT(dir, n, dx, dy);

    return true;
}

bool JSAudioContext::JSGetter_bufferSize(JSContext *cx,
                                         JS::MutableHandleValue vp)
{
    vp.setInt32(m_Audio->m_OutputParameters->m_FramesPerBuffer);
    return true;
}

bool JSAudioContext::JSGetter_channels(JSContext *cx, JS::MutableHandleValue vp)
{
    AudioParameters *params = m_Audio->m_OutputParameters;
    vp.setInt32(params->m_Channels);

    return true;
}

bool JSAudioContext::JSGetter_sampleRate(JSContext *cx,
                                         JS::MutableHandleValue vp)
{
    AudioParameters *params = m_Audio->m_OutputParameters;
    vp.setInt32(params->m_SampleRate);

    return true;
}

bool JSAudioContext::JSGetter_volume(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setNumber(m_Audio->getVolume());

    return true;
}

bool JSAudioContext::JSSetter_volume(JSContext *cx, JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Audio->setVolume((float)vp.toNumber());
    }

    return true;
}

bool JSAudioContext::createContext()
{
    if (m_JsRt != NULL) return false;

    if ((m_JsRt = JS_NewRuntime(JS::DefaultHeapMaxBytes, JS::DefaultNurseryBytes))
        == NULL) {
        NUI_LOG("Failed to init JS runtime\n");
        return false;
    }

    NidiumJS::SetJSRuntimeOptions(m_JsRt);

    JS_SetGCParameter(m_JsRt, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetGCParameter(m_JsRt, JSGC_SLICE_TIME_BUDGET, 15);

    if ((m_JsTcx = JS_NewContext(m_JsRt, 8192)) == NULL) {
        NUI_LOG("Failed to init JS context\n");
        return false;
    }

    NidiumLocalContext::InitJSThread(m_JsRt, m_JsTcx);

    JSAutoRequest ar(m_JsTcx);

    // JS_SetGCParameterForThread(this->tcx, JSGC_MAX_CODE_CACHE_BYTES, 16 *
    // 1024 * 1024);
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_LATEST);

    JS::RootedObject global(
        m_JsTcx, JS_NewGlobalObject(m_JsTcx, &Global_AudioThread_class, nullptr,
                                    JS::DontFireOnNewGlobalHook, options));

    JSAutoCompartment ac(m_JsTcx, global);

    m_JsGlobalObj = global;

    // We don't actually needs to root a global object, but we
    // need to store a reference to the global object in a
    // JS::Heap and this reference needs to be traced.
    NidiumLocalContext::RootObjectUntilShutdown(m_JsGlobalObj);

    if (!JS_InitStandardClasses(m_JsTcx, global)) {
        NUI_LOG("Failed to init std class\n");
        return false;
    }
    JS_SetErrorReporter(m_JsRt, reportError);
    JS_FireOnNewGlobalObject(m_JsTcx, global);
    JSConsole::RegisterObject(m_JsTcx);
    JSAudioNodeThreaded::RegisterObject(m_JsTcx);

    return true;
}

bool JSAudioContext::run(char *str)
{
    if (!m_JsTcx) {
        NUI_LOG("No JS context for audio thread\n");
        return false;
    }

    JSAutoRequest ar(m_JsTcx);
    JSAutoCompartment ac(m_JsTcx, m_JsGlobalObj);

    JS::CompileOptions options(m_JsTcx);
    JS::RootedObject globalObj(m_JsTcx, JS::CurrentGlobalOrNull(m_JsTcx));
    options.setIntroductionType("audio Thread").setUTF8(true);

    JS::RootedFunction fun(m_JsTcx);
    JS::AutoObjectVector scopeChain(m_JsTcx);

    bool state = JS::CompileFunction(m_JsTcx, scopeChain, options,
                                    "Audio_run", 0, nullptr,
                                    str, strlen(str), &fun);

    if (!state) {
        JS_ReportError(m_JsTcx, "Failed to compile script on audio thread\n");
        return false;
    }

    JS::RootedValue rval(m_JsTcx);
    JS_CallFunction(m_JsTcx, globalObj, fun, JS::HandleValueArray::empty(), &rval);

    return true;
}

void JSAudioContext::addNode(JSAudioNode *node)
{
    JSAudioContext::NodeListItem *item
        = new JSAudioContext::NodeListItem(node, nullptr, m_Nodes);

    if (m_Nodes != nullptr) {
        m_Nodes->prev = item;
    }

    m_Nodes = item;
}

void JSAudioContext::removeNode(JSAudioNode *node)
{
    JSAudioContext::NodeListItem *item = m_Nodes;
    while (item != NULL) {
        if (item->curr == node) {
            if (item->prev != NULL) {
                item->prev->next = item->next;
            } else {
                m_Nodes = item->next;
            }

            if (item->next != NULL) {
                item->next->prev = item->prev;
            }

            delete item;

            break;
        }
        item = item->next;
    }
}

void JSAudioContext::ShutdownCallback(void *custom)
{
    JSAudioContext *audio              = static_cast<JSAudioContext *>(custom);

#ifdef DEBUG
    JSAudioContext::NodeListItem *node = audio->m_Nodes;
    while (node != NULL) {
        ndm_log(NDM_LOG_DEBUG, "JSAudioContext", "All nodes should have been destroyed");
        assert(false);
    }
#endif

    NidiumLocalContext::UnrootObject(audio->m_JsGlobalObj);
    audio->m_JsGlobalObj = nullptr;

    NidiumLocalContext *nlc = NidiumLocalContext::Get();
    nlc->shutdown();

    if (audio->m_JsTcx != NULL) {
        JSRuntime *rt = JS_GetRuntime(audio->m_JsTcx);

        JS_DestroyContext(audio->m_JsTcx);
        JS_DestroyRuntime(rt);

        audio->m_JsTcx = NULL;
    }

    NIDIUM_PTHREAD_SIGNAL(&audio->m_ShutdownWait)
}

JSAudioContext::~JSAudioContext()
{
    m_Audio->lockSources();
    m_Audio->lockQueue();

    // Delete all nodes
    JSAudioContext::NodeListItem *node = m_Nodes;
    JSAudioContext::NodeListItem *next = nullptr;
    while (node != nullptr) {
        next = node->next;
        // Node destructor will remove the node
        // from the nodes linked list
        delete node->curr;
        node = next;
    }

    // Clear threaded js context
    m_Audio->postMessage(JSAudioContext::ShutdownCallback, this, true);

    NIDIUM_PTHREAD_WAIT(&m_ShutdownWait)

    // Unlock the sources thread, so the decode thread can exit
    // when we call Audio::shutdown()
    m_Audio->unlockSources();

    // Shutdown the audio
    m_Audio->shutdown();

    m_Audio->unlockQueue();

    // And delete the audio
    delete m_Audio;

    JSAudioContext::m_Ctx = NULL;
}

void JSAudioContext::CtxCallback(void *custom)
{
    JSAudioContext *audio = static_cast<JSAudioContext *>(custom);

    if (!audio->createContext()) {
        NUI_LOG("Failed to create audio thread context\n");
        // JS_ReportError(jsNode->audio->cx, "Failed to create audio thread
        // context\n");
        // XXX : Cannot report error from another thread?
    }
}


JSFunctionSpec *JSAudioContext::ListMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN(JSAudioContext, run, 1),
            CLASSMAPPER_FN(JSAudioContext, load, 1),
            CLASSMAPPER_FN(JSAudioContext, createNode, 3),
            CLASSMAPPER_FN(JSAudioContext, connect, 2),
            CLASSMAPPER_FN(JSAudioContext, disconnect, 2),
            CLASSMAPPER_FN(JSAudioContext, pFFT, 2),

            JS_FS_END };

    return funcs;
}
JSPropertySpec *JSAudioContext::ListProperties()
{
    static JSPropertySpec props[]
        = { CLASSMAPPER_PROP_GS(JSAudioContext, volume),

            CLASSMAPPER_PROP_G(JSAudioContext, bufferSize),
            CLASSMAPPER_PROP_G(JSAudioContext, channels),
            CLASSMAPPER_PROP_G(JSAudioContext, sampleRate),

            JS_PS_END };

    return props;
}

void JSAudioContext::RegisterObject(JSContext *cx)
{
    JSAudioContext::ExposeClass<0>(cx, "AudioContext");
}

} // namespace Binding
} // namespace Nidium
