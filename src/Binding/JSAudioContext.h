/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsaudiocontext_h__
#define binding_jsaudiocontext_h__


#include "Frontend/Context.h"
#include "AV/Audio.h"
#include "Binding/ClassMapper.h"
#include "Binding/JSAV.h"
#include "Macros.h"

namespace Nidium {
namespace Binding {

class JSAudioNode;

class JSAudioContext : public ClassMapper<JSAudioContext>
{
public:
    static JSAudioContext *GetContext();
    static JSAudioContext *GetContext(JSContext * cx, unsigned int bufferSize,
                               unsigned int channels, unsigned int sampleRate);

    struct Nodes
    {
        JSAudioNode *curr;

        Nodes *prev;
        Nodes *next;

        Nodes(JSAudioNode *curr, Nodes *prev, Nodes *next)
            : curr(curr), prev(prev), next(next) { }
        Nodes() : curr(NULL), prev(NULL), next(NULL) { }
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
    static void RunCallback(void *custom);
    static void CtxCallback(void *custom);
    static void ShutdownCallback(void *custom);
    void unroot();

    static void RegisterObject(JSContext *cx);
    static JSPropertySpec *ListProperties();
    static JSFunctionSpec *ListMethods();

    ~JSAudioContext();
protected:
     NIDIUM_DECL_JSCALL(run);
     NIDIUM_DECL_JSCALL(load);
     NIDIUM_DECL_JSCALL(createNode);
     NIDIUM_DECL_JSCALL(connect);
     NIDIUM_DECL_JSCALL(disconnect);
     NIDIUM_DECL_JSCALL(pFFT);

    NIDIUM_DECL_JSGETTERSETTER(volume);
    NIDIUM_DECL_JSGETTER(buffersize);
    NIDIUM_DECL_JSGETTER(channels);
    NIDIUM_DECL_JSGETTER(sampleRate);
private:
    JSAudioContext(AV::Audio *audio);
    static JSAudioContext *m_Instance;
};



} // namespace Binding
} // namespace Nidium

#endif

