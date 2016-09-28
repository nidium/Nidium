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

extern void
reportError(JSContext *cx, const char *message, JSErrorReport *report);

static JSClass Global_AudioThread_class
    = { "_GLOBALAudioThread",
        JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
        JS_PropertyStub,
        JS_DeletePropertyStub,
        JS_PropertyStub,
        JS_StrictPropertyStub,
        JS_EnumerateStub,
        JS_ResolveStub,
        JS_ConvertStub,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        JS_GlobalObjectTraceHook,
        JSCLASS_NO_INTERNAL_MEMBERS };


JSAudioContext::JSAudioContext(Audio *audio)
    : m_Audio(audio), m_Nodes(NULL),
      m_JsGlobalObj(NULL), m_JsRt(NULL), m_JsTcx(NULL), m_Target(NULL)
{
    JSAudioContext::m_Instance = this;

    this->root();

    NIDIUM_PTHREAD_VAR_INIT(&m_ShutdownWait)

    m_Audio->postMessage(JSAudioContext::CtxCallback, static_cast<void *>(this), true);
}

JSAudioContext *JSAudioContext::GetContext()
{
    return JSAudioContext::m_Instance;
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

    audio->setMainCtx(cx);

    return new JSAudioContext(audio);
}

void JSAudioContext::RunCallback(void *custom)
{
    JSAudioContext *audio = JSAudioContext::GetContext();

    if (!audio) return; // This should not happend

    char *str = static_cast<char *>(custom);
    audio->run(str);
    JS_free(audio->getJSContext(), custom);
}

bool JSAudioContext::JS_run(JSContext *cx, JS::CallArgs &args)
{
    JSAudioContext *audio    = JSAudioContext::GetContext();

    CHECK_INVALID_CTX(audio);

    JS::RootedString fn(cx);
    JS::RootedFunction nfn(cx);
    if ((nfn = JS_ValueToFunction(cx, args[0])) == NULL
        || (fn = JS_DecompileFunctionBody(cx, nfn, 0)) == NULL) {
        JS_ReportError(cx, "Failed to read callback function\n");
        return false;
    }

    char *funStr = JS_EncodeString(cx, fn);
    if (!funStr) {
        JS_ReportError(cx,
                       "Failed to convert callback function to source string");
        return false;
    }

    audio->m_Audio->postMessage(JSAudioContext::RunCallback,
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
    JSAudioNode *node;

    node  = NULL;

    JS::RootedString name(cx);
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

    JSClass *audioNode_class = JSAudioNode::GetJSClass();
    JS::RootedObject ret(
        cx, JS_NewObjectForConstructor(cx, audioNode_class, args));
    if (!ret) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    JS_SetReservedSlot(ret, 0, JSVAL_NULL);

    JSAutoByteString cname(cx, name);
    try {
        if (strcmp("source", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::SOURCE, in, out, this);

            AudioSource *source = static_cast<AudioSource *>(node->m_Node);
            source->eventCallback(JSAudioNode::onEvent, node);

            JS_DefineFunctions(cx, ret, AudioNodeSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeSource_props);
        } else if (strcmp("custom-source", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::CUSTOM_SOURCE, in, out,
                                   this);

            AudioSourceCustom *source
                = static_cast<AudioSourceCustom *>(node->m_Node);
            source->eventCallback(JSAudioNode::onEvent, node);

            JS::RootedValue tmp(cx, JS::NumberValue(0));
            JS_DefineProperty(
                cx, ret, "position", tmp,
                JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED
                    | JSPROP_NATIVE_ACCESSORS,
                JS_CAST_NATIVE_TO(
                    nidium_audionode_custom_source_position_getter,
                    JSPropertyOp),
                JS_CAST_NATIVE_TO(
                    nidium_audionode_custom_source_position_setter,
                    JSStrictPropertyOp));

            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
            JS_DefineFunctions(cx, ret, AudioNodeCustomSource_funcs);
        } else if (strcmp("custom", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::CUSTOM, in, out, this);
            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
        } else if (strcmp("reverb", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::REVERB, in, out, this);
        } else if (strcmp("delay", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::DELAY, in, out, this);
        } else if (strcmp("gain", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::GAIN, in, out, this);
        } else if (strcmp("target", cname.ptr()) == 0) {
            if (this->m_Target != NULL) {
                JS::RootedObject retObj(cx, this->m_Target->getJSObject());
                args.rval().setObjectOrNull(retObj);
                return true;
            } else {
                node = new JSAudioNode(Audio::TARGET, in, out, this);
                this->m_Target = node;
            }
        } else if (strcmp("stereo-enhancer", cname.ptr()) == 0) {
            node = new JSAudioNode(Audio::STEREO_ENHANCER, in, out,
                                   this);
        } else {
            JS_ReportError(cx, "Unknown node name : %s\n", cname.ptr());
            return false;
        }
    } catch (AudioNodeException *e) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", e->what());
        return false;
    }

    if (node == NULL || node->m_Node == NULL) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", cname.ptr());
        return false;
    }

    this->initNode(node, ret, name);

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSAudioContext::JS_connect(JSContext *cx, JS::CallArgs &args)
{
    NodeLink *nlink1;
    NodeLink *nlink2;

    Audio *audio = this->m_Audio;

    JS::RootedObject link1(cx);
    JS::RootedObject link2(cx);
    if (!JS_ConvertArguments(cx, args, "oo", link1.address(),
                             link2.address())) {
        return false;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class,
                                               &args);
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class,
                                               &args);

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return false;
    }

    if (nlink1->m_Type == AV::INPUT && nlink2->m_Type == AV::OUTPUT) {
        if (!audio->connect(nlink2, nlink1)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return false;
        }
    } else if (nlink1->m_Type == AV::OUTPUT && nlink2->m_Type == AV::INPUT) {
        if (!audio->connect(nlink1, nlink2)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return false;
        }
    } else {
        JS_ReportError(cx, "connect() take one input and one output\n");
        return false;
    }

    args.rval().setUndefined();

    return true;
}

