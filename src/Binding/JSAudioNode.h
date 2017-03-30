/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsaudionode_h__
#define binding_jsaudionode_h__

#include "AV/Audio.h"
#include "AV/AudioNode.h"
#include "AV/Video.h"
#include "Binding/JSAV.h"
#include "Binding/JSAudioContext.h"

#include <vector>

using Nidium::AV::Audio;
using Nidium::AV::AudioNode;
using Nidium::AV::NodeEvent;
using Nidium::AV::AudioNodeException;

namespace Nidium {
namespace Binding {

class JSAudio;
class JSVideo;
class JSTransferable;
class JSTransferableFunction;

// {{{ JSAudioNode
class JSAudioNode
{
public:
    virtual ~JSAudioNode()
    {
        if (!this->isReleased()) {
            this->releaseNode();
        }
    };

    /*
            Returns true when the releaseNode
    */
    bool isReleased()
    {
        return m_IsReleased;
    }

    JSAudioContext *getAudioContext()
    {
        return JSAudioContext::GetContext();
    }

    AudioNode *getNode()
    {
        return m_Node;
    }

    template <typename T>
    T *getNode()
    {
        return static_cast<T *>(m_Node);
    }

    NIDIUM_DECL_JSCALL(_set);

protected:
    AudioNode *m_Node = nullptr;
    JSContext *m_JSCx;

    /*
        Safely delete the underlying audio node. This method MUST be
        called before freeing any resource associated with the node.
    */
    void releaseNode();

    /*
        Create an audio node of the given |type|
     */
    bool createNode(Audio::Node type, unsigned int in, unsigned int out)
    {
        JSAudioContext *jaudio = JSAudioContext::GetContext();
        Audio *audio           = jaudio->m_Audio;

        try {
            m_Node = audio->createNode(type, in, out);
        } catch (AudioNodeException *e) {
            ndm_logf(NDM_LOG_ERROR, "JSAudioNode", "Audio node creation failed : %s", e->what());
            return false;
        }

        return true;
    }

    /*
            Create the JSObject for the node, root it, and assign all input &
       ouput.
    */
    template <typename T>
    JSAudioNode *createObject(JSContext *cx)
    {
        if (!m_Node) return nullptr;

        JSAudioContext *jaudio = JSAudioContext::GetContext();
        unsigned int in        = m_Node->m_InCount;
        unsigned int out       = m_Node->m_OutCount;

        m_JSCx = cx;

        JS::RootedObject jnode(cx, T::CreateObject(cx, static_cast<T *>(this)));
        T *node = static_cast<T *>(T::GetInstance(jnode, cx));
        node->root();

        jaudio->addNode(node);

        JS::RootedObject inputLinks(cx, JS_NewArrayObject(cx, in));
        JS::RootedObject outputLinks(cx, JS_NewArrayObject(cx, out));

        for (int i = 0; i < in; i++) {
            JSAudioNodeLink *link = new JSAudioNodeLink(m_Node->m_Input[i]);
            JS::RootedObject jlink(cx, JSAudioNodeLink::CreateObject(cx, link));

            JS_DefineElement(cx, inputLinks, i, jlink, 0);

            m_Links.push_back(link);
        }

        for (int i = 0; i < out; i++) {
            JSAudioNodeLink *link = new JSAudioNodeLink(m_Node->m_Output[i]);
            JS::RootedObject jlink(cx, JSAudioNodeLink::CreateObject(cx, link));

            JS_DefineElement(cx, outputLinks, i, jlink, 0);

            m_Links.push_back(link);
        }

        if (in > 0) {
            JS_DefineProperty(cx, jnode, "input", inputLinks,
                              JSPROP_ENUMERATE | JSPROP_READONLY
                                  | JSPROP_PERMANENT);
        }

        if (out > 0) {
            JS_DefineProperty(cx, jnode, "output", outputLinks,
                              JSPROP_ENUMERATE | JSPROP_READONLY
                                  | JSPROP_PERMANENT);
        }

        return node;
    }

private:
    bool m_IsReleased = false;
    std::vector<JSAudioNodeLink *> m_Links;
};
// }}}

// {{{ JSAudioNodeSource
class JSAudioNodeSource : public ClassMapperWithEvents<JSAudioNodeSource>,
                          public JSAudioNode,
                          public JSAVSourceBase,
                          public JSAVSourceEventInterface
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeSource::ExposeClass<0>(cx, "AudioNodeSource");
    }

