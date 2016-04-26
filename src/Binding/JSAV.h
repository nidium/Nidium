#ifndef binding_jsav_h__
#define binding_jsav_h__

#include <Core/Messages.h>
#include <Binding/JSExposer.h>

#include <Audio.h>
#include <AudioNode.h>
#include <Video.h>

#include <Graphics/SkiaContext.h>

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

class NidiumJS;

class JSAudioNode;
class Canvas2DContext;

// {{{ JSAVMessageCallback
struct JSAVMessageCallback {
    JS::PersistentRootedObject callee;
    int ev;
    int arg1;
    int arg2;

    JSAVMessageCallback(JSContext *cx, JS::HandleObject callee, int ev, int arg1, int arg2)
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

// {{{ JSAVSource
class JSAVSource
{
    public:
        static inline bool PropSetter(AV::AVSource *source, uint8_t id, JS::MutableHandleValue vp);
        static inline bool PropGetter(AV::AVSource *source, JSContext *ctx, uint8_t id, JS::MutableHandleValue vp);
};
// }}}

// {{{ JSAudio
class JSAudio: public JSExposer<JSAudio>
{
    public :
        static JSAudio *GetContext(JSContext *cx, JS::HandleObject obj, \
            unsigned int bufferSize, unsigned int channels, unsigned int sampleRate);
        static JSAudio *GetContext();

        struct Nodes {
            JSAudioNode *curr;

            Nodes *prev;
            Nodes *next;

            Nodes(JSAudioNode *curr, Nodes *prev, Nodes *next)
                : curr(curr), prev(prev), next(next) {}
            Nodes()
                : curr(NULL), prev(NULL), next(NULL) {}
        };

        AV::Audio *m_Audio;
        Nodes *m_Nodes;
        pthread_t m_ThreadIO;

        NIDIUM_PTHREAD_VAR_DECL(m_ShutdownWait)

        JS::Heap<JSObject*> m_JsGlobalObj;

        JSRuntime *m_JsRt;
        JSContext *m_JsTcx;
        JSAudioNode *m_Target;

        bool createContext();
        void initNode(JSAudioNode *node, JS::HandleObject jnode, JS::HandleString name);
        bool run(char *str);
        static void CtxCallback(void *custom);
        static void RunCallback(void *custom);
        static void ShutdownCallback(void *custom);
        void unroot();

        static void RegisterObject(JSContext *cx);

        ~JSAudio();
    private :
        JSAudio(AV::Audio *audio, JSContext *cx, JS::HandleObject obj);
        static JSAudio *m_Instance;
};
// }}}

// {{{ JSAudioNode
class JSAudioNode: public JSExposer<JSAudioNode>, public Core::Messages
{
    public :
        JSAudioNode(JS::HandleObject obj, JSContext *cx,
            AV::Audio::Node type, int in, int out, JSAudio *audio)
            :   JSExposer<JSAudioNode>(obj, cx), m_nJs(NULL),
                m_Audio(audio), m_Node(NULL), m_NodeType(type), m_NodeObj(nullptr), m_HashObj(nullptr),
                m_ArrayContent(NULL), m_IsDestructing(false)
        {
            m_JSObject = NULL;

            try {
                m_Node = audio->m_Audio->createNode(type, in, out);
            } catch (AV::AudioNodeException *e) {
                throw;
            }

            if (type == AV::Audio::CUSTOM || type == AV::Audio::CUSTOM_SOURCE) {
                NIDIUM_PTHREAD_VAR_INIT(&m_ShutdownWait);
            }

            this->add();

            for (int i = 0; i < END_FN; i++) {
                m_TransferableFuncs[i] = NULL;
            }
        }

        JSAudioNode(JS::HandleObject obj, JSContext *cx,
               AV::Audio::Node type, AV::AudioNode *node, JSAudio *audio)
            :  JSExposer<JSAudioNode>(obj, cx), m_nJs(NULL), m_Audio(audio), m_Node(node), m_NodeType(type),
               m_NodeObj(nullptr), m_HashObj(nullptr), m_ArrayContent(NULL), m_IsDestructing(false)
        {
            this->add();

            for (int i = 0; i < END_FN; i++) {
                m_TransferableFuncs[i] = NULL;
            }
        }

        ~JSAudioNode();

        struct Message {
            JSAudioNode *jsNode;
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
        JSAudio *m_Audio;
        AV::AudioNode *m_Node;
        AV::Audio::Node m_NodeType;

        // Custom m_Node
        JSTransferableFunction *m_TransferableFuncs[END_FN];
        static void CustomCallback(const struct AV::NodeEvent *ev);
        static void SetPropCallback(AV::AudioNode *node, void *custom);
        static void ShutdownCallback(AV::AudioNode *node, void *custom);
        static void InitCustomObject(AV::AudioNode *node, void *custom);
        static void DeleteTransferableFunc(AV::AudioNode *node, void *custom);
        bool createHashObj();

        JS::PersistentRootedObject *m_NodeObj;
        JS::PersistentRootedObject *m_HashObj;

        NIDIUM_PTHREAD_VAR_DECL(m_ShutdownWait)

        // Source m_Node
        void *m_ArrayContent;
        void onMessage(const Core::SharedMessages::Message &msg);
        static void onEvent(const struct AV::AVSourceEvent *cev);

        // Custom source m_Node
        static void SeekCallback(AV::AudioCustomSource *node, double seekTime, void *custom);
        static bool PropSetter(JSAudioNode *node, JSContext *cx,
                uint8_t id, JS::MutableHandleValue vp);

        static void RegisterObject(JSContext *cx);
    private :
        void add();
        bool m_IsDestructing;
};
// }}}

// {{{ JSVideo
class JSVideo : public JSExposer<JSVideo>, public Core::Messages
{
    public :
        JSVideo(JS::HandleObject obj, Canvas2DContext *canvasCtx, JSContext *cx);

        AV::Video *m_Video;

        JS::Heap<JSObject *> m_AudioNode;
        void *m_ArrayContent;

        int m_Width;
        int m_Height;
        int m_Left;
        int m_Top;

        void stopAudio();
        void setSize(int width, int height);
        void close();

        static void RegisterObject(JSContext *cx);
        static void FrameCallback(uint8_t *data, void *custom);
        void onMessage(const Core::SharedMessages::Message &msg);
        static void onEvent(const struct AV::AVSourceEvent *cev);

        ~JSVideo();
    private :
        void releaseAudioNode();
        bool m_IsDestructing;
        Canvas2DContext *m_CanvasCtx;
        JSContext *cx;
};
// }}}

} // namespace Nidium
} // namespace Binding

#endif

