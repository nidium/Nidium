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

namespace Nidium {
namespace Binding {

class JSAudioNode;
class JSAudioNodeTarget;

class JSAudioNodeLink : public ClassMapper<JSAudioNodeLink>
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeLink::ExposeClass<0>(cx, "AudioNodeLink");
    }

    JSAudioNodeLink(AV::NodeLink *link) : m_Link(link){};
    virtual ~JSAudioNodeLink(){};

    AV::NodeLink *get()
    {
        return m_Link;
    }

private:
    AV::NodeLink *m_Link;
};

class JSAudioContext : public ClassMapper<JSAudioContext>
{
public:
    static JSAudioContext *GetContext();
    static JSAudioContext *GetContext(JSContext *cx,
                                      unsigned int bufferSize,
                                      unsigned int channels,
                                      unsigned int sampleRate);

    struct NodeListItem
    {
        JSAudioNode *curr  = nullptr;
        NodeListItem *prev = nullptr;
        NodeListItem *next = nullptr;

        NodeListItem(JSAudioNode *curr, NodeListItem *prev, NodeListItem *next)
            : curr(curr), prev(prev), next(next){}
        NodeListItem(){}
    };

    AV::Audio *m_Audio;
    pthread_t m_ThreadIO;

    NIDIUM_PTHREAD_VAR_DECL(m_ShutdownWait)

    JS::Heap<JSObject *> m_JsGlobalObj;

    JSRuntime *m_JsRt  = nullptr;
    JSContext *m_JsTcx = nullptr;

    bool createContext();

    bool run(char *str);
    void addNode(JSAudioNode *node);
    void removeNode(JSAudioNode *node);
    static void RunCallback(void *custom);
    static void CtxCallback(void *custom);
    static void ShutdownCallback(void *custom);

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
    NIDIUM_DECL_JSGETTER(bufferSize);
    NIDIUM_DECL_JSGETTER(channels);
    NIDIUM_DECL_JSGETTER(sampleRate);

private:
    static JSAudioContext *m_Ctx;

    JSAudioNodeTarget *m_Target = nullptr;
    NodeListItem *m_Nodes       = nullptr;

    JSAudioContext(JSContext *cx, AV::Audio *audio);
};


} // namespace Binding
} // namespace Nidium

#endif