    static JSPropertySpec *ListProperties();
    static JSFunctionSpec *ListMethods();

    JSAudioNodeSource(JSContext *cx, unsigned int out)
    {
        this->createNode(Audio::SOURCE, 0, out);
        this->createObject<JSAudioNodeSource>(cx);
        this->listenSourceEvents(reinterpret_cast<AV::AudioSource *>(m_Node));
    }

    virtual ~JSAudioNodeSource(){}

protected:
    JSAV_PASSTHROUGH_G(JSAVSourceBase, position)
    JSAV_PASSTHROUGH_S(JSAVSourceBase, position)
    JSAV_PASSTHROUGH_G(JSAVSourceBase, duration)
    JSAV_PASSTHROUGH_G(JSAVSourceBase, metadata)
    JSAV_PASSTHROUGH_G(JSAVSourceBase, bitrate)

    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, open)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, play)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, pause)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, stop)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, close)

    // JSAVSourceBase virtual functions implementation
    AV::AVSource *getSource() override
    {
        return static_cast<AV::AudioSource *>(m_Node);
    }

    // JSAVSourceEventInterface virtual functions implementation
    bool isReleased() override
    {
        return JSAudioNode::isReleased();
    }

    JSContext *getJSContext() override
    {
        return ClassMapperWithEvents<JSAudioNodeSource>::getJSContext();
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue val) override
    {
        return ClassMapperWithEvents<JSAudioNodeSource>::fireJSEvent(name, val);
    }
};
// }}}

// {{{ JSAudioNodeCustomBase
class JSAudioNodeThreaded;
class JSAudioNodeCustomBase : public JSAudioNode, virtual public Core::Messages
{
public:
    enum TransferableFunction
    {
        PROCESS_FN,
        SETTER_FN,
        INIT_FN,
        SEEK_FN,
        END_FN
    };

    JSAudioNodeCustomBase();
    virtual ~JSAudioNodeCustomBase()
    {
        m_Node->callback(JSAudioNodeCustomBase::ShutdownCallback, this, true);

        NIDIUM_PTHREAD_WAIT(&m_ShutdownWait);
    }

    virtual void onMessage(const Core::SharedMessages::Message &msg) override;

    static void OnAssignInit(AudioNode *n, void *custom);
    static void OnSet(AudioNode *n, void *custom);
    static void ShutdownCallback(AudioNode *n, void *custom);
    static void Processor(const struct NodeEvent *ev);

    JSTransferableFunction *getFunction(TransferableFunction funID)
    {
        return m_TransferableFuncs[funID];
    }

    void releaseFunction(TransferableFunction funID);

    virtual JSObject *getJSObject() = 0;
    virtual bool fireJSEvent(const char *name, JS::MutableHandleValue val) = 0;

protected:
    NIDIUM_PTHREAD_VAR_DECL(m_ShutdownWait)

    void initThreadedNode();
    JSObject *getThreadedObject();

    NIDIUM_DECL_JSCALL(_set);
    NIDIUM_DECL_JSCALL(_assignProcessor);
    NIDIUM_DECL_JSCALL(_assignInit);
    NIDIUM_DECL_JSCALL(_assignSetter);

private:
    JSTransferableFunction *m_TransferableFuncs[END_FN];
    JSAudioNodeThreaded *m_ThreadedNode = nullptr;
};
// }}}

