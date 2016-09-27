/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef binding_jsaudio_h__
#define binding_jsaudio_h__

#include "Binding/JSExposer.h"
#include "AV/Audio.h"
#include "Binding/JSAudioNode.h"
#include "Binding/JSAudioContext.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {

class JSAudioNode;

bool nidium_audio_prop_setter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     bool strict,
                                     JS::MutableHandleValue vp);
bool nidium_audio_prop_getter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     JS::MutableHandleValue vp);

class JSAudio : public JSExposer<JSAudio>
{
public:
    static JSAudio *GetContext(JSContext *cx,
                               JS::HandleObject obj,
                               unsigned int bufferSize,
                               unsigned int channels,
                               unsigned int sampleRate);
    static JSAudio *GetContext();

    struct Nodes
    {
        JSAudioNode *curr;

        Nodes *prev;
        Nodes *next;

        Nodes(JSAudioNode *curr, Nodes *prev, Nodes *next)
            : curr(curr), prev(prev), next(next)
        {
        }
        Nodes() : curr(NULL), prev(NULL), next(NULL)
        {
        }
    };

    AV::Audio *m_Audio;
    Nodes *m_Nodes;
    pthread_t m_ThreadIO;

    NIDIUM_PTHREAD_VAR_DECL(m_ShutdownWait)

    JS::Heap<JSObject *> m_JsGlobalObj;

    JSRuntime *m_JsRt;
    JSContext *m_JsTcx;
    JSAudioNode *m_Target;

    bool createContext();
    void
    initNode(JSAudioNode *node, JS::HandleObject jnode, JS::HandleString name);
    bool run(char *str);
    static void CtxCallback(void *custom);
    static void RunCallback(void *custom);
    static void ShutdownCallback(void *custom);
    void unroot();

    static void RegisterObject(JSContext *cx);

    ~JSAudio();

private:
    JSAudio(AV::Audio *audio, JSContext *cx, JS::HandleObject obj);
    static JSAudio *m_Instance;
};


} // namespace Binding
} // namespace Nidium



#endif

