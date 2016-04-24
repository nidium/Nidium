#ifndef binding_jsav_h__
#define binding_jsav_h__

#include <Core/Messages.h>
#include <Binding/JSExposer.h>

#include <Audio.h>
#include <AudioNode.h>
#include <Video.h>

#include <Graphics/Skia.h>

#define CUSTOM_SOURCE_SEND 100

namespace Nidium {
namespace Binding {

enum {
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

namespace Nidium {
    namespace Binding {
        class NidiumJS;
    }
}

class NativeJSAudioNode;
class NativeCanvas2DContext;

// {{{ NativeJSAVMessageCallback
struct NativeJSAVMessageCallback {
    JS::PersistentRootedObject callee;
    int ev;
    int arg1;
    int arg2;

    NativeJSAVMessageCallback(JSContext *cx, JS::HandleObject callee, int ev, int arg1, int arg2)
        : callee(cx, callee), ev(ev), arg1(arg1), arg2(arg2) {};
};
// }}}

// {{{ JSTransferableFunction
class JSTransferableFunction
{
    public :
        JSTransferableFunction(JSContext *destCx) : m_Data(NULL), m_Bytes(0), m_Fn(destCx), m_DestCx(destCx)
        {
            m_Fn.get().setUndefined();
        }

        bool prepare(JSContext *cx, JS::HandleValue val);
        bool call(JS::HandleObject obj, JS::HandleValueArray params, JS::MutableHandleValue rval);

        ~JSTransferableFunction();
   private :
        bool transfert();

        uint64_t *m_Data;
        size_t m_Bytes;

        JS::PersistentRootedValue m_Fn;
        JSContext *m_DestCx;
};
// }}}

// {{{ NativeJSAVSource
class NativeJSAVSource
{
    public:
        static inline bool propSetter(AV::NativeAVSource *source, uint8_t id, JS::MutableHandleValue vp);
        static inline bool propGetter(AV::NativeAVSource *source, JSContext *ctx, uint8_t id, JS::MutableHandleValue vp);
};
// }}}

// {{{ NativeJSAudio
class NativeJSAudio: public JSExposer<NativeJSAudio>
{
    public :
        static NativeJSAudio *getContext(JSContext *cx, JS::HandleObject obj, \
            unsigned int bufferSize, unsigned int channels, unsigned int sampleRate);
        static NativeJSAudio *getContext();

        struct Nodes {
            NativeJSAudioNode *curr;

            Nodes *prev;
            Nodes *next;

            Nodes(NativeJSAudioNode *curr, Nodes *prev, Nodes *next)
                : curr(curr), prev(prev), next(next) {}
            Nodes()
                : curr(NULL), prev(NULL), next(NULL) {}
        };

        AV::NativeAudio *m_Audio;
        Nodes *m_Nodes;
        pthread_t m_ThreadIO;

        NATIVE_PTHREAD_VAR_DECL(m_ShutdownWait)

        JS::Heap<JSObject*> m_JsGlobalObj;

        JSRuntime *m_JsRt;
        JSContext *m_JsTcx;
        NativeJSAudioNode *m_Target;

        bool createContext();
        void initNode(NativeJSAudioNode *node, JS::HandleObject jnode, JS::HandleString name);
        bool run(char *str);
        static void ctxCallback(void *custom);
        static void runCallback(void *custom);
        static void shutdownCallback(void *custom);
        void unroot();

        static void registerObject(JSContext *cx);

        ~NativeJSAudio();
    private :
        NativeJSAudio(AV::NativeAudio *audio, JSContext *cx, JS::HandleObject obj);
        static NativeJSAudio *m_Instance;
};
// }}}

// {{{ NativeJSAudioNode
class NativeJSAudioNode: public JSExposer<NativeJSAudioNode>, public Core::Messages
{
    public :
        NativeJSAudioNode(JS::HandleObject obj, JSContext *cx,
            AV::NativeAudio::Node type, int in, int out, NativeJSAudio *audio)
            :   JSExposer<NativeJSAudioNode>(obj, cx), m_nJs(NULL),
                m_Audio(audio), m_Node(NULL), m_NodeType(type), m_NodeObj(nullptr), m_HashObj(nullptr),
                m_ArrayContent(NULL), m_IsDestructing(false)
        {
            m_JSObject = NULL;

            try {
                m_Node = audio->m_Audio->createNode(type, in, out);
            } catch (AV::NativeAudioNodeException *e) {
                throw;
            }

            if (type == AV::NativeAudio::CUSTOM || type == AV::NativeAudio::CUSTOM_SOURCE) {
                NATIVE_PTHREAD_VAR_INIT(&m_ShutdownWait);
            }

            this->add();

            for (int i = 0; i < END_FN; i++) {
                m_TransferableFuncs[i] = NULL;
            }
        }

