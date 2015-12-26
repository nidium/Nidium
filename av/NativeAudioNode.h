#ifndef nativeaudionode_h__
#define nativeaudionode_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <exception>

#include "NativeAV.h"
#include "NativeAudio.h"

#define NATIVE_AUDIONODE_ARGS_SIZE      32
#define NATIVE_AUDIONODE_WIRE_SIZE      256
#define NATIVE_AUDIONODE_CHANNEL_SIZE   32

struct AVFormatContext;
struct AVPacket;
struct AVDictionary;
struct AVCodecContext;
struct SwrContext;
typedef void PaUtilConverter(void*, int, void*, int, unsigned int, struct PaUtilTriangularDitherGenerator*);

class Resampler;
class NativeAudioNode;

enum TypeIO {INPUT, OUTPUT};
enum ArgType {INT, DOUBLE};

struct NodeIO {
    NativeAudioNode *node;
    bool feedback;
    float *frame;
    NodeIO(NativeAudioNode *node, float *frame) : node(node), feedback(false), frame(frame) {};
};

struct NodeEvent {
    float **data;
    unsigned long size;
    void *custom;
};

struct NodeLink {
    int count;
    int channel;
    bool haveFeedback;
    NativeAudioNode *node;
    NodeIO *wire[NATIVE_AUDIONODE_WIRE_SIZE];
    TypeIO type;

    NodeLink (TypeIO type, int channel, NativeAudioNode *node) :
        count(0), channel(channel), haveFeedback(false), node(node), type(type)
    {
        for (int i = 0; i < NATIVE_AUDIONODE_WIRE_SIZE; i++) {
            wire[i] = NULL;
        }
    };
};

// TODO : Cleanup callbacks
typedef void (*NodeCallback)(const struct NodeEvent *ev); // Simple on thread callback
typedef void (*ArgCallback)(NativeAudioNode *m_Node, int m_Id, void *m_Val, int m_Size); // Callback for node arguments
typedef void (*NodeMessageCallback)(NativeAudioNode *m_Node, void *m_Custom); // Message posting to thread TODO : Normalize m_Args

class NativeAudioNode
{
    public :
        struct ExportsArgs {
            const char *m_Name;
            ArgType m_Type;
            void * m_Ptr;
            ArgCallback m_Cbk;
            int m_Id;

            ExportsArgs(const char *name, ArgType type, void *ptr) : m_Name(name), m_Type(type), m_Ptr(ptr) {};
            ExportsArgs(const char *name, ArgType type, int id, ArgCallback cbk) :
               m_Name(name), m_Type(type), m_Ptr(NULL), m_Cbk(cbk), m_Id(id)  {};
        };

        ExportsArgs *m_Args[NATIVE_AUDIONODE_ARGS_SIZE];

        // XXX : Normalize callbacks ?
        struct Message {
            NativeAudioNode *m_Node;
            ExportsArgs *m_Arg;
            unsigned long m_Size;
            void *m_Val;
            Message(NativeAudioNode *node, ExportsArgs *arg, void *val, unsigned long size);
            ~Message();
        };

        struct CallbackMessage {
            NodeMessageCallback m_Cbk;
            NativeAudioNode *m_Node;
            void *m_Custom;
            CallbackMessage(NodeMessageCallback cbk, NativeAudioNode *node, void *custom)
                : m_Cbk(cbk), m_Node(node), m_Custom(custom) {}
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
        NativeAudio *m_Audio;

        NativeAudioNode(int inCount, int outCount, NativeAudio *audio);
        void resetFrames();
        void resetFrame(int channel);
        void callback(NodeMessageCallback cbk, void *custom);
        bool set(const char *name, ArgType type, void *value, unsigned long size);
        void get(const char *name);
        void updateFeedback(NativeAudioNode *nOut);
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
        virtual bool isActive() {
            return true;
        }
        virtual ~NativeAudioNode() = 0;
    protected:
        bool m_DoNotProcess;
        float *newFrame();

    private:
        void post(int msg, ExportsArgs *arg, void *val, unsigned long size);
        bool isFrameOwner(float *frame)
        {
            void *tmp = (void *)*((ptrdiff_t *)&(frame[m_Audio->m_OutputParameters->m_BufferSize
                / m_Audio->m_OutputParameters->m_Channels]));
            return tmp == (void*)this;
        }
        NodeIO **getWire(NodeLink *link)
        {
            for (int i = 0; i < NATIVE_AUDIONODE_WIRE_SIZE; i++) {
                if (link->wire[i] == NULL) {
                    return &link->wire[i];
                }
            }
            return NULL;
        }
};

class NativeAudioNodeTarget : public NativeAudioNode
{
    public :
        NativeAudioNodeTarget(int inCount, int outCount, NativeAudio *audio);

        virtual bool process();
};

class NativeAudioNodeCustom : public NativeAudioNode
{
    public :
        NativeAudioNodeCustom(int inCount, int outCount, NativeAudio *audio);

        void setCallback(NodeCallback cbk, void *custom);

        virtual bool process();
    private :
        NodeCallback m_Cbk;
        void * m_Custom;
};

class NativeAudioNodeStereoEnhancer : public NativeAudioNode
{
    public :
        NativeAudioNodeStereoEnhancer(int inCount, int outCount, NativeAudio *audio);

