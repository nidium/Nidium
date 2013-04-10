#ifndef nativeaudio_h__
#define nativeaudio_h__

#include <pthread.h>
#include <stdint.h>
#include "NativeAV.h"
#include "native_netlib.h"

#if 0
  #define SPAM(a) printf a
#else
  #define SPAM(a) (void)0
#endif

#define NATIVE_AVDECODE_BUFFER_SAMPLES  16384
#define NATIVE_RESAMPLER_BUFFER_SAMPLES 1024
#define NATIVE_AUDIO_CHECK_EXIT_THREAD if (audio->threadShutdown) {\
    SPAM(("Exiting"));\
    return NULL;\
}\

class NativeJS;
class NativeAudioTrack;
class NativeAudioNode;
class NativeAudioNodeTarget;
struct NodeLink;
struct Coro;

class NativeSharedMessages;

struct PaUtilRingBuffer;
struct PaStreamCallbackTimeInfo;
typedef void PaStream;
typedef unsigned long PaStreamCallbackFlags;

class NativeAudio
{
    public:
        NativeAudio(ape_global *net, int bufferSize, int channels, int sampleRate);

        friend class NativeVideo;

        enum SampleFormat {
            FLOAT32 = sizeof(float), 
            DOUBLE = sizeof(double), 
            INT32 = sizeof(int32_t),
            INT24 = sizeof(int), 
            INT16 = sizeof(int16_t), 
            UINT8 = sizeof(uint8_t)
        };

        enum Node {
            SOURCE, GAIN, TARGET, CUSTOM
        };

        ape_global *net;

        NativeAudioParameters *outputParameters;
        NativeAudioParameters *inputParameters;

        // TODO : Use friend class instead of exposing those var
        float *nullBuffer;
        NativeSharedMessages *sharedMsg;
        pthread_cond_t bufferNotEmpty, queueHaveData, queueHaveSpace;
        pthread_mutex_t recurseLock;
        PaUtilRingBuffer *rBufferOut;

        static void *queueThread(void *args);
        static void *decodeThread(void *args);
        void bufferData();

        int openOutput();
        int openInput();

        NativeAudioNodeTarget *output;
        int tracksCount;

        NativeAudioTrack *addTrack(int out, bool external);
        void removeTrack(NativeAudioTrack *track);
        NativeAudioNode *createNode(NativeAudio::Node node, int input, int ouput);
        bool connect(NodeLink *input, NodeLink *output);
        bool disconnect(NodeLink *input, NodeLink *output);

        static inline int getSampleSize(int sampleFmt);
        double getLatency();

        void wakeup();
        void shutdown();

        Coro *mainCoro;

        ~NativeAudio();
    private:
        struct NativeAudioTracks {
            NativeAudioTrack *curr;

            NativeAudioTracks *next;
            NativeAudioTracks *prev;
        };

        PaStream *inputStream;
        PaStream *outputStream;

        float *rBufferOutData;
        float *cbkBuffer;

        pthread_mutex_t decodeLock, queueLock, shutdownLock;
        pthread_t threadDecode;
        pthread_t threadQueue;

        bool haveData, notEmpty;
        bool threadShutdown;

        NativeAudioTracks *tracks;
        int queueCount;

        void processQueue();
        inline bool haveSourceActive(bool excludeExternal);
        static int paOutputCallback(const void *inputBuffer, void *outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void *userData);

        int paOutputCallbackMethod(const void *inputBuffer, void *outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags);

        bool convert();
};


#endif