// {{{ JSAudioNodeCustom
class JSAudioNodeCustom : public ClassMapperWithEvents<JSAudioNodeCustom>,
                          public JSAudioNodeCustomBase
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeCustom::ExposeClass<0>(cx, "AudioNodeCustom");
    }

    static JSFunctionSpec *ListMethods();

    JSAudioNodeCustom(JSContext *cx, unsigned int in, unsigned int out)
    {
        this->createNode(Audio::CUSTOM, in, out);
        this->createObject<JSAudioNodeCustom>(cx);
    }

    virtual ~JSAudioNodeCustom(){}

    // JSAudioNodeCustomBase virtual functions implementation
    JSObject *getJSObject() override
    {
        return ClassMapperWithEvents<JSAudioNodeCustom>::getJSObject();
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue val) override
    {
        return ClassMapperWithEvents<JSAudioNodeCustom>::fireJSEvent(name, val);
    }

protected:
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, set);
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, assignProcessor);
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, assignInit);
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, assignSetter);
};
// }}}

// {{{ JSAudioNodeCustomSource
class JSAudioNodeCustomSource
    : public ClassMapperWithEvents<JSAudioNodeCustomSource>,
      public JSAVSourceEventInterface,
      public JSAudioNodeCustomBase
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeCustomSource::ExposeClass<0>(cx, "AudioNodeCustomSource");
    }

    static JSPropertySpec *ListProperties();
    static JSFunctionSpec *ListMethods();
    static void
    OnSeek(AV::AudioSourceCustom *node, double seekTime, void *custom);

    JSAudioNodeCustomSource(JSContext *cx, unsigned int out)
    {
        this->createNode(Audio::CUSTOM_SOURCE, 0, out);
        this->createObject<JSAudioNodeCustomSource>(cx);
        this->listenSourceEvents(
            reinterpret_cast<AV::AudioSourceCustom *>(m_Node));
    }

    virtual ~JSAudioNodeCustomSource(){}

    void onMessage(const Core::SharedMessages::Message &msg) override;

    // JSAudioNodeCustomBase virtual functions implementation
    JSObject *getJSObject() override
    {
        return ClassMapperWithEvents<JSAudioNodeCustomSource>::getJSObject();
    }

protected:
    NIDIUM_DECL_JSGETTER(position);
    NIDIUM_DECL_JSSETTER(position);

    NIDIUM_DECL_JSCALL(play);
    NIDIUM_DECL_JSCALL(pause);
    NIDIUM_DECL_JSCALL(stop);

    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, set);
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, assignProcessor);
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, assignInit);
    JSAV_PASSTHROUGH_CALL(JSAudioNodeCustomBase, assignSetter);
    NIDIUM_DECL_JSCALL(assignSeek);

    // JSAVSourceEventInterface virtual functions implementation
    bool isReleased() override
    {
        return JSAudioNode::isReleased();
    }

    JSContext *getJSContext() override
    {
        return ClassMapperWithEvents<JSAudioNodeCustomSource>::getJSContext();
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue val) override
    {
        return ClassMapperWithEvents<JSAudioNodeCustomSource>::fireJSEvent(name,
                                                                           val);
    }
};
// }}}

// {{{ JSAudioNodeTarget
class JSAudioNodeTarget : public ClassMapper<JSAudioNodeTarget>,
                          public JSAudioNode
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeTarget::ExposeClass<0>(cx, "AudioNodeTarget");
    }

    JSAudioNodeTarget(JSContext *cx, unsigned int in)
    {
        this->createNode(Audio::TARGET, in, 0);
        this->createObject<JSAudioNodeTarget>(cx);
    }

    virtual ~JSAudioNodeTarget()
    {
    }
};
// }}}

