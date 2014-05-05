#ifndef nativejsav_h__
#define nativejsav_h__

#include "NativeJSExposer.h"
#include <NativeAudio.h>
#include <NativeAudioNode.h>
#include <NativeVideo.h>
#include <NativeMessages.h>

#include "NativeSkia.h"

#define CUSTOM_SOURCE_SEND 100

enum {
    NODE_EV_PROP_DATA, 
    NODE_EV_PROP_SIZE,
    NODE_PROP_TYPE,
    NODE_CUSTOM_PROP_PROCESS,
    NODE_CUSTOM_PROP_INIT,
    NODE_CUSTOM_PROP_SETTER,
    AUDIO_PROP_VOLUME,
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

struct NativeJSAVMessageCallback {
    JSObject *callee;
    int ev;
    int arg1;
    int arg2;

    NativeJSAVMessageCallback(JSObject *callee, int ev, int arg1, int arg2)
        : callee(callee), ev(ev), arg1(arg1), arg2(arg2) {};
};

class JSTransferableFunction
{
    public :
        JSTransferableFunction() : m_Data(NULL), m_Fn(NULL), m_DestCx(NULL) 
        {
        }

        bool prepare(JSContext *cx, jsval val);
        bool call(JSContext *cx, JSObject *obj, int argc, jsval *params, jsval *rval);

        ~JSTransferableFunction();
   private :
        bool transfert(JSContext *destCx);

        uint64_t *m_Data;
        size_t m_Bytes;

        JS::Value *m_Fn;
        JSContext *m_DestCx;
};

class NativeJSAVSource
{
    public:
        static inline int open(NativeAVSource *source, JSContext *cx, unsigned argc, jsval *vp);
        static inline int play();
        static inline int pause();
        static inline int stop();

        static inline bool propSetter(NativeAVSource *source, int id, JSMutableHandleValue vp);
        static inline bool propGetter(NativeAVSource *source, JSContext *ctx, int id, JSMutableHandleValue vp);
};

class NativeJSAudio: public NativeJSExposer<NativeJSAudio>
{
    public :
        static NativeJSAudio *getContext(JSContext *cx, JSObject *obj, int bufferSize, int channels, int sampleRate);
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

        NativeAudio *audio;

        Nodes *nodes;

        pthread_t threadIO;

        NATIVE_PTHREAD_VAR_DECL(m_ShutdownWait)

        JSObject *gbl;

        JSRuntime *rt;
        JSContext *tcx;

        NativeJSAudioNode *target;

        bool createContext();
        bool run(char *str);
        static void ctxCallback(NativeAudioNode *node, void *custom);
        static void runCallback(NativeAudioNode *node, void *custom);
        static void shutdownCallback(NativeAudioNode *dummy, void *custom);
        void unroot();

        void onMessage(const NativeSharedMessages::Message &msg);
        static void registerObject(JSContext *cx);

        ~NativeJSAudio();
    private : 
        NativeJSAudio(NativeAudio *audio, JSContext *cx, JSObject *obj);
        static NativeJSAudio *instance;
};

class NativeJSAudioNode: public NativeJSExposer<NativeJSAudioNode>, public NativeMessages
{
    public :
        NativeJSAudioNode(NativeAudio::Node type, int in, int out, NativeJSAudio *audio) 
            :  audio(audio), node(NULL), type(type), nodeObj(NULL), hashObj(NULL), 
               arrayContent(NULL), m_IsDestructing(false)
        { 
            this->jsobj = NULL;

            try {
                this->node = audio->audio->createNode(type, in, out);
            } catch (NativeAudioNodeException *e) {
                throw;
            }

            if (type == NativeAudio::CUSTOM || type == NativeAudio::CUSTOM_SOURCE) {
                NATIVE_PTHREAD_VAR_INIT(&this->m_ShutdownWait);
            }

            this->add();

            for (int i = 0; i < END_FN; i++) {
                m_TransferableFuncs[i] = NULL;
            }
        }

        NativeJSAudioNode(NativeAudio::Node type, NativeAudioNode *node, NativeJSAudio *audio) 
            :  audio(audio), node(node), type(type), 
               hashObj(NULL), arrayContent(NULL) 
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
        NativeJS *njs;
        NativeJSAudio *audio;
        NativeAudioNode *node;
        NativeAudio::Node type;

        // Custom node
        JSTransferableFunction *m_TransferableFuncs[END_FN];
        static void customCallback(const struct NodeEvent *ev);
        static void setPropCallback(NativeAudioNode *node, void *custom);
        static void shutdownCallback(NativeAudioNode *node, void *custom);
        static void initCustomObject(NativeAudioNode *node, void *custom);
        bool createHashObj();

        JSObject *nodeObj;
        JSObject *hashObj;

        NATIVE_PTHREAD_VAR_DECL(m_ShutdownWait)

        // Source node
        void *arrayContent;
        void onMessage(const NativeSharedMessages::Message &msg);
        static void onEvent(const struct NativeAVSourceEvent *cev);

        // Custom source node
        static void seekCallback(NativeAudioCustomSource *node, double seekTime, void *custom);
        static bool propSetter(NativeJSAudioNode *node, JSContext *cx, 
                int id, JSMutableHandleValue vp);

        static void registerObject(JSContext *cx);
    private : 
        void add();
        bool m_IsDestructing;
};

class NativeJSVideo : public NativeJSExposer<NativeJSVideo>, public NativeMessages
{
    public :
        NativeJSVideo(NativeSkia *nskia, JSContext *cx);

        NativeVideo *video;

        JSObject *audioNode;
        void *arrayContent;

        int width;
        int height;

        void stopAudio();

        static void registerObject(JSContext *cx);
        static void frameCallback(uint8_t *data, void *custom);
        void onMessage(const NativeSharedMessages::Message &msg);
        static void onEvent(const struct NativeAVSourceEvent *cev);
        
        ~NativeJSVideo();
    private :
        bool m_IsDestructing;
        NativeSkia *nskia;
        JSContext *cx;
};

#endif
