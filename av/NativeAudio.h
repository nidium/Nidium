#ifndef nativeaudio_h__
#define nativeaudio_h__

#include <stdint.h>
#include <pthread.h>

#include <native_netlib.h>
#include <jspubtd.h>

#include "NativeAV.h"

#if 0
  #define SPAM(a) \
    fprintf(stdout, ">%ld / ", av_gettime()/1000); \
    printf a
#else
  #define SPAM(a) (void)0
#endif

#define NATIVE_AUDIO_CHECK_EXIT_THREAD if (audio->m_ThreadShutdown) {\
    SPAM(("Exiting\n"));\
    return NULL;\
}\

class NativeAudioSource;
class NativeAudioNode;
class NativeAudioNodeTarget;
struct NodeLink;

class NativeSharedMessages;

struct PaUtilRingBuffer;
struct PaStreamCallbackTimeInfo;
typedef void PaStream;
typedef unsigned long PaStreamCallbackFlags;
typedef int PaStreamCallback(const void *input, void *output, unsigned long frameCount, 
        const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

typedef void (*AudioMessageCallback)(void *custom); 

class NativeAudio
{
    public:
        NativeAudio(ape_global *net, unsigned int bufferSize, unsigned int channels, unsigned int sampleRate);

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

        struct CallbackMessage {
            AudioMessageCallback m_Cbk;
            void *m_Custom;
            CallbackMessage(AudioMessageCallback cbk, void *custom)
                : m_Cbk(cbk), m_Custom(custom) {}
        };

        ape_global *m_Net;

        NativeAudioParameters *m_OutputParameters;
        NativeAudioParameters *m_InputParameters;
        NativeSharedMessages *m_SharedMsg;
        int m_SourcesCount;
        int64_t m_PlaybackStartTime;
        int64_t m_PlaybackConsumedFrame;
        NativeAudioNodeTarget *m_Output;

        static void *queueThread(void *args);
        static void *decodeThread(void *args);
        void bufferData();
        static int InitPortAudioOutput(NativeAudioParameters *params, 
                PaStream **outputStream, PaStreamCallback *callback, void *userData);
        static int GetOutputBufferSize(NativeAudioParameters *params);
        int openOutput();
        int openInput();
        NativeAudioNode *addSource(NativeAudioNode *source, bool externallyManaged);
        void removeSource(NativeAudioSource *source);
        NativeAudioNode *createNode(NativeAudio::Node node, int input, int ouput);
        bool connect(NodeLink *input, NodeLink *output);
        bool disconnect(NodeLink *input, NodeLink *output);
        void setVolume(float volume);
        float getVolume();
        static int getOutputBufferSize();
        double getLatency();
        void wakeup();
        void shutdown();
        static void sourceNeedWork(void *ptr);
        void lockSources();
        void unlockSources();
        void lockQueue();
        void unlockQueue();
        void postMessage(AudioMessageCallback cbk, void *custom);
        void postMessage(AudioMessageCallback cbk, void *custom, bool block);

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
        bool m_QueueFreeLock;
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

