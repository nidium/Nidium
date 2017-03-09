/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef av_audionode_h__
#define av_audionode_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <exception>

#include "AV.h"
#include "Audio.h"

#define NIDIUM_AUDIO_NODE_ARGS_SIZE 32
#define NIDIUM_AUDIO_NODE_WIRE_SIZE 256
#define NIDIUM_AUDIO_NODE_CHANNEL_SIZE 32

struct AVFormatContext;
struct AVCodecContext;
struct AVDictionary;
struct AVPacket;

class Resampler;

struct SwrContext;

namespace Nidium {
namespace AV {

typedef void PaUtilConverter(void *,
                             int,
                             void *,
                             int,
                             unsigned int,
                             struct PaUtilTriangularDitherGenerator *);

class AudioNode;

enum ArgType
{
    INT,
    DOUBLE
};

// {{{ NodeIO
struct NodeIO
{
    AudioNode *m_Node;
    bool m_Feedback;
    float *m_Frame;
    NodeIO(AudioNode *node, float *frame)
        : m_Node(node), m_Feedback(false), m_Frame(frame){};
};
// }}}

// {{{ Event
struct NodeEvent
{
    float **data;
    unsigned long size;
    void *custom;
};
// }}}

// {{{ NodeLink
class NodeLink
{
public:
    enum Type
    {
        kNodeLink_Input,
        kNodeLink_Output
    };

    int m_Count;
    int m_Channel;
    bool m_HaveFeedback;
    AudioNode *m_Node;
    NodeIO *wire[NIDIUM_AUDIO_NODE_WIRE_SIZE];

    NodeLink(Type type, int channel, AudioNode *node)
        : m_Count(0), m_Channel(channel), m_HaveFeedback(false), m_Node(node),
          m_Type(type)
    {
        for (int i = 0; i < NIDIUM_AUDIO_NODE_WIRE_SIZE; i++) {
            wire[i] = NULL;
        }
    };

    bool isInput()
    {
        return m_Type == kNodeLink_Input;
    }

    bool isOutput()
    {
        return m_Type == kNodeLink_Output;
    }

private:
    Type m_Type;
};
// }}}

// TODO : Cleanup callbacks
typedef void (*NodeCallback)(
    const struct NodeEvent *ev); // Simple on thread callback
typedef void (*ArgCallback)(AudioNode *m_Node,
                            int m_Id,
                            void *m_Val,
                            int m_Size); // Callback for node arguments
typedef void (*NodeMessageCallback)(
    AudioNode *m_Node,
    void *m_Custom); // Message posting to thread TODO : Normalize m_Args

// {{{ AudioNode
class AudioNode
{
public:
    struct ExportsArgs
    {
        const char *m_Name;
        ArgType m_Type;
        void *m_Ptr;
        ArgCallback m_Cbk;
        int m_Id;

        ExportsArgs(const char *name, ArgType type, void *ptr)
            : m_Name(name), m_Type(type), m_Ptr(ptr), m_Cbk(NULL), m_Id(0){};
        ExportsArgs(const char *name, ArgType type, int id, ArgCallback cbk)
            : m_Name(name), m_Type(type), m_Ptr(NULL), m_Cbk(cbk), m_Id(id){};
    };

    ExportsArgs *m_Args[NIDIUM_AUDIO_NODE_ARGS_SIZE];

    // XXX : Normalize callbacks ?
    struct Message
    {
        AudioNode *m_Node;
        ExportsArgs *m_Arg;
        unsigned long m_Size;
        void *m_Val;
        Message(AudioNode *node,
                ExportsArgs *arg,
                void *val,
                unsigned long size);
        ~Message();
    };

    struct CallbackMessage
    {
        NodeMessageCallback m_Cbk;
        AudioNode *m_Node;
        void *m_Custom;
        CallbackMessage(NodeMessageCallback cbk, AudioNode *node, void *custom)
            : m_Cbk(cbk), m_Node(node), m_Custom(custom)
        {
        }
    };
    float **m_Frames;
    // true if all the m_Frames are zeroed
    bool m_NullFrames;
    bool m_Processed;
    // true if the node is connected to a source and a target
    bool m_IsConnected;
    NodeLink *m_Input[32];
    NodeLink *m_Output[32];
    int m_InCount;
    int m_OutCount;
    // XXX : Should be private
    // (but for now JS need access to it)
    Audio *m_Audio;

