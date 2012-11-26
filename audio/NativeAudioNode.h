#ifndef nativeaudionode_h__
#define nativeaudionode_h__

#include "NativeAudio.h"
#include <stdio.h>

#define NATIVE_AUDIONODE_ARGS_SIZE 32
#define NATIVE_AUDIONODE_CHANNEL_SIZE 32
#define NATIVE_AUDIONODE_WIRE_SIZE 32

struct AVFormatContext;
struct AVPacket;
struct AVCodecContext;
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
    NodeIO *wire[32];
    TypeIO type;

    NodeLink (TypeIO type, int channel, NativeAudioNode *node) : 
        count(0), channel(channel), haveFeedback(false), node(node), type(type) 
    {
        for (int i = 0; i < 32; i++) {
            wire[i] = NULL;
        }
    };
};

// XXX : Normalize arguments
typedef void (*NodeCallback)(const struct NodeEvent *ev);
typedef void (*NodeMessageCallback)(NativeAudioNode *node, void *custom);

class NativeAudioNode
{
    public :
        struct ExportsArgs {
            const char *name;
            ArgType type;
            void *ptr;

            ExportsArgs(const char *name, ArgType type, void *ptr) : name(name), type(type), ptr(ptr) {};
        };

        ExportsArgs *args[NATIVE_AUDIONODE_ARGS_SIZE];

        struct Message {
            NativeAudioNode *node;
            void *source, *dest;
            unsigned long size;
            Message(NativeAudioNode *node, void *source, void *dest, unsigned long size);
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

        int nodeProcessed;
        int totalProcess;
        NodeLink *input[32];
        NodeLink *output[32];

        int inQueueCount;

        int inCount;
        int outCount;

        // XXX : Should be private 
        // (but for now JS need access to it)
        NativeAudio *audio;

        NativeAudioNode(int inCount, int outCount, NativeAudio *audio);

        void callback(NodeMessageCallback cbk, void *custom);
        bool set(const char *name, ArgType type, void *value, unsigned long size);
        void get(const char *name);

        void updateFeedback(NativeAudioNode *nOut);
        void updateWiresFrame(int channel, float *frame);

        void queue(NodeLink *in, NodeLink *out);
        void unqueue(NodeLink *in, NodeLink *out);

        bool recurseGetData();

        virtual bool process() = 0;

        ~NativeAudioNode();

    private:
        void post(int msg, void *source, void *dest, unsigned long size);
};

class NativeAudioNodeTarget : public NativeAudioNode
{
    public :
        NativeAudioNodeTarget(int inCount, int outCount, NativeAudio *audio) 
            : NativeAudioNode(inCount, outCount, audio)
        { 
            SPAM(("Target init\n"));

        }

        virtual bool process()
        {
            SPAM(("process called on target\n"));
            return true;
        }
};

#if 0
class NativeAudioNodeWirdo : public NativeAudioNode
{
    public :
        NativeAudioNodeWirdo (int inCount, int outCount, NativeAudioParameters *params) : NativeAudioNode(inCount, outCount, params) 
        {
            printf("Wirdo init\n");
            printf("count %d/%d\n", inCount, outCount);
            /*
            for (int i = 0; i < inCount; i++) {
                this->inQueue[j]
            }
            */
        }

        float gain;

        virtual bool process() 
        {
            SPAM(("|process called on wirdo\n"));
            for (int i = 0; i < 256; i++) {
                this->frames[2][i] = this->frames[0][i];
                this->frames[3][i] = this->frames[1][i];
            }
            return true;
        }
};
#endif

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

class NativeAudioTrack : public NativeAudioNode
{
    public:
        NativeAudioTrack(int out, NativeAudio *audio);

        NativeAudioParameters *outputParameters;

        pthread_cond_t *bufferNotEmpty;

        PaUtilRingBuffer *rBufferIn;
        PaUtilRingBuffer *rBufferOut;

        bool opened;
        bool playing;
        bool stopped;
        bool repeat;
        int nbChannel;

        int open(void *buffer, int size);
        void play();
        void pause();
        void stop();

        int avail();
        int buffer();
        int buffer(int n);

        virtual bool process();
        bool work();
        bool decode();
        int resample(float *dest, int destSamples);
        bool getFrame();

        ~NativeAudioTrack();

    private:
        class BufferReader
        {
            public:
                BufferReader(uint8_t *buffer, unsigned long bufferSize);

                static int read(void *opaque, uint8_t *buffer, int size);
                static int64_t seek(void *opaque, int64_t offset, int whence);

                ~BufferReader() {};
            private:
                uint8_t *buffer;
                unsigned long bufferSize;
                unsigned long pos;
        };

	    AVFormatContext *container;
        AVCodecContext *avctx;
        BufferReader *br;

        struct TmpFrame {
            int size;
            int nbSamples;
            float *data;
        } tmpFrame;
        AVPacket *tmpPacket;

        bool frameConsumed;
        bool packetConsumed;
        int samplesConsumed;
        int audioStream;

        PaUtilConverter *sCvt;
        Resampler *fCvt;

        unsigned char *avioBuffer;
        float *fBufferInData, *fBufferOutData;
        void *rBufferOutData, *rBufferInData;

        bool eof;

        void close(bool reset);
};
#endif
