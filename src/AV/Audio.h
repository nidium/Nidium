/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef av_audio_h__
#define av_audio_h__

#include <stdint.h>
#include <pthread.h>

#include <ape_netlib.h>
#include "Core/SharedMessages.h"

#include "AV.h"

struct PaUtilRingBuffer;
struct PaStreamCallbackTimeInfo;
typedef void PaStream;
typedef unsigned long PaStreamCallbackFlags;
typedef int PaStreamCallback(const void *input,
                             void *output,
                             unsigned long frameCount,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData);

namespace Nidium {
namespace AV {

#if 0
#define SPAM(a)                                      \
    fprintf(stdout, ">%ld / ", av_gettime() / 1000); \
    printf a
#else
#define SPAM(a) (void)0
#endif

#define NIDIUM_AUDIO_CHECK_EXIT_THREAD \
    if (audio->m_ThreadShutdown) {     \
        SPAM(("Exiting\n"));           \
        return NULL;                   \
    }

class AudioSource;
class AudioNode;
class AudioNodeTarget;
class NodeLink;
typedef void (*AudioMessageCallback)(void *custom);

class Audio
{
public:
    Audio(ape_global *net,
          unsigned int bufferSize = 0,
          unsigned int channels = 2,
          unsigned int sampleRate = 44100);

    friend class Video;
    friend class AudioSource;
    friend class AudioSourceCustom;

    enum SampleFormat
    {
        FLOAT32 = sizeof(float),
        DOUBLE  = sizeof(double),
        INT32   = sizeof(int32_t),
        INT24   = sizeof(int),
        INT16   = sizeof(int16_t),
        UINT8   = sizeof(uint8_t)
    };

    enum Node
    {
        SOURCE,
        GAIN,
        TARGET,
        CUSTOM,
        CUSTOM_SOURCE,
        REVERB,
        DELAY,
        STEREO_ENHANCER
    };

    struct CallbackMessage
    {
        AudioMessageCallback m_Cbk;
        void *m_Custom;
        CallbackMessage(AudioMessageCallback cbk, void *custom)
            : m_Cbk(cbk), m_Custom(custom)
        {
        }
    };

    ape_global *m_Net;

    AudioParameters *m_OutputParameters;
    AudioParameters *m_InputParameters;
    Core::SharedMessages *m_SharedMsg;
    int m_SourcesCount;
    int64_t m_PlaybackStartTime;
    int64_t m_PlaybackConsumedFrame;
    AudioNodeTarget *m_Output;

    static void *queueThread(void *args);
    static void *decodeThread(void *args);
    void bufferData();
    static int InitPortAudioOutput(AudioParameters *params,
                                   PaStream **outputStream,
                                   PaStreamCallback *callback,
                                   void *userData);
    static int GetOutputBufferSize(AudioParameters *params);
    int openOutput();
    int openInput();
    AudioNode *addSource(AudioNode *source, bool externallyManaged);
    void removeSource(AudioSource *source);
    AudioNode *createNode(Audio::Node node, int input, int ouput);
    bool connect(NodeLink *input, NodeLink *output);
    bool disconnect(NodeLink *input, NodeLink *output);
    void setVolume(float volume);
    float getVolume();
    static int getOutputBufferSize();
    double getLatency();
    void wakeup(bool block = true);
    void shutdown();
    static void sourceNeedWork(void *ptr);
    void lockSources();
    void unlockSources();
    void lockQueue();
    void unlockQueue();
    void postMessage(AudioMessageCallback cbk, void *custom);
    void postMessage(AudioMessageCallback cbk, void *custom, bool block);

    ~Audio();

private:
    struct AudioSources
    {
        AudioNode *curr;
        bool externallyManaged;

        AudioSources *next;
        AudioSources *prev;
    };

    PaStream *m_InputStream;
    PaStream *m_OutputStream;
    float *m_rBufferOutData;
    float m_volume;
    pthread_t m_ThreadDecode;
    pthread_t m_ThreadQueue;
    NIDIUM_PTHREAD_VAR_DECL(m_QueueHaveData)
    NIDIUM_PTHREAD_VAR_DECL(m_QueueHaveSpace)
    NIDIUM_PTHREAD_VAR_DECL(m_QueueNeedData)
    NIDIUM_PTHREAD_VAR_DECL(m_QueueMessagesFlushed)
    pthread_mutex_t m_ShutdownLock, m_RecurseLock, m_SourcesLock;
    PaUtilRingBuffer *m_rBufferOut;
    bool m_HaveData, m_NotEmpty;
    bool m_SourceNeedWork;
    bool m_QueueFreeLock;
    bool m_SharedMsgFlush;
    bool m_ThreadShutdown;
    AudioSources *m_Sources;
    int m_QueueCount;

    void readMessages();
    void readMessages(bool flush);
    void processQueue();
    bool canWriteFrame();

    inline bool haveSourceActive(bool excludeExternal);
    static int paOutputCallback(const void *inputBuffer,
                                void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo *timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData);

    int paOutputCallbackMethod(const void *inputBuffer,
                               void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags);
};

} // namespace AV
} // namespace Nidium

#endif
