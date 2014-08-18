#ifndef nativeaudio_h__
#define nativeaudio_h__

#include <pthread.h>
#include <stdint.h>
#include "NativeAV.h"
#include "native_netlib.h"
#include <jspubtd.h>

#if 0
  #define SPAM(a) \
    printf(">%lld / ", av_gettime()/1000); \
    printf a
#else
  #define SPAM(a) (void)0
#endif

#define NATIVE_RESAMPLER_BUFFER_SAMPLES 1024
#define NATIVE_AUDIO_CHECK_EXIT_THREAD if (audio->threadShutdown) {\
    SPAM(("Exiting\n"));\
    return NULL;\
}\

class NativeJS;
class NativeAudioSource;
class NativeAudioNode;
class NativeAudioNodeTarget;
struct NodeLink;

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
        friend class NativeAudioSource;
        friend class NativeAudioCustomSource;

        enum SampleFormat {
            FLOAT32 = sizeof(float), 
            DOUBLE = sizeof(double), 
            INT32 = sizeof(int32_t),
            INT24 = sizeof(int), 
            INT16 = sizeof(int16_t), 
            UINT8 = sizeof(uint8_t)
        };

        enum Node {
            SOURCE, GAIN, TARGET, CUSTOM, CUSTOM_SOURCE, REVERB, DELAY,
            STEREO_ENHANCER
        };

        ape_global *net;

        NativeAudioParameters *outputParameters;
        NativeAudioParameters *inputParameters;

        NativeSharedMessages *sharedMsg;

        int sourcesCount;

        static void *queueThread(void *args);
        static void *decodeThread(void *args);
        void bufferData();

        int openOutput();
        int openInput();

        NativeAudioNodeTarget *output;

        NativeAudioNode *addSource(NativeAudioNode *source, bool externallyManaged);
        void removeSource(NativeAudioSource *source);
        NativeAudioNode *createNode(NativeAudio::Node node, int input, int ouput);
        bool connect(NodeLink *input, NodeLink *output);
        bool disconnect(NodeLink *input, NodeLink *output);
        void setVolume(float volume);
        float getVolume();

        static inline int getSampleSize(int sampleFmt);
        double getLatency();

        void wakeup();
        void shutdown();

        static void sourceNeedWork(void *ptr);
        void lockSources();
        void unlockSources();

        void lockQueue();
        void unlockQueue();

        JSContext *getMainCtx() const {
            return m_MainCtx;
        }

        void setMainCtx(JSContext *ctx) {
            m_MainCtx = ctx;
        }

        ~NativeAudio();
    private:
        struct NativeAudioSources {
            NativeAudioNode *curr;
            bool externallyManaged;

            NativeAudioSources *next;
            NativeAudioSources *prev;
        };

        PaStream *inputStream;
        PaStream *outputStream;

        float *rBufferOutData;
        float volume;

        pthread_t threadDecode;
        pthread_t threadQueue;

        NATIVE_PTHREAD_VAR_DECL(queueHaveData)
        NATIVE_PTHREAD_VAR_DECL(queueHaveSpace)
        NATIVE_PTHREAD_VAR_DECL(queueNeedData)
        NATIVE_PTHREAD_VAR_DECL(queueMessagesFlushed)

        pthread_mutex_t shutdownLock, recurseLock, sourcesLock;

        PaUtilRingBuffer *rBufferOut;

        bool haveData, notEmpty;
        bool m_SourceNeedWork;
        bool m_SharedMsgFlush;
        bool threadShutdown;

        NativeAudioSources *sources;
        int queueCount;

        void readMessages();
        void readMessages(bool flush);

        void processQueue();
        bool canWriteFrame();

        JSContext *m_MainCtx;

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

};


#endif