        double m_Width;

        virtual bool process();
};

class NativeAudioNodeReverb : public NativeAudioNode
{
    public :
        NativeAudioNodeReverb(int inCount, int outCount, NativeAudio *audio);

        double m_Delay;

        virtual bool process();
};

#if 0
class NativeAudioNodeMixer : public NativeAudioNode
{
    public :
        NativeAudioNodeMixer(int m_InCount, int m_OutCount, NativeAudioParameters *params) : NativeAudioNode(m_InCount, m_OutCount, params)
        {
            printf("Mixer init\n");
            printf("count %d/%d\n", m_InCount, m_OutCount);
        }

        virtual bool process()
        {
            SPAM(("|process called on mixer\n"));

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
#endif

class NativeAudioSource: public NativeAudioNode, public NativeAVSource
{
    public:
        NativeAudioSource(int out, NativeAudio *audio, bool external);

        friend class NativeVideo;
        friend class NativeVideoAudioSource;

        NativeAudioParameters *m_OutputParameters;
        pthread_cond_t *m_BufferNotEmpty;
        PaUtilRingBuffer *m_rBufferOut;
        NativeAVReader *m_Reader;
        bool m_ExternallyManaged;
        bool m_Playing;
        bool m_Stopped;
        bool m_Loop;
        int m_NbChannel;

        void play();
        void pause();
        void stop();
        void close();
        int open(const char *src);
        int open(void *buffer, int size);
        int openInit();
        static void openInitCoro(void *arg);
        int initStream();
        int initInternal();
        void seek(double time);

        int avail();
        virtual bool buffer();
        void buffer(AVPacket *pkt);
        static void bufferCoro(void *arg);
        bool bufferInternal();

        virtual bool process();
        bool isActive();
        bool work();
        bool decode();
        int resample(int destSamples);
        double getClock();
        double drop(double ms);
        void closeInternal(bool reset);

        virtual ~NativeAudioSource();
    private:
        AVCodecContext *m_CodecCtx;

        struct TmpFrame {
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

class NativeAudioCustomSource : public NativeAudioNodeCustom
{
    public:
        NativeAudioCustomSource(int out, NativeAudio *audio)
            : NativeAudioNodeCustom(0, out, audio), m_Playing(false),
              m_SeekCallback(NULL)
        {
        }

        typedef void (*SeekCallback)(NativeAudioCustomSource *m_Node, double ms, void *m_Custom);

        bool m_Playing;
        SeekCallback m_SeekCallback;
        void *m_Custom;
        double m_SeekTime;

        void setSeek(SeekCallback cbk, void *custom);

        void play();
        void pause();
        void stop();
        void seek(double pos);
        static void seekMethod(NativeAudioNode *node, void *custom);

        bool process();
        bool isActive();
};

class NativeAudioProcessor
{
  public:
    virtual void process(float *in, int *i) = 0;
    virtual ~NativeAudioProcessor() = 0;
};

class NativeAudioNodeProcessor: public NativeAudioNode
{
  public:
    NativeAudioNodeProcessor(int inCount, int outCount, NativeAudio *audio)
        : NativeAudioNode(inCount, outCount, audio)
    {
        for (int i = 0; i < NATIVE_AUDIONODE_CHANNEL_SIZE; i++) {
            for (int j = 0; j < NATIVE_AUDIONODE_CHANNEL_SIZE; j++) {
                m_Processor[i][j] = NULL;
            }
        }
    };

    void setProcessor(int channel, ...)
    {
        va_list args;
        va_start(args, channel);
        NativeAudioProcessor *p = va_arg(args, NativeAudioProcessor*);

        while (p != NULL) {
            this->setProcessor(channel, p);
            p = va_arg(args, NativeAudioProcessor*);
        }

        va_end(args);
    }

    void setProcessor(int channel, NativeAudioProcessor *processor) {
        for (int i = 0; i < NATIVE_AUDIONODE_CHANNEL_SIZE; i++) {
            if (m_Processor[channel][i] == NULL) {
                printf("Adding processor %d/%d %p\n", channel, i, processor);
                m_Processor[channel][i] = processor;
                break;
            }
        }
    }

    bool process()
    {
        for (int i = 0; i < m_Audio->m_OutputParameters->m_FramesPerBuffer; i++) {
            for (int j = 0; j < m_InCount; j++) {
                for (int k = 0; k < NATIVE_AUDIONODE_CHANNEL_SIZE; k++) {
                    NativeAudioProcessor *p = m_Processor[m_Input[j]->channel][k];
                    if (p != NULL) {
                        p->process(&m_Frames[m_Input[j]->channel][i], &i);
                    } else {
                        break;
                    }
                }
            }
        }
        return true;
    };

    ~NativeAudioNodeProcessor() {}
  private:
    NativeAudioProcessor *m_Processor[NATIVE_AUDIONODE_CHANNEL_SIZE][NATIVE_AUDIONODE_CHANNEL_SIZE];
};

class NativeAudioNodeException : public std::exception
{
    public:
        NativeAudioNodeException (const char *err)
            : m_Err(err)
        {
        }

        virtual const char *what() const throw()
        {
            return m_Err;
        }

        virtual ~NativeAudioNodeException() throw () {}
    private:
        const char *m_Err;
};

#endif