bool JSAudioContext::JS_disconnect(JSContext *cx, JS::CallArgs &args)
{
    NodeLink *nlink1;
    NodeLink *nlink2;

    Audio *audio = this->m_Audio;

    JS::RootedObject link1(cx);
    JS::RootedObject link2(cx);
    if (!JS_ConvertArguments(cx, args, "oo", link1.address(),
                             link2.address())) {
        return true;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class,
                                               &args);
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class,
                                               &args);

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return false;
    }

    if (nlink1->m_Type == AV::INPUT && nlink2->m_Type == AV::OUTPUT) {
        audio->disconnect(nlink2, nlink1);
    } else if (nlink1->m_Type == AV::OUTPUT && nlink2->m_Type == AV::INPUT) {
        audio->disconnect(nlink1, nlink2);
    } else {
        JS_ReportError(cx, "disconnect() take one input and one output\n");
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

    if (JS_GetObjectAsFloat64Array(x, &dlenx, &dx) == NULL) {
        JS_ReportError(cx, "Can't convert typed array (expected Float64Array)");
        return false;
    }
    if (JS_GetObjectAsFloat64Array(y, &dleny, &dy) == NULL) {
        JS_ReportError(cx, "Can't convert typed array (expected Float64Array)");
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

bool JSAudioContext::JSGetter_buffersize(JSContext *cx, JS::MutableHandleValue vp)
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();

    CHECK_INVALID_CTX(jaudio);
    AudioParameters *params = jaudio->m_Audio->m_OutputParameters;
    vp.setInt32(params->m_BufferSize / 8);

    return true;
}

bool JSAudioContext::JSGetter_channels(JSContext *cx, JS::MutableHandleValue vp)
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();

    CHECK_INVALID_CTX(jaudio);
    AudioParameters *params = jaudio->m_Audio->m_OutputParameters;
    vp.setInt32(params->m_Channels);

    return true;
}