    AudioNode(int inCount, int outCount, Audio *audio);
    void resetFrames();
    void resetFrame(int channel);
    void callback(NodeMessageCallback cbk, void *custom);
    void callback(NodeMessageCallback cbk, void *custom, bool block);
    bool set(const char *name, ArgType type, void *value, unsigned long size);
    void get(const char *name);
    void updateFeedback(AudioNode *nOut);
    void updateWiresFrame(int channel, float *frame);
    void updateWiresFrame(int channel, float *frame, float *discardFrame);
    bool queue(NodeLink *in, NodeLink *out);
    bool unqueue(NodeLink *in, NodeLink *out);
    bool recurseGetData(int *sourceFailed);
    void processQueue();
    bool updateIsConnectedInput();
    bool updateIsConnectedOutput();
    bool updateIsConnected();
    bool updateIsConnected(bool input, bool output);
    virtual bool process() = 0;
    virtual bool isActive()
    {
        return true;
    }
    virtual ~AudioNode() = 0;

protected:
    bool m_DoNotProcess;
    float *newFrame();

private:
    void post(int msg, ExportsArgs *arg, void *val, unsigned long size);
    bool isFrameOwner(float *frame)
    {
        void *tmp = (void *)*(
            (ptrdiff_t *)&(frame[m_Audio->m_OutputParameters->m_BufferSize
                                 / m_Audio->m_OutputParameters->m_Channels]));
        return tmp == static_cast<void *>(this);
    }
    NodeIO **getWire(NodeLink *link)
    {
        for (int i = 0; i < NIDIUM_AUDIO_NODE_WIRE_SIZE; i++) {
            if (link->wire[i] == NULL) {
                return &link->wire[i];
            }
        }
        return NULL;
    }
};
// }}}

// {{{ AudioNodeTarget
class AudioNodeTarget : public AudioNode
{
public:
    AudioNodeTarget(int inCount, int outCount, Audio *audio);

    virtual bool process();
};
// }}}

// {{{ AudioNodeCustom
class AudioNodeCustom : public AudioNode
{
public:
    AudioNodeCustom(int inCount, int outCount, Audio *audio);

    void setProcessor(NodeCallback cbk, void *custom);

    virtual bool process() override;
private:
    NodeCallback m_Cbk = nullptr;
    void *m_Custom = nullptr;
};
// }}}

// {{{ AudioNodeStereoEnhance
class AudioNodeStereoEnhancer : public AudioNode
{
public:
    AudioNodeStereoEnhancer(int inCount, int outCount, Audio *audio);

    double m_Width;

    virtual bool process();
};
// }}}

// {{{ AudioNodeReverb
class AudioNodeReverb : public AudioNode
{
public:
    AudioNodeReverb(int inCount, int outCount, Audio *audio);

    double m_Delay;

    virtual bool process();
};
// }}}

#if 0
// {{{ AudioNodeMixer
class AudioNodeMixer : public AudioNode
{
    public :
        AudioNodeMixer(int m_InCount, int m_OutCount, AudioParameters *params) :
          AudioNode(m_InCount, m_OutCount, params)
        {
            ndm_log(NDM_LOG_DEBUG, "AudioNode", "Mixer init");
            ndm_logf(NDM_LOG_DEBUG, "AudioNode", "count %d/%d", m_InCount, m_OutCount);
        }

        virtual bool process()
        {
            SPAM(("[AudioNode] process called on mixer"));

            if (m_OutCount == 2) {
                for (int i = 0; i < 256; i++) {
                    float tmpL, tmpR;

                    tmpL = tmpR = 0;

                    for (int j = 0; j < m_InCount; j += 2) {
                        tmpL += m_Frames[j][i];
                        tmpR += m_Frames[j+1][i];
                    }
                    m_Frames[0][i] = tmpL/(m_InCount/2);
                    m_Frames[1][i] = tmpR/(m_InCount/2);
                }
            } else if (m_OutCount == 1) {
                float tmpL, tmpR;

                tmpL = tmpR = 0;
                for (int i = 0; i < 256; i++) {
                    for (int j = 0; j < m_InCount; j++) {
                        tmpL += m_Frames[j][i];
                    }
                    m_Frames[0][i] = tmpL/(m_InCount);
                }
            }
            return true;
        }
};
// }}}
#endif

// {{{ AudioSource
class AudioSource : public AudioNode, public AVSource
{
public:
    AudioSource(int out, Audio *audio, bool external);

    friend class Video;
    friend class VideoAudioSource;

    AudioParameters *m_OutputParameters;
    pthread_cond_t *m_BufferNotEmpty;
    PaUtilRingBuffer *m_rBufferOut;
    AVReader *m_Reader;
    bool m_ExternallyManaged;
    bool m_Playing;
    bool m_PlayWhenReady;
    bool m_Stopped;
    bool m_Loop;
    int m_NbChannel;

    void play() override;
    void pause() override;
    void stop() override;
    void close() override;
    int open(const char *src) override;
    int open(void *buffer, int size) override;
    int openInit() override;
    static void openInitCoro(void *arg);
    int initStream();
    int initInternal();
    void seek(double time) override;

