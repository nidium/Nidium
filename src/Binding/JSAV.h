#ifndef binding_jsav_h__
#define binding_jsav_h__

#include "Core/Messages.h"
#include "Binding/JSExposer.h"


#include "Audio.h"

#include "Graphics/SkiaContext.h"

#define CUSTOM_SOURCE_SEND 100

using Nidium::Core::SharedMessages;

namespace Nidium {
namespace Binding {

#define NJS (NidiumJS::GetObject(m_Cx))

#define CHECK_INVALID_CTX(obj)                       \
    if (!obj) {                                      \
        JS_ReportError(cx, "Invalid Audio context"); \
        return false;                                \
    }

enum
{
    NODE_EV_PROP_DATA,
    NODE_EV_PROP_SIZE,
    NODE_PROP_TYPE,
    NODE_PROP_INPUT,
    NODE_PROP_OUTPUT,
    AUDIO_PROP_VOLUME,
    AUDIO_PROP_BUFFERSIZE,
    AUDIO_PROP_CHANNELS,
    AUDIO_PROP_SAMPLERATE,
    VIDEO_PROP_WIDTH,
    VIDEO_PROP_HEIGHT,
    VIDEO_PROP_ONFRAME,
    VIDEO_PROP_CANVAS,
    SOURCE_PROP_POSITION,
    SOURCE_PROP_DURATION,
    SOURCE_PROP_METADATA,
    SOURCE_PROP_BITRATE,
    CUSTOM_SOURCE_PROP_SEEK
};

const char *JSAVEventRead(int ev);

JS::Value consumeSourceMessage(JSContext *cx,
                                      const SharedMessages::Message &msg);

class NidiumJS;

class Canvas2DContext;

// {{{ JSAVMessageCallback
struct JSAVMessageCallback
{
    JS::PersistentRootedObject m_Callee;
    int m_Ev;
    int m_Arg1;
    int m_Arg2;

    JSAVMessageCallback(
        JSContext *cx, JS::HandleObject callee, int ev, int arg1, int arg2)
        : m_Callee(cx, callee), m_Ev(ev), m_Arg1(arg1), m_Arg2(arg2){};
};
// }}}

// {{{ JSTransferableFunction
class JSTransferableFunction
{
public:
    JSTransferableFunction(JSContext *destCx)
        : m_Data(NULL), m_Bytes(0), m_Fn(destCx), m_DestCx(destCx)
    {
        m_Fn.get().setUndefined();
    }

    bool prepare(JSContext *cx, JS::HandleValue val);
    bool call(JS::HandleObject obj,
              JS::HandleValueArray params,
              JS::MutableHandleValue rval);

    ~JSTransferableFunction();

private:
    bool transfert();

    uint64_t *m_Data;
    size_t m_Bytes;

    JS::PersistentRootedValue m_Fn;
    JSContext *m_DestCx;
};
// }}}

// {{{ JSAVSource
class JSAVSource
{
public:
    static inline bool
    PropSetter(AV::AVSource *source, uint8_t id, JS::MutableHandleValue vp);
    static inline void GetMetadata(JSContext *cx, AV::AVSource *source, JS::MutableHandleValue vp);
    static inline bool PropGetter(AV::AVSource *source,
                                  JSContext *ctx,
                                  uint8_t id,
                                  JS::MutableHandleValue vp);
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
