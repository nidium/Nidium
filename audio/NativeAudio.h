#ifndef nativeaudio_h__
#define nativeaudio_h__

#include <portaudio.h>
#include <pthread.h>
#include "pa_ringbuffer.h"
#include "pa_converters.h"
#include "pa_dither.h"
#include "zita-resampler/resampler.h"
#include "NativeAudioNode.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#if 0
  #define SPAM(a) printf a
#else
  #define SPAM(a) (void)0
#endif

#define NATIVE_AVIO_BUFFER_SIZE 2048 
#define NATIVE_AVDECODE_BUFFER_SAMPLES 16384 
#define NATIVE_RESAMPLER_BUFFER_SAMPLES 1024


class NativeAudioTrack;

class NativeAudio
{
    public:
        NativeAudio(int bufferSize, int channels, int sampleRate);

        enum SampleFormat {
            FLOAT32 = sizeof(float), 
            INT24 = sizeof(int), 
            INT16 = sizeof(int16_t), 
            UINT8 = sizeof(uint8_t)
        };

        enum Node {
            SOURCE, GAIN, TARGET 
        };

        NativeAudioParameters *outputParameters;
        NativeAudioParameters *inputParameters;

        static void *queueThread(void *args);
        static void *decodeThread(void *args);
        void bufferData();

        int openOutput();
        int openInput();

        NativeAudioNodeTarget *output;

        NativeAudioTrack *addTrack(int out);
        NativeAudioNode *createNode(NativeAudio::Node node, int input, int ouput);
        void connect(NativeAudioNode::NodeLink *input, NativeAudioNode::NodeLink *output);

        static inline int getSampleSize(int sampleFmt);

        void shutdown();

        ~NativeAudio();
    private:
        struct NativeAudioTracks {
            NativeAudioTrack *curr;

            NativeAudioTracks *next;
            NativeAudioTracks *prev;
        };

        NativeSharedMessages *sharedMsg;

        PaStream *inputStream;
        PaStream *outputStream;

        PaUtilRingBuffer rBufferOut;
        float *rBufferOutData;
        float *nullBuffer;
        float *cbkBuffer;

        pthread_cond_t bufferNotEmpty, queueHaveData;
        pthread_mutex_t decodeLock, queueLock, shutdownLock;

        bool threadShutdown;

        NativeAudioTracks *tracks;
        int tracksCount;
        int queueCount;

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

class NativeAudioTrack;

class NativeAudioTrack : public NativeAudioNode
{
    public:
        NativeAudioTrack(int out, NativeAudioParameters *outputParameters, NativeSharedMessages *msg, pthread_cond_t *bufferNotEmpty);

        NativeAudioParameters *outputParameters;

        pthread_cond_t *bufferNotEmpty;

        PaUtilRingBuffer rBufferIn;
        PaUtilRingBuffer rBufferOut;

        bool opened;
        bool playing;
        bool paused;
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
                BufferReader(uint8_t *buffer, unsigned long bufferSize) 
                    : buffer(buffer), bufferSize(bufferSize), pos(0) {}

                static int read(void *opaque, uint8_t *buffer, int size) {
                    BufferReader *reader = (BufferReader *)opaque;

                    if (reader->pos + size > reader->bufferSize) {
                        size = reader->bufferSize - reader->pos;
                    }

                    if (size > 0) {
                        memcpy(buffer, reader->buffer + reader->pos, size);
                        reader->pos += size;
                    }

                    return size;
                }

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
        AVPacket tmpPacket;

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
