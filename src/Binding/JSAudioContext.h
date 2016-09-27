/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsaudiocontext_h__
#define binding_jsaudiocontext_h__


#include "AV/Audio.h"
#include "Binding/ClassMapper.h"
#include "Binding/JSAV.h"

namespace Nidium {
namespace Binding {

static void AudioContext_Finalize(JSFreeOp *fop, JSObject *obj);
extern bool nidium_audio_prop_setter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     bool strict,
                                     JS::MutableHandleValue vp);
extern bool nidium_audio_prop_getter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     JS::MutableHandleValue vp);

static bool nidium_audio_run(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_audio_load(JSContext *cx, unsigned argc, JS::Value *vp);
static bool
nidium_audio_createnode(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_audio_connect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool
nidium_audio_disconnect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_audio_pFFT(JSContext *cx, unsigned argc, JS::Value *vp);



static JSPropertySpec AudioContext_props[] = {
    NIDIUM_JS_PSGS("volume",
                   AUDIO_PROP_VOLUME,
                   nidium_audio_prop_getter,
                   nidium_audio_prop_setter),
    NIDIUM_JS_PSG(
        "bufferSize", AUDIO_PROP_BUFFERSIZE, nidium_audio_prop_getter),
    NIDIUM_JS_PSG("channels", AUDIO_PROP_CHANNELS, nidium_audio_prop_getter),
    NIDIUM_JS_PSG(
        "sampleRate", AUDIO_PROP_SAMPLERATE, nidium_audio_prop_getter),
    JS_PS_END
};

static JSFunctionSpec AudioContext_funcs[]
    = { JS_FN("run", nidium_audio_run, 1, NIDIUM_JS_FNPROPS),
        JS_FN("load", nidium_audio_load, 1, NIDIUM_JS_FNPROPS),
        JS_FN("createNode", nidium_audio_createnode, 3, NIDIUM_JS_FNPROPS),
        JS_FN("connect", nidium_audio_connect, 2, NIDIUM_JS_FNPROPS),
        JS_FN("disconnect", nidium_audio_disconnect, 2, NIDIUM_JS_FNPROPS),
        JS_FN("pFFT", nidium_audio_pFFT, 2, NIDIUM_JS_FNPROPS),
        JS_FS_END };

static JSClass AudioContext_class = { "AudioContext",
                                      JSCLASS_HAS_PRIVATE,
                                      JS_PropertyStub,
                                      JS_DeletePropertyStub,
                                      JS_PropertyStub,
                                      JS_StrictPropertyStub,
                                      JS_EnumerateStub,
                                      JS_ResolveStub,
                                      JS_ConvertStub,
                                      AudioContext_Finalize,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      JSCLASS_NO_INTERNAL_MEMBERS };



} // namespace Binding
} // namespace Nidium

#endif

