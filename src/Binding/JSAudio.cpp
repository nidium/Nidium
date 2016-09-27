/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Frontend/Context.h"
#include "Binding/JSAudio.h"
#include "Binding/JSConsole.h"
#include "Macros.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {

extern void
reportError(JSContext *cx, const char *message, JSErrorReport *report);

// {{{ Preamble

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

// }}}

// {{{ Implementation
JSAudio *JSAudio::GetContext(JSContext *cx,
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

    return new JSAudio(audio);
}

void JSAudio::initNode(JSAudioNode *node,
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

JSAudio *JSAudio::GetContext()
{
    return JSAudio::m_Instance;
}

JSAudio::JSAudio(Audio *audio)
    : m_Audio(audio), m_Nodes(NULL),
      m_JsGlobalObj(NULL), m_JsRt(NULL), m_JsTcx(NULL), m_Target(NULL)
{
    JSAudio::m_Instance = this;

    this->root();

    NIDIUM_PTHREAD_VAR_INIT(&m_ShutdownWait)

    m_Audio->postMessage(JSAudio::CtxCallback, static_cast<void *>(this), true);
}

bool JSAudio::createContext()
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

bool JSAudio::run(char *str)
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

void JSAudio::ShutdownCallback(void *custom)
{
    JSAudio *audio        = static_cast<JSAudio *>(custom);
    JSAudio::Nodes *nodes = audio->m_Nodes;

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

JSAudio::~JSAudio()
{
    m_Audio->lockSources();
    m_Audio->lockQueue();

    // Unroot all js audio nodes
    this->unroot();

    // Delete all nodes
    JSAudio::Nodes *nodes = m_Nodes;
    JSAudio::Nodes *next = NULL;
    while (nodes != NULL) {
        next = nodes->next;
        // Node destructor will remove the node
        // from the nodes linked list
        delete nodes->curr;
        nodes = next;
    }

    // Unroot custom nodes objects and clear threaded js context
    m_Audio->postMessage(JSAudio::ShutdownCallback, this, true);

    NIDIUM_PTHREAD_WAIT(&m_ShutdownWait)

    // Unlock the sources, so the decode thread can exit
    // when we call Audio::shutdown()
    m_Audio->unlockSources();

    // Shutdown the audio
    m_Audio->shutdown();

    m_Audio->unlockQueue();

    // And delete the audio
    delete m_Audio;

    JSAudio::m_Instance = NULL;
}


void JSAudio::CtxCallback(void *custom)
{
    JSAudio *audio = static_cast<JSAudio *>(custom);

    if (!audio->createContext()) {
        NUI_LOG("Failed to create audio thread context\n");
        // JS_ReportError(jsNode->audio->cx, "Failed to create audio thread
        // context\n");
        // XXX : Can't report error from another thread?
    }
}


bool JSAudio::JS_getContext(JSContext *cx, JS::CallArgs &args)
{
    unsigned int bufferSize, channels, sampleRate;
    unsigned int argc = args.length();

    bufferSize = 0;
    channels = 2;
    sampleRate = 44100;
    switch (argc) {
        case 3:
            JS::ToUint32(cx, args[2], &sampleRate);
            //ft
        case 2:
            JS::ToUint32(cx, args[1], &channels);
            //ft
        case 1:
            JS::ToUint32(cx, args[0], &bufferSize);
            break;
        default:
            break;
    }
    switch (bufferSize) {
        case 0:
        case 128:
        case 256:
        case 512:
        case 1024:
        case 2048:
        case 4096:
        case 8192:
        case 16384:
            // Supported buffer size
            // Multiply by 8 to get the bufferSize in bytes
            // rather than in samples per buffer
            bufferSize *= 8;
            break;
        default:
            JS_ReportError(cx,
                           "Unsuported buffer size %d. "
                           "Supported values are : 0, 128, 256, 512, 1024, "
                           "2048, 4096, 8192, 16384\n",
                           bufferSize);
            return false;
            break;
    }

    if (channels < 1 || channels > 32) {
        JS_ReportError(cx,
                       "Unsuported channels number %d. Channels must be "
                       "between 1 and 32\n",
                       channels);
        return false;
    }

    if (sampleRate < 22050 || sampleRate > 96000) {
        JS_ReportError(cx,
                       "Unsuported sample rate %dKHz. Sample rate must be "
                       "between 22050 and 96000\n",
                       sampleRate);
        return false;
    }

    bool paramsChanged = false;
    JSAudio *jaudio    = JSAudio::GetContext(cx);

    if (jaudio) {
        AudioParameters *params = jaudio->m_Audio->m_OutputParameters;
        if (params->m_AskedBufferSize != bufferSize
            || params->m_Channels != channels
            || params->m_SampleRate != sampleRate) {
            paramsChanged = true;
        }
    }

    if (!paramsChanged && jaudio) {
        JS::RootedObject retObj(cx, jaudio->getJSObject());
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    if (paramsChanged) {
        JSContext *m_Cx = cx;
        JS_SetPrivate(jaudio->getJSObject(), NULL);
        NJS->unrootObject(jaudio->getJSObject());
        delete jaudio;
    }

    JS::RootedObject ret(
        cx, JS_NewObjectForConstructor(cx, &AudioContext_class, args));

    JSAudio *naudio
        = JSAudio::GetContext(cx, bufferSize, channels, sampleRate);

    if (naudio == NULL) {
        JS_ReportError(cx, "Failed to initialize audio context\n");
        return false;
    }

    args.rval().setObjectOrNull(ret);

    return true;
}


JSFunctionSpec *JSAudio::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSAudio, getContext, 3),
        JS_FS_END
    };

    return funcs;
}

// }}}

// {{{ Registration
void JSAudio::RegisterObject(JSContext *cx)
{
    JSAudio::ExposeClass<0>(cx, "Audio");
}
// }}}

} // namespace Binding
} // namespace Nidium


