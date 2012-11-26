#ifndef nativejsaudio_h__
#define nativejsaudio_h__

#include "NativeJSExposer.h"
#include <NativeAudio.h>
#include <NativeAudioNode.h>

enum {
    NODE_EV_PROP_DATA, 
    NODE_EV_PROP_SIZE,
    NODE_CUSTOM_PROP_BUFFER
};

/*
class NativeJSAudioLink 
{
    public :
        NativeJSAudioLink(NativeAudioLink *link);
        ~NativeJSAudioLink();
}

*/

class NativeJS;

class NativeJSAudio: public NativeJSExposer
{
    public :
        NativeJSAudio(int size, int channels, int frequency);

        bool createContext();

        ~NativeJSAudio();

        NativeAudio *audio;

        pthread_t threadIO;
        pthread_t threadDecode;
        pthread_t threadQueue;

        JSObject *jsobj;

        JSRuntime *rt;
        JSContext *tcx;

        static void registerObject(JSContext *cx);
};

class NativeJSAudioNode: public NativeJSExposer
{
    public :
        NativeJSAudioNode(NativeAudioNode *node, NativeJSAudio *audio) 
            :  audio(audio), node(node), bufferFn(NULL), nodeObj(NULL), hashObj(NULL), fn(NULL),arrayBuff(NULL) {}
        NativeJSAudioNode(NativeAudioNode *node) 
            : audio(audio), node(node), bufferFn(NULL), nodeObj(NULL), hashObj(NULL), fn(NULL),arrayBuff(NULL) {}

        ~NativeJSAudioNode();

        struct Message {
            NativeJSAudioNode *jsNode;
            char *name;
            struct {
                uint64_t *datap;
                size_t nbytes;
            } clone;
        };

        // Common
        NativeJS *njs;
        NativeJSAudio *audio;
        NativeAudioNode *node;
        JSObject *jsobj;

        // Custom node
        static void cbk(const struct NodeEvent *ev);
        static void ctxCallback(NativeAudioNode *node, void *custom);
        static void setPropCallback(NativeAudioNode *node, void *custom);
        bool createHashObj();
        JSFunction *bufferFn;
        JSObject *nodeObj;
        JSObject *hashObj;
        jsval fnval;
        JSString *fn;

        // Source node
        jsval *arrayBuff;

        static void registerObject(JSContext *cx);
};

#endif