        NativeJSAudioNode(JS::HandleObject obj, JSContext *cx,
               AV::NativeAudio::Node type, AV::NativeAudioNode *node, NativeJSAudio *audio)
            :  JSExposer<NativeJSAudioNode>(obj, cx), m_nJs(NULL), m_Audio(audio), m_Node(node), m_NodeType(type),
               m_NodeObj(nullptr), m_HashObj(nullptr), m_ArrayContent(NULL), m_IsDestructing(false)
        {
            this->add();

            for (int i = 0; i < END_FN; i++) {
                m_TransferableFuncs[i] = NULL;
            }
        }

        ~NativeJSAudioNode();

        struct Message {
            NativeJSAudioNode *jsNode;
            char *name;
            struct {
                uint64_t *datap;
                size_t nbytes;
            } clone;
        };

        enum TransferableFunction {
            PROCESS_FN, SETTER_FN, INIT_FN, SEEK_FN, END_FN
        };

        // Common
        NidiumJS *m_nJs;
        NativeJSAudio *m_Audio;
        AV::NativeAudioNode *m_Node;
        AV::NativeAudio::Node m_NodeType;

        // Custom m_Node
        JSTransferableFunction *m_TransferableFuncs[END_FN];
        static void customCallback(const struct AV::NodeEvent *ev);
        static void setPropCallback(AV::NativeAudioNode *node, void *custom);
        static void shutdownCallback(AV::NativeAudioNode *node, void *custom);
        static void initCustomObject(AV::NativeAudioNode *node, void *custom);
        static void deleteTransferableFunc(AV::NativeAudioNode *node, void *custom);
        bool createHashObj();

        JS::PersistentRootedObject *m_NodeObj;
        JS::PersistentRootedObject *m_HashObj;

        NATIVE_PTHREAD_VAR_DECL(m_ShutdownWait)

        // Source m_Node
        void *m_ArrayContent;
        void onMessage(const Core::SharedMessages::Message &msg);
        static void onEvent(const struct AV::NativeAVSourceEvent *cev);

        // Custom source m_Node
        static void seekCallback(AV::NativeAudioCustomSource *node, double seekTime, void *custom);
        static bool propSetter(NativeJSAudioNode *node, JSContext *cx,
                uint8_t id, JS::MutableHandleValue vp);

        static void registerObject(JSContext *cx);
    private :
        void add();
        bool m_IsDestructing;
};
// }}}

// {{{ NativeJSVideo
class NativeJSVideo : public JSExposer<NativeJSVideo>, public Core::Messages
{
    public :
        NativeJSVideo(JS::HandleObject obj, NativeCanvas2DContext *canvasCtx, JSContext *cx);

        AV::NativeVideo *m_Video;

        JS::Heap<JSObject *> m_AudioNode;
        void *m_ArrayContent;

        int m_Width;
        int m_Height;
        int m_Left;
        int m_Top;

        void stopAudio();
        void setSize(int width, int height);
        void close();

        static void registerObject(JSContext *cx);
        static void frameCallback(uint8_t *data, void *custom);
        void onMessage(const Core::SharedMessages::Message &msg);
        static void onEvent(const struct AV::NativeAVSourceEvent *cev);

        ~NativeJSVideo();
    private :
        void releaseAudioNode();
        bool m_IsDestructing;
        NativeCanvas2DContext *m_CanvasCtx;
        JSContext *cx;
};
// }}}

} // namespace Nidium
} // namespace Binding

#endif