bool JSAudioContext::JSGetter_sampleRate(JSContext *cx, JS::MutableHandleValue vp)
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();

    CHECK_INVALID_CTX(jaudio);
    AudioParameters *params = jaudio->m_Audio->m_OutputParameters;
    vp.setInt32(params->m_SampleRate);

    return true;
}

bool JSAudioContext::JSGetter_volume(JSContext *cx, JS::MutableHandleValue vp)
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();

    CHECK_INVALID_CTX(jaudio);
    AudioParameters *params = jaudio->m_Audio->m_OutputParameters;
    vp.setNumber(jaudio->m_Audio->getVolume());

    return true;
}

bool JSAudioContext::JSSetter_volume(JSContext *cx, JS::MutableHandleValue vp)
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();

    CHECK_INVALID_CTX(jaudio);

    if (vp.isNumber()) {
        jaudio->m_Audio->setVolume((float)vp.toNumber());
    }

    return true;
}

void JSAudioContext::initNode(JSAudioNode *node,
                       JS::HandleObject jnode,
                       JS::HandleString name)
{
    int in  = node->m_Node->m_InCount;
    int out = node->m_Node->m_OutCount;

    JS::RootedValue nameVal(m_Cx, STRING_TO_JSVAL(name));
    JS_DefineProperty(m_Cx, jnode, "type", nameVal,
                      JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
    JS::RootedObject arrayIn(m_Cx, JS_NewArrayObject(m_Cx, in));
    JS::RootedObject arrayOut(m_Cx, JS_NewArrayObject(m_Cx, out));
    JS::RootedValue inputLinks(m_Cx, OBJECT_TO_JSVAL(arrayIn));
    JS::RootedValue outputLinks(m_Cx, OBJECT_TO_JSVAL(arrayOut));

    for (int i = 0; i < in; i++) {
        JS::RootedObject link(m_Cx, JS_NewObject(m_Cx, &AudioNodeLink_class,
                                                 JS::NullPtr(), JS::NullPtr()));
        JS_SetPrivate(link, node->m_Node->m_Input[i]);
        JS_DefineElement(m_Cx, inputLinks.toObjectOrNull(), i,
                         OBJECT_TO_JSVAL(link), nullptr, nullptr, 0);
    }

    for (int i = 0; i < out; i++) {
        JS::RootedObject link(m_Cx, JS_NewObject(m_Cx, &AudioNodeLink_class,
                                                 JS::NullPtr(), JS::NullPtr()));
        JS_SetPrivate(link, node->m_Node->m_Output[i]);

        JS_DefineElement(m_Cx, outputLinks.toObjectOrNull(), i,
                         OBJECT_TO_JSVAL(link), nullptr, nullptr, 0);
    }

    if (in > 0) {
        JS_DefineProperty(m_Cx, jnode, "input", inputLinks,
                          JSPROP_ENUMERATE | JSPROP_READONLY
                              | JSPROP_PERMANENT);
    }

    if (out > 0) {
        JS_DefineProperty(m_Cx, jnode, "output", outputLinks,
                          JSPROP_ENUMERATE | JSPROP_READONLY
                              | JSPROP_PERMANENT);
    }

    node->m_nJs = NJS;

    node->setJSObject(jnode);
    node->setJSContext(m_Cx);

    NJS->rootObjectUntilShutdown(node->getJSObject());
    JS_SetPrivate(jnode, node);
}

bool JSAudioContext::createContext()
{
    if (m_JsRt != NULL) return false;

    if ((m_JsRt = JS_NewRuntime(128L * 1024L * 1024L, JS_USE_HELPER_THREADS))
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

    JS_SetStructuredCloneCallbacks(m_JsRt, NidiumJS::m_JsScc);

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

    js::SetDefaultObjectForContext(m_JsTcx, global);
    if (!JS_InitStandardClasses(m_JsTcx, global)) {
        NUI_LOG("Failed to init std class\n");
        return false;
    }
    JS_SetErrorReporter(m_JsTcx, reportError);
    JS_FireOnNewGlobalObject(m_JsTcx, global);
    JSConsole::RegisterObject(m_JsTcx);

    JS_SetRuntimePrivate(m_JsRt, NidiumJS::GetObject(m_Audio->getMainCtx()));

    // JS_SetContextPrivate(this->tcx, static_cast<void *>(this));

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

    JS::RootedFunction fun(
        m_JsTcx, JS_CompileFunction(m_JsTcx, globalObj, "Audio_run", 0, nullptr,
                                    str, strlen(str), options));
    if (!fun.get()) {
        JS_ReportError(m_JsTcx, "Failed to compile script on audio thread\n");
        return false;
    }

    JS::RootedValue rval(m_JsTcx);
    JS_CallFunction(m_JsTcx, globalObj, fun, JS::HandleValueArray::empty(),
                    &rval);

    return true;
}

void JSAudioContext::ShutdownCallback(void *custom)
{
    JSAudioContext *audio        = static_cast<JSAudioContext *>(custom);
    JSAudioContext::Nodes *nodes = audio->m_Nodes;

    // Let's shutdown all custom nodes
    while (nodes != NULL) {
        if (nodes->curr->m_NodeType == Audio::CUSTOM
            || nodes->curr->m_NodeType == Audio::CUSTOM_SOURCE) {
            nodes->curr->ShutdownCallback(nodes->curr->m_Node, nodes->curr);
        }

        nodes = nodes->next;
    }

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

    // Unroot all js audio nodes
    this->unroot();

    // Delete all nodes
    JSAudioContext::Nodes *nodes = m_Nodes;
    JSAudioContext::Nodes *next = NULL;
    while (nodes != NULL) {
        next = nodes->next;
        // Node destructor will remove the node
        // from the nodes linked list
        delete nodes->curr;
        nodes = next;
    }

    // Unroot custom nodes objects and clear threaded js context
    m_Audio->postMessage(JSAudioContext::ShutdownCallback, this, true);

    NIDIUM_PTHREAD_WAIT(&m_ShutdownWait)

    // Unlock the sources, so the decode thread can exit
    // when we call Audio::shutdown()
    m_Audio->unlockSources();

    // Shutdown the audio
    m_Audio->shutdown();

    m_Audio->unlockQueue();

    // And delete the audio
    delete m_Audio;

    JSAudioContext::m_Instance = NULL;
}

void JSAudioContext::CtxCallback(void *custom)
{
    JSAudioContext *audio = static_cast<JSAudioContext *>(custom);

    if (!audio->createContext()) {
        NUI_LOG("Failed to create audio thread context\n");
        // JS_ReportError(jsNode->audio->cx, "Failed to create audio thread
        // context\n");
        // XXX : Can't report error from another thread?
    }
}


JSFunctionSpec *JSAudioContext::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSAudioContext, run, 1),
        CLASSMAPPER_FN(JSAudioContext, load, 1),
        CLASSMAPPER_FN(JSAudioContext, createNode, 3),
        CLASSMAPPER_FN(JSAudioContext, connect, 2),
        CLASSMAPPER_FN(JSAudioContext, disconnect, 2),
        CLASSMAPPER_FN(JSAudioContext, pFFT, 2),

        JS_FS_END
    };

    return funcs;
}
JSPropertySpec *JSAudioContext::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_GS(JSAudioContext, volume),

        CLASSMAPPER_PROP_G(JSAudioContext, buffersize),
        CLASSMAPPER_PROP_G(JSAudioContext, channels),
        CLASSMAPPER_PROP_G(JSAudioContext, sampleRate),

        JS_PS_END
    };

    return props;
}

void JSAudioContext::RegisterObject(JSContext *cx)
{
    JSAudioContext::ExposeClass<0>(cx, "AudioContext");
}

} // namespace Binding
} // namespace Nidium

