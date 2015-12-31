#ifndef nativeaudio_h__
#define nativeaudio_h__

#include <stdint.h>
#include <pthread.h>

#include <native_netlib.h>
#include <jspubtd.h>

#include "NativeAV.h"

#if 0
  #define SPAM(a) \
    fprintf(stdout, ">%lld / ", av_gettime()/1000); \
    printf a
#else
  #define SPAM(a) (void)0
#endif

#define NATIVE_RESAMPLER_BUFFER_SAMPLES 1024
#define NATIVE_AUDIO_CHECK_EXIT_THREAD if (audio->m_ThreadShutdown) {\
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

        ape_global *m_Net;

        NativeAudioParameters *m_OutputParameters;
        NativeAudioParameters *m_InputParameters;
        NativeSharedMessages *m_SharedMsg;
        int m_SourcesCount;
        NativeAudioNodeTarget *m_Output;

        static void *queueThread(void *args);
        static void *decodeThread(void *args);
        void bufferData();
        int openOutput();
        int openInput();
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

        PaStream *m_InputStream;
        PaStream *m_OutputStream;
        float *m_rBufferOutData;
        float m_volume;
        pthread_t m_ThreadDecode;
        pthread_t m_ThreadQueue;
        NATIVE_PTHREAD_VAR_DECL(m_QueueHaveData)
        NATIVE_PTHREAD_VAR_DECL(m_QueueHaveSpace)
        NATIVE_PTHREAD_VAR_DECL(m_QueueNeedData)
        NATIVE_PTHREAD_VAR_DECL(m_QueueMessagesFlushed)
        pthread_mutex_t m_ShutdownLock, m_RecurseLock, m_SourcesLock;
        PaUtilRingBuffer *m_rBufferOut;
        bool m_HaveData, m_NotEmpty;
        bool m_SourceNeedWork;
        bool m_SharedMsgFlush;
        bool m_ThreadShutdown;
        NativeAudioSources *m_Sources;
        int m_QueueCount;

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

