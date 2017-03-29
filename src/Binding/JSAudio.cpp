/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Frontend/Context.h"
#include "Binding/JSAudio.h"
#include "Binding/JSAudioNode.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {

JSFunctionSpec *JSAudio::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSAudio, getContext, 0),
        JS_FS_END
    };

    return funcs;
}

void JSAudio::RegisterObject(JSContext *cx)
{
    JSAudio::ExposeClass(cx, "NidiumAudio", JSCLASS_HAS_RESERVED_SLOTS(1));
    JSAudio::CreateUniqueInstance(cx, new JSAudio(), "Audio");
}

void JSAudio::RegisterAllObjects(JSContext *cx)
{
    JSAudio::RegisterObject(cx);
    JSAudioContext::RegisterObject(cx);
    JSAudioNodeGain::RegisterObject(cx);
    JSAudioNodeBuffers::RegisterObject(cx);
    JSAudioNodeStereoEnhancer::RegisterObject(cx);
    JSAudioNodeLink::RegisterObject(cx);
    JSAudioNodeDelay::RegisterObject(cx);
    JSAudioNodeReverb::RegisterObject(cx);
    JSAudioNodeCustom::RegisterObject(cx);
    JSAudioNodeCustomSource::RegisterObject(cx);
    JSAudioNodeSource::RegisterObject(cx);
    JSAudioNodeTarget::RegisterObject(cx);
}

bool JSAudio::JS_getContext(JSContext *cx, JS::CallArgs &args)
{
    unsigned int bufferSize, channels, sampleRate;

    bufferSize = 0;
    channels   = 0;
    sampleRate = 0;

    switch (args.length()) {
        case 3:
            JS::ToUint32(cx, args[2], &sampleRate);
        case 2:
            JS::ToUint32(cx, args[1], &channels);
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
            bufferSize *= AV::Audio::FLOAT32;
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

    if (channels != 0 && (channels < 1 || channels > 32)) {
        JS_ReportError(cx,
                       "Unsuported channels number %d. Channels must be "
                       "between 1 and 32\n",
                       channels);
        return false;
    }

    if (sampleRate != 0 && (sampleRate < 22050 || sampleRate > 96000)) {
        JS_ReportError(cx,
                       "Unsuported sample rate %dKHz. Sample rate must be "
                       "between 22050 and 96000\n",
                       sampleRate);
        return false;
    }

    bool paramsChanged     = false;
    JSAudioContext *jaudio = JSAudioContext::GetContext();

    if (jaudio) {
        AudioParameters *params = jaudio->m_Audio->m_OutputParameters;
        if (params->m_AskedBufferSize != bufferSize
            || (channels != 0 && params->m_Channels != channels)
            || (sampleRate != 0 && params->m_SampleRate != sampleRate)) {
            paramsChanged = true;
        }
    }

    if (!paramsChanged && jaudio) {
        JS::RootedObject retObj(cx, jaudio->getJSObject());
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    if (paramsChanged) {
        delete jaudio;
    }

    JSAudioContext *audioCtx
        = JSAudioContext::GetContext(cx, bufferSize, channels, sampleRate);

    if (audioCtx == NULL) {
        JS_ReportError(cx, "Failed to initialize audio context\n");
        return false;
    }

    JS::RootedObject jsAudioContext(cx, audioCtx->getJSObject());

    args.rval().setObjectOrNull(jsAudioContext);

    return true;
}

} // namespace Binding
} // namespace Nidium
