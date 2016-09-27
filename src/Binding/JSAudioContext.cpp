/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSAudioContext.h"
#include "Binding/JSAudioNode.h"

#include "Core/Utils.h"
#include "AV/Audio.h"
#include "Frontend/Context.h"

using namespace Nidium::AV;
using namespace Nidium::Core;

namespace Nidium {
namespace Binding {

template <>
JSClass *JSExposer<JSAudio>::jsclass = &AudioContext_class;

bool nidium_audio_prop_setter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     bool strict,
                                     JS::MutableHandleValue vp);
bool nidium_audio_prop_getter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     JS::MutableHandleValue vp);


static bool nidium_audio_run(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSAudio *audio    = JSAudio::GetContext();

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

    audio->m_Audio->postMessage(JSAudio::RunCallback,
                                static_cast<void *>(funStr));

    return true;
}

static bool nidium_audio_load(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Not implemented");
    return false;
}

static bool nidium_audio_createnode(JSContext *cx, unsigned argc, JS::Value *vp)
{
    int in, out;
    JSAudio *audio;
    JSAudioNode *node;

    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSAudio, &AudioContext_class);
    audio = CppObj;
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

    JS::RootedObject ret(
        cx, JS_NewObjectForConstructor(cx, &AudioNode_class, args));
    if (!ret) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    JS_SetReservedSlot(ret, 0, JSVAL_NULL);

    JSAutoByteString cname(cx, name);
    try {
        if (strcmp("source", cname.ptr()) == 0) {
            node = new JSAudioNode(ret, cx, Audio::SOURCE, in, out, audio);

            AudioSource *source = static_cast<AudioSource *>(node->m_Node);
            source->eventCallback(JSAudioNode::onEvent, node);

            JS_DefineFunctions(cx, ret, AudioNodeSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeSource_props);
        } else if (strcmp("custom-source", cname.ptr()) == 0) {
            node = new JSAudioNode(ret, cx, Audio::CUSTOM_SOURCE, in, out,
                                   audio);

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
            node = new JSAudioNode(ret, cx, Audio::CUSTOM, in, out, audio);
            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
        } else if (strcmp("reverb", cname.ptr()) == 0) {
            node = new JSAudioNode(ret, cx, Audio::REVERB, in, out, audio);
        } else if (strcmp("delay", cname.ptr()) == 0) {
            node = new JSAudioNode(ret, cx, Audio::DELAY, in, out, audio);
        } else if (strcmp("gain", cname.ptr()) == 0) {
            node = new JSAudioNode(ret, cx, Audio::GAIN, in, out, audio);
        } else if (strcmp("target", cname.ptr()) == 0) {
            if (audio->m_Target != NULL) {
                JS::RootedObject retObj(cx, audio->m_Target->getJSObject());
                args.rval().setObjectOrNull(retObj);
                return true;
            } else {
                node            = new JSAudioNode(ret, cx, Audio::TARGET, in, out, audio);
                audio->m_Target = node;
            }
        } else if (strcmp("stereo-enhancer", cname.ptr()) == 0) {
            node = new JSAudioNode(ret, cx, Audio::STEREO_ENHANCER, in, out,
                                   audio);
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

    audio->initNode(node, ret, name);

    args.rval().setObjectOrNull(ret);

    return true;
}

static bool nidium_audio_connect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NodeLink *nlink1;
    NodeLink *nlink2;
    JSAudio *jaudio;

    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSAudio, &AudioContext_class);
    jaudio       = CppObj;
    Audio *audio = jaudio->m_Audio;

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

static bool nidium_audio_disconnect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NodeLink *nlink1;
    NodeLink *nlink2;
    JSAudio *jaudio;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudio, &AudioContext_class);
    jaudio       = CppObj;
    Audio *audio = jaudio->m_Audio;

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

static bool nidium_audio_pFFT(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
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

bool nidium_audio_prop_setter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     bool strict,
                                     JS::MutableHandleValue vp)
{
    JSAudio *jaudio = JSAudio::GetContext();

    CHECK_INVALID_CTX(jaudio);

    if (vp.isNumber()) {
        jaudio->m_Audio->setVolume((float)vp.toNumber());
    }

    return true;
}

bool nidium_audio_prop_getter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     JS::MutableHandleValue vp)
{
    JSAudio *jaudio = JSAudio::GetContext();

    CHECK_INVALID_CTX(jaudio);

    AudioParameters *params = jaudio->m_Audio->m_OutputParameters;

    switch (id) {
        case AUDIO_PROP_BUFFERSIZE:
            vp.setInt32(params->m_BufferSize / 8);
            break;
        case AUDIO_PROP_CHANNELS:
            vp.setInt32(params->m_Channels);
            break;
        case AUDIO_PROP_SAMPLERATE:
            vp.setInt32(params->m_SampleRate);
            break;
        case AUDIO_PROP_VOLUME:
            vp.setNumber(jaudio->m_Audio->getVolume());
            break;
        default:
            return false;
            break;
    }

    return true;
}

void AudioContext_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSAudio *audio = (JSAudio *)JS_GetPrivate(obj);
    if (audio != NULL) {
        JS_SetPrivate(obj, nullptr);
        delete audio;
    }
}

void JSAudio::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &AudioContext_class,
                 nidium_Audio_constructor, 0, AudioContext_props,
                 AudioContext_funcs, nullptr, nullptr);
}


} // namespace Binding
} // namespace Nidium