    int avail();
    virtual bool buffer();
    void buffer(AVPacket *pkt);
    static void bufferCoro(void *arg);
    bool bufferInternal();

    virtual bool process() override;
    bool isActive() override;
    bool work();
    bool decode();
    int resample(int destSamples);
    double getClock() override;
    double drop(double ms);
    void closeInternal(bool reset);

    virtual ~AudioSource();
private:
    AVCodecContext *m_CodecCtx;

    struct TmpFrame
    {
        int size;
        int nbSamples;
        float *data;
    } m_TmpFrame;
    AVPacket *m_TmpPacket;

    static void seekCoro(void *arg);
    void seekInternal(double time);

    double m_Clock;
    bool m_FrameConsumed;
    bool m_PacketConsumed;
    int m_SamplesConsumed;
    int m_AudioStream;
    int m_FailedDecoding;

    SwrContext *m_SwrCtx;
    PaUtilConverter *m_sCvt;
    Resampler *m_fCvt;

    unsigned char *m_AvioBuffer;
    float *m_fBufferInData, *m_fBufferOutData;
    void *m_rBufferOutData;

    bool m_Buffering;
};
// }}}

// {{{ AudioSourceCustom
class AudioSourceCustom : public AudioNodeCustom,
                          public AVSourceEventInterface
{
public:
    AudioSourceCustom(int out, Audio *audio)
        : AudioNodeCustom(0, out, audio), m_Playing(false),
          m_SeekCallback(NULL), m_Custom(NULL), m_SeekTime(0.0f)
    {
    }

    typedef void (*SeekCallback)(AudioSourceCustom *m_Node,
                                 double ms,
                                 void *m_Custom);

    bool m_Playing;
    SeekCallback m_SeekCallback;
    void *m_Custom;
    double m_SeekTime;

    void setSeek(SeekCallback cbk, void *custom);

    void play();
    void pause();
    void stop();
    void seek(double pos);
    static void seekMethod(AudioNode *node, void *custom);

    bool process() override;
    bool isActive() override;
};
// }}}

// {{{ AudioProcessor
class AudioProcessor
{
public:
    virtual void process(float *in, int *i) = 0;
    virtual ~AudioProcessor() = 0;
};
// }}}

// {{{ AudioNodeProcessor
class AudioNodeProcessor : public AudioNode
{
public:
    AudioNodeProcessor(int inCount, int outCount, Audio *audio)
        : AudioNode(inCount, outCount, audio)
    {
        for (int i = 0; i < NIDIUM_AUDIO_NODE_CHANNEL_SIZE; i++) {
            for (int j = 0; j < NIDIUM_AUDIO_NODE_CHANNEL_SIZE; j++) {
                m_Processor[i][j] = NULL;
            }
        }
    };

    void setProcessor(int channel, ...)
    {
        va_list args;
        va_start(args, channel);
        AudioProcessor *p = va_arg(args, AudioProcessor *);

        while (p != NULL) {
            this->setProcessor(channel, p);
            p = va_arg(args, AudioProcessor *);
        }

        va_end(args);
    }

    void setProcessor(int channel, AudioProcessor *processor)
    {
        for (int i = 0; i < NIDIUM_AUDIO_NODE_CHANNEL_SIZE; i++) {
            if (m_Processor[channel][i] == NULL) {
                m_Processor[channel][i] = processor;
                break;
            }
        }
    }

    bool process()
    {
        for (int i = 0; i < m_Audio->m_OutputParameters->m_FramesPerBuffer;
             i++) {
            for (int j = 0; j < m_InCount; j++) {
                for (int k = 0; k < NIDIUM_AUDIO_NODE_CHANNEL_SIZE; k++) {
                    AudioProcessor *p = m_Processor[m_Input[j]->m_Channel][k];
                    if (p != NULL) {
                        p->process(&m_Frames[m_Input[j]->m_Channel][i], &i);
                    } else {
                        break;
                    }
                }
            }
        }
        return true;
    };

    ~AudioNodeProcessor()
    {
    }

private:
    AudioProcessor *m_Processor[NIDIUM_AUDIO_NODE_CHANNEL_SIZE]
                               [NIDIUM_AUDIO_NODE_CHANNEL_SIZE];
};
// }}}

// {{{ AudioNodeException
class AudioNodeException : public std::exception
{
public:
    AudioNodeException(const char *err) : m_Err(err)
    {
    }

    virtual const char *what() const throw()
    {
        return m_Err;
    }

    virtual ~AudioNodeException() throw()
    {
    }

private:
    const char *m_Err;
};
// }}}

} // namespace AV
} // namespace Nidium

#endif
