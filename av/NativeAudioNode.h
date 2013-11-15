#ifndef nativeaudionode_h__
#define nativeaudionode_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "NativeAV.h"
#include "NativeAudio.h"
#include "NativeFileIO.h"
#include <exception>
#include <SkRefCnt.h>

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
typedef void (*ArgCallback)(NativeAudioNode *node, int id, void *val, int size); // Callback for node arguments
typedef void (*NodeMessageCallback)(NativeAudioNode *node, void *custom); // Message posting to thread TODO : Normalize args

class NativeAudioNode : public SkRefCnt
{
    public :
        struct ExportsArgs {
            const char *name;
            ArgType type;
            void *ptr;
            ArgCallback cbk;
            int id;

            ExportsArgs(const char *name, ArgType type, void *ptr) : name(name), type(type), ptr(ptr) {};
            ExportsArgs(const char *name, ArgType type, int id, ArgCallback cbk) : name(name), type(type), ptr(NULL), cbk(cbk), id(id)  {};
        };

        ExportsArgs *args[NATIVE_AUDIONODE_ARGS_SIZE];

        // XXX : Normalize callbacks ?
        struct Message {
            NativeAudioNode *node;
            ExportsArgs *arg;
            unsigned long size;
            void *val;
            Message(NativeAudioNode *node, ExportsArgs *arg, void *val, unsigned long size);
            ~Message();
        };

        struct CallbackMessage {
            NodeMessageCallback cbk;
            NativeAudioNode *node;
            void *custom;
            CallbackMessage(NodeMessageCallback cbk, NativeAudioNode *node, void *custom)
                : cbk(cbk), node(node), custom(custom) {}
        };


        float **frames;

        bool nullFrames;
        bool processed;
        bool isConnected;

        NodeLink *input[32];
        NodeLink *output[32];

        int inCount;
        int outCount;

        // XXX : Should be private 
        // (but for now JS need access to it)
        NativeAudio *audio;

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
        void updateIsConnected();

        virtual bool process() = 0;
        virtual bool isActive() {
            return true;
        }

        virtual ~NativeAudioNode() = 0;
    protected:
        bool doNotProcess;
        float *newFrame();

    private:
        void post(int msg, ExportsArgs *arg, void *val, unsigned long size);
        bool isFrameOwner(float *frame)
        {
            void *tmp = (void *)*((ptrdiff_t *)&(frame[this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels]));
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

class NativeAudioNodeGain : public NativeAudioNode
{
    public :
        NativeAudioNodeGain(int inCount, int outCount, NativeAudio *audio);

        double gain;

        virtual bool process();
};

class NativeAudioNodeCustom : public NativeAudioNode
{
    public :
        NativeAudioNodeCustom(int inCount, int outCount, NativeAudio *audio);

        void setCallback(NodeCallback cbk, void *custom);

        virtual bool process();
    private : 
        NodeCallback cbk;
        void *custom;
};

class NativeAudioNodeDelay : public NativeAudioNode
{
    public :
        NativeAudioNodeDelay(int inCount, int outCount, NativeAudio *audio);
        
        enum Args {
            DELAY, WET, DRY 
        };

        double delay;
        double wet;
        double dry;
        float **buffers;

        virtual bool process();
        static void argCallback(NativeAudioNode *node, int id, void *val, int size);
    
        ~NativeAudioNodeDelay();
    private : 
        int idx;
};

class NativeAudioNodeStereoEnhancer : public NativeAudioNode
{
    public :
        NativeAudioNodeStereoEnhancer(int inCount, int outCount, NativeAudio *audio);
        
        double width;

        virtual bool process();
};

class NativeAudioNodeReverb : public NativeAudioNode
{
    public :
        NativeAudioNodeReverb(int inCount, int outCount, NativeAudio *audio);

        double delay;

        virtual bool process();
};

#if 0
class NativeAudioNodeMixer : public NativeAudioNode
{
    public :
        NativeAudioNodeMixer(int inCount, int outCount, NativeAudioParameters *params) : NativeAudioNode(inCount, outCount, params) 
        {
            printf("Mixer init\n");
            printf("count %d/%d\n", inCount, outCount);
        }

        virtual bool process() 
        {
            SPAM(("|process called on mixer\n"));

            if (this->outCount == 2) {
                for (int i = 0; i < 256; i++) {
                    float tmpL, tmpR;

                    tmpL = tmpR = 0;

                    for (int j = 0; j < this->inCount; j+=2) {
                        tmpL += this->frames[j][i];
                        tmpR += this->frames[j+1][i];
                    }
                    this->frames[0][i] = tmpL/(this->inCount/2);
                    this->frames[1][i] = tmpR/(this->inCount/2);
                }
            } else if (this->outCount == 1) {
                float tmpL, tmpR;

                tmpL = tmpR = 0;
                for (int i = 0; i < 256; i++) {
                    for (int j = 0; j < this->inCount; j++) {
                        tmpL += this->frames[j][i];
                    }
                    this->frames[0][i] = tmpL/(this->inCount);
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

        NativeAudioParameters *outputParameters;

        pthread_cond_t *bufferNotEmpty;

        PaUtilRingBuffer *rBufferOut;
        NativeAVReader *reader;

        bool externallyManaged;
        bool playing;
        bool stopped;
        bool loop;
        int nbChannel;
        bool doClose;

        void play();
        void pause();
        void stop();
        void close();
        int open(const char *chroot, const char *src);
        int open(void *buffer, int size);
        int openInit();
        static void openInitCoro(void *arg);
        int initStream();
        int initInternal();
        void seek(double time);

        void onProgress(size_t buffered, size_t len) {};

        int avail();
        bool buffer();
        void buffer(AVPacket *pkt);
        static void bufferCoro(void *arg);
        bool bufferInternal();

        virtual bool process();
        bool isActive();
        bool work();
        bool decode();
        int resample(int destSamples);
        double getClock();
        void drop(double ms);

        void closeInternal(bool reset);
        ~NativeAudioSource();
    private:
        AVCodecContext *codecCtx;

        struct TmpFrame {
            int size;
            int nbSamples;
            float *data;
        } tmpFrame;
        AVPacket *tmpPacket;
  
        static void seekCoro(void *arg);
        void seekInternal(double time);

        double clock;
        bool frameConsumed;
        bool packetConsumed;
        int samplesConsumed;
        int audioStream;
        int m_FailedDecoding;

        SwrContext *swrCtx;
        PaUtilConverter *sCvt;
        Resampler *fCvt;

        unsigned char *avioBuffer;
        float *fBufferInData, *fBufferOutData;
        void *rBufferOutData;

        bool eof;
        bool buffering;
};

class NativeAudioCustomSource : public NativeAudioNodeCustom
{
    public:
        NativeAudioCustomSource(int out, NativeAudio *audio) 
            : NativeAudioNodeCustom(0, out, audio), m_Playing(true)
        {
        }

        typedef void (*SeekCallback)(NativeAudioCustomSource *node, double ms, void *custom);

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
        bool isActive() override;
};

class NativeAudioNodeException : public std::exception
{
    public:
        NativeAudioNodeException (const char *err)
            : err(err)
        {
        }

        virtual const char *what() const throw() 
        {
            return err;
        }

        virtual ~NativeAudioNodeException() throw () {}
    private:
        const char *err;
};
#endif
