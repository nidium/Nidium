#ifndef nativejsav_h__
#define nativejsav_h__

#include <NativeMessages.h>

#include <NativeJSExposer.h>
#include <NativeAudio.h>
#include <NativeAudioNode.h>
#include <NativeVideo.h>
#include <NativeSkia.h>

#define CUSTOM_SOURCE_SEND 100

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

class NativeJS;
class NativeJSAudioNode;
class NativeCanvas2DContext;

struct NativeJSAVMessageCallback {
    JS::PersistentRootedObject callee;
    int ev;
    int arg1;
    int arg2;

    NativeJSAVMessageCallback(JSContext *cx, JS::HandleObject callee, int ev, int arg1, int arg2)
        : callee(cx, callee), ev(ev), arg1(arg1), arg2(arg2) {};
};

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

class NativeJSAVSource
{
    public:
        static inline bool propSetter(NativeAVSource *source, uint8_t id, JS::MutableHandleValue vp);
        static inline bool propGetter(NativeAVSource *source, JSContext *ctx, uint8_t id, JS::MutableHandleValue vp);
};

class NativeJSAudio: public NativeJSExposer<NativeJSAudio>
{
    public :
        static NativeJSAudio *getContext(JSContext *cx, JS::HandleObject obj, unsigned int bufferSize, unsigned int channels, unsigned int sampleRate);
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

        NativeAudio *m_Audio;
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
        NativeJSAudio(NativeAudio *audio, JSContext *cx, JS::HandleObject obj);
        static NativeJSAudio *m_Instance;
};

class NativeJSAudioNode: public NativeJSExposer<NativeJSAudioNode>, public NativeMessages
{
    public :
        NativeJSAudioNode(JS::HandleObject obj, JSContext *cx,
            NativeAudio::Node type, int in, int out, NativeJSAudio *audio)
            :   NativeJSExposer<NativeJSAudioNode>(obj, cx), m_nJs(NULL),
                m_Audio(audio), m_Node(NULL), m_NodeType(type), m_NodeObj(nullptr), m_HashObj(nullptr),
                m_ArrayContent(NULL), m_IsDestructing(false)
        {
            m_JSObject = NULL;

            try {
                m_Node = audio->m_Audio->createNode(type, in, out);
            } catch (NativeAudioNodeException *e) {
                throw;
            }

            if (type == NativeAudio::CUSTOM || type == NativeAudio::CUSTOM_SOURCE) {
                NATIVE_PTHREAD_VAR_INIT(&m_ShutdownWait);
            }

            this->add();

            for (int i = 0; i < END_FN; i++) {
                m_TransferableFuncs[i] = NULL;
            }
        }

        NativeJSAudioNode(JS::HandleObject obj, JSContext *cx,
               NativeAudio::Node type, NativeAudioNode *node, NativeJSAudio *audio)
            :  NativeJSExposer<NativeJSAudioNode>(obj, cx), m_nJs(NULL), m_Audio(audio), m_Node(node), m_NodeType(type),
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
        NativeJS *m_nJs;
        NativeJSAudio *m_Audio;
        NativeAudioNode *m_Node;
        NativeAudio::Node m_NodeType;

        // Custom m_Node
        JSTransferableFunction *m_TransferableFuncs[END_FN];
        static void customCallback(const struct NodeEvent *ev);
        static void setPropCallback(NativeAudioNode *node, void *custom);
        static void shutdownCallback(NativeAudioNode *node, void *custom);
        static void initCustomObject(NativeAudioNode *node, void *custom);
        static void deleteTransferableFunc(NativeAudioNode *node, void *custom);
        bool createHashObj();

        JS::PersistentRootedObject *m_NodeObj;
        JS::PersistentRootedObject *m_HashObj;

        NATIVE_PTHREAD_VAR_DECL(m_ShutdownWait)

        // Source m_Node
        void *m_ArrayContent;
        void onMessage(const NativeSharedMessages::Message &msg);
        static void onEvent(const struct NativeAVSourceEvent *cev);

        // Custom source m_Node
        static void seekCallback(NativeAudioCustomSource *node, double seekTime, void *custom);
        static bool propSetter(NativeJSAudioNode *node, JSContext *cx,
                uint8_t id, JS::MutableHandleValue vp);

        static void registerObject(JSContext *cx);
    private :
        void add();
        bool m_IsDestructing;
};

class NativeJSVideo : public NativeJSExposer<NativeJSVideo>, public NativeMessages
{
    public :
        NativeJSVideo(JS::HandleObject obj, NativeCanvas2DContext *canvasCtx, JSContext *cx);

        NativeVideo *m_Video;

        JS::PersistentRootedObject m_AudioNode;
        void *m_ArrayContent;

        int m_Width;
        int m_Height;
        int m_Left;
        int m_Top;

        void stopAudio();
        void setSize(int width, int height);

        static void registerObject(JSContext *cx);
        static void frameCallback(uint8_t *data, void *custom);
        void onMessage(const NativeSharedMessages::Message &msg);
        static void onEvent(const struct NativeAVSourceEvent *cev);

        ~NativeJSVideo();
    private :
        bool m_IsDestructing;
        NativeCanvas2DContext *m_CanvasCtx;
        JSContext *cx;
};

#endif

