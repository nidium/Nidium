/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsav_h__
#define binding_jsav_h__

#include "Core/Messages.h"
#include "Binding/ClassMapper.h"
#include "AV/Audio.h"

#define CUSTOM_SOURCE_SEND 100

using Nidium::Core::SharedMessages;

namespace Nidium {
namespace Binding {

#define JSAV_PASSTHROUGH_G(CLASS, NAME)         \
    NIDIUM_DECL_JSGETTER(NAME)                  \
    {                                           \
        return CLASS::JSGetter__##NAME(cx, vp); \
    };

#define JSAV_PASSTHROUGH_S(CLASS, NAME)         \
    NIDIUM_DECL_JSSETTER(NAME)                  \
    {                                           \
        return CLASS::JSSetter__##NAME(cx, vp); \
    };

#define JSAV_PASSTHROUGH_CALL(CLASS, NAME)  \
    NIDIUM_DECL_JSCALL(NAME)                \
    {                                       \
        return CLASS::JS__##NAME(cx, args); \
    };

#define NJS (NidiumJS::GetObject(m_Cx))

#define GET_AUDIO_CONTEXT(cx)                              \
    JSAudioContext *jaudio = JSAudioContext::GetContext(); \
    if (!jaudio) {                                         \
        JS_ReportErrorUTF8(cx, "No Audio context");        \
        return false;                                      \
    }

#define JS_AUDIO_THREAD_PROLOGUE()                               \
    JSAudioContext *audioContext = JSAudioContext::GetContext(); \
    JSContext *cx = audioContext->m_JsTcx;                       \
    JSAutoRequest ar(cx);                                        \
    JS::RootedObject global(cx, audioContext->m_JsGlobalObj);    \
    JSAutoCompartment ac(cx, global);


class NidiumJS;

class Canvas2DContext;

// {{{ JSAVSourceEventInterface
class JSAVSourceEventInterface : public Core::Messages
{
public:
    JSAVSourceEventInterface(){};
    virtual ~JSAVSourceEventInterface(){};

    void listenSourceEvents(AV::AVSourceEventInterface *eventInterface);

    /*
        Receive events (play, pause, stop, ...) and forward them
        trought postMessage() to the main thread
     */
    static void onEvent(const struct AV::AVSourceEvent *cev);

    /*
        Handle common events for sources
     */
    virtual void onMessage(const Core::SharedMessages::Message &msg) override;

protected:
    /*
        Virtual function to be implemented by the derived class if
        it needs to do custom processing on the received events.
     */
    virtual void onSourceMessage(const Core::SharedMessages::Message &msg){}
    virtual bool isReleased() = 0;
    virtual JSContext *getJSContext() = 0;
    virtual bool fireJSEvent(const char *name, JS::MutableHandleValue val) = 0;
};
// }}}

// {{{ JSAVSourceBase
class JSAVSourceBase
{
public:
    void *m_ArrayContent = nullptr;

    virtual ~JSAVSourceBase()
    {
        if (m_ArrayContent != nullptr) {
            free(m_ArrayContent);
        }
    }

    NIDIUM_DECL_JSCALL(_open);
    NIDIUM_DECL_JSCALL(_play);
    NIDIUM_DECL_JSCALL(_pause);
    NIDIUM_DECL_JSCALL(_stop);
    NIDIUM_DECL_JSCALL(_close);

    NIDIUM_DECL_JSGETTERSETTER(_position);
    NIDIUM_DECL_JSGETTER(_duration);
    NIDIUM_DECL_JSGETTER(_metadata);
    NIDIUM_DECL_JSGETTER(_bitrate);

protected:
    virtual AV::AVSource *getSource() = 0;
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