// {{{ Standard nodes (Delay, Reverb, Gain, StereoEnhancer)
#define DEFINE_STD_JS_AUDIO_NODE(NAME, ID)                                  \
    class JSAudioNode##NAME : public ClassMapper<JSAudioNode##NAME>,        \
                              public JSAudioNode                            \
    {                                                                       \
    public:                                                                 \
        static void RegisterObject(JSContext *cx)                           \
        {                                                                   \
            JSAudioNode##NAME::ExposeClass<0>(cx, "AudioNode" #NAME);       \
        }                                                                   \
        static JSFunctionSpec *ListMethods()                                \
        {                                                                   \
            static JSFunctionSpec funcs[]                                   \
                = { CLASSMAPPER_FN(JSAudioNode##NAME, set, 1), JS_FS_END }; \
            return funcs;                                                   \
        }                                                                   \
        JSAudioNode##NAME(JSContext *cx, unsigned int in, unsigned int out) \
        {                                                                   \
            this->createNode(ID, in, out);                                  \
            this->createObject<JSAudioNode##NAME>(cx);                      \
        }                                                                   \
        virtual ~JSAudioNode##NAME()                                        \
        {                                                                   \
        }                                                                   \
                                                                            \
    protected:                                                              \
        JSAV_PASSTHROUGH_CALL(JSAudioNode, set)                             \
    };

DEFINE_STD_JS_AUDIO_NODE(Reverb, Audio::REVERB)
DEFINE_STD_JS_AUDIO_NODE(Delay, Audio::DELAY)
DEFINE_STD_JS_AUDIO_NODE(Gain, Audio::GAIN)
DEFINE_STD_JS_AUDIO_NODE(StereoEnhancer, Audio::STEREO_ENHANCER)
#undef DEFINE_STD_JS_AUDIO_NODE
// }}}

// {{{ JSAudioNodeBuffers
class JSAudioNodeBuffers : public ClassMapper<JSAudioNodeBuffers>
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeBuffers::ExposeClass<0>(cx, "AudioNodeFrame");
    }

    JSAudioNodeBuffers(JSAudioContext *cx,
                       float **data,
                       JSAudioNodeCustomBase *node,
                       AV::AudioParameters *params);

    float *getBuffer(unsigned int idx);
    // Return the number of frame
    unsigned int getCount()
    {
        return m_Count;
    }
    // Return the size in bytes of a frame
    unsigned int getSize()
    {
        return m_Size;
    }

private:
    unsigned int m_Count = 0;
    unsigned int m_Size  = 0;
    JS::Heap<JSObject *> m_DataArray;
};
// }}}

// {{{ JSAudioNodeThreaded
class JSAudioNodeThreaded : public ClassMapper<JSAudioNodeThreaded>
{
public:
    static const int THREADED_MESSAGE = 1337;

    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeThreaded::ExposeClass<0>(cx, "AudioNodeThreaded",
                                            JSCLASS_HAS_RESERVED_SLOTS(1));
    }

    static JSFunctionSpec *ListMethods();

    void set(JS::HandleValue val);

    JSAudioNodeThreaded(JSAudioNodeCustomBase *node);

    virtual ~JSAudioNodeThreaded()
    {
        JSAudioContext *jaudio = JSAudioContext::GetContext();
        JSAutoRequest ar(m_Cx);
        JS::RootedObject global(m_Cx, jaudio->m_JsGlobalObj);
        JSAutoCompartment ac(m_Cx, global);

        JS_SetReservedSlot(this->getJSObject(), 0, JS::NullHandleValue);
        JS_SetReservedSlot(global, 0, JS::NullHandleValue);
    }

protected:
    NIDIUM_DECL_JSCALL(set);
    NIDIUM_DECL_JSCALL(get);
    NIDIUM_DECL_JSCALL(send);

private:
    JSAudioNodeCustomBase *m_Node;
    JS::Heap<JSObject *> m_HashObj;
};
// }}}
} // namespace Binding
} // namespace Nidium


#endif
