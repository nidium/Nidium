/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Frontend/Context.h"
#include "Binding/JSAudio.h"
#include "Macros.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {


// {{{JSAudio
static bool
nidium_Audio_constructor(JSContext *cx, unsigned argc, JS::Value *vp);
static bool
nidium_audio_getcontext(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Audio_class = { "Audio",
                               JSCLASS_HAS_PRIVATE,
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
                               nullptr,
                               JSCLASS_NO_INTERNAL_MEMBERS };

static JSFunctionSpec Audio_static_funcs[]
    = { JS_FN("getContext", nidium_audio_getcontext, 3, NIDIUM_JS_FNPROPS),
        JS_FS_END };

static bool
nidium_Audio_constructor(JSContext *cx, unsigned argc, JS::Value *vp)

{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}
// }}}


bool nidium_audio_getcontext(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    unsigned int bufferSize, channels, sampleRate;

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
    JSAudioContext *jaudio    = JSAudioContext::GetContext();

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

    JSClass * audioContextClass = JSAudioContext::GetJSClass();
    JS::RootedObject ret( cx, JS_NewObjectForConstructor(cx, audioContextClass));

    JSAudioContext *naudio
        = JSAudioContext::GetContext(cx, bufferSize, channels, sampleRate);

    if (naudio == NULL) {
        JS_ReportError(cx, "Failed to initialize audio context\n");
        return false;
    }

    args.rval().setObjectOrNull(ret);

    return true;
}

// {{{ Registration

void audio_RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Audio_class,
                 nidium_Audio_constructor, 0, nullptr, nullptr, nullptr,
                 Audio_static_funcs);
}
// }}}

} // namespace Binding
} // namespace Nidium


