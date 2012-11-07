#ifndef nativejsaudio_h__
#define nativejsaudio_h__

#include "NativeJSExposer.h"
#include <NativeAudio.h>
#include <NativeAudioNode.h>

/*
class NativeJSAudioLink 
{
    public :
        NativeJSAudioLink(NativeAudioLink *link);
        ~NativeJSAudioLink();
}

*/

class NativeJSAudio: public NativeJSExposer
{
    public :
        NativeJSAudio(int size, int channels, int frequency);
        ~NativeJSAudio();

        NativeAudio *audio;

        pthread_t threadIO;
        pthread_t threadDecode;
        pthread_t threadQueue;

        JSObject *jsobj;

        static void registerObject(JSContext *cx);
};

class NativeJSAudioNode: public NativeJSExposer
{
    public :
        NativeJSAudioNode(NativeAudioNode *node) : node(node), arrayBuff(NULL) {}
        ~NativeJSAudioNode();

        NativeAudioNode *node;
        JSObject *jsobj;
        jsval *arrayBuff;

        static void registerObject(JSContext *cx);
};

#endif
