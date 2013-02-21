#ifndef nativejsav_h__
#define nativejsav_h__

#include "NativeJSExposer.h"
#include <NativeAudio.h>
#include <NativeAudioNode.h>

#include "NativeVideo.h"
#include "NativeSkia.h"

#define NATIVE_AUDIO_THREAD_MESSAGE_CALLBACK 0x100

enum {
    NODE_EV_PROP_DATA, 
    NODE_EV_PROP_SIZE,
    NODE_CUSTOM_PROP_BUFFER
};

class NativeJS;
class NativeJSAudioNode;

class NativeJSAudio: public NativeJSExposer
{
    public :
        NativeJSAudio(int size, int channels, int frequency);

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
        pthread_t threadDecode;
        pthread_t threadQueue;

        pthread_cond_t shutdowned;
        pthread_mutex_t shutdownLock;

        JSObject *jsobj;
        JSObject *gbl;

        JSRuntime *rt;
        JSContext *tcx;
        const char *fun;

        bool createContext();
        static void shutdownCallback(NativeAudioNode *dummy, void *custom);
        static void shutdown();

        static void registerObject(JSContext *cx);

        ~NativeJSAudio();
    private : 
        static NativeJSAudio *instance;
};

class NativeJSAudioNode: public NativeJSExposer
{
    public :
        NativeJSAudioNode(NativeAudio::Node type, int in, int out, NativeJSAudio *audio) 
            :  audio(audio), type(type), bufferFn(NULL), bufferObj(NULL), bufferStr(NULL), 
               nodeObj(NULL), hashObj(NULL), finalized(false), arrayContent(NULL) 
        { 

            if (type == NativeAudio::CUSTOM) {
                pthread_cond_init(&this->shutdowned, NULL);
                pthread_mutex_init(&this->shutdownLock, NULL);
            }

            this->node = audio->audio->createNode(type, in, out);
        }

        NativeJSAudioNode(NativeAudio::Node type, NativeAudioNode *node, NativeJSAudio *audio) 
            :  audio(audio), node(node), type(type), bufferFn(NULL), bufferObj(NULL), bufferStr(NULL), 
               nodeObj(NULL), hashObj(NULL), finalized(false), arrayContent(NULL) 
        { 
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

        struct MessageCallback {
            JSObject *callee;
            const char *prop;
            int *value;

            MessageCallback(JSObject *callee, const char *prop, int *value)
                : callee(callee), prop(prop), value(value) {};
            ~MessageCallback() {
                delete value;
            }
        };

        // Common
        NativeJS *njs;
        NativeJSAudio *audio;
        NativeAudioNode *node;
        JSObject *jsobj;
        NativeAudio::Node type;

        // Custom node
        static void customCbk(const struct NodeEvent *ev);
        static void ctxCallback(NativeAudioNode *node, void *custom);
        static void setPropCallback(NativeAudioNode *node, void *custom);
        static void shutdownCallback(NativeAudioNode *node, void *custom);
        bool createHashObj();
        JSFunction *bufferFn;
        JSObject *bufferObj;
        const char *bufferStr;
        JSObject *nodeObj;
        JSObject *hashObj;
        bool finalized;

        pthread_cond_t shutdowned;
        pthread_mutex_t shutdownLock;

        // Source node
        void *arrayContent;
        static void sourceCbk(const struct TrackEvent *ev);

        static void registerObject(JSContext *cx);
};

class NativeJSVideo : public NativeJSExposer
{
    public :
        NativeJSVideo(NativeJSAudio *audio, NativeSkia *nskia, JSContext *cx);

        NativeVideo *video;
        JSObject *jsobj;
        JSContext *cx;
        NativeJSAudio *jaudio;
        NativeSkia *nskia;
        void *arrayContent;

        static void registerObject(JSContext *cx);
        static void frameCallback(int width, int height, uint8_t *data, void *custom);
        
        ~NativeJSVideo();
};

#endif
