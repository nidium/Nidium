#ifndef nativeaudio_h__
#define nativeaudio_h__

#include <portaudio.h>
#include <pthread.h>
#include "pa_ringbuffer.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#define NATIVE_AV_BUFFER_SIZE 2048 

class NativeAudioTrack;

class NativeAudio
{
    public:
        NativeAudio();

        enum SampleFormat {FLOAT32 = 4, INT24 = 3, INT16 = 2, UINT8 = 1};

        struct OutputParameters {
            int bufferSize, channel, sampleFmt, sampleRate;
        };

        struct InputParameters {
            int bufferSize, channel, sampleRate;
            SampleFormat sampleFmt;
        };

        OutputParameters outputParameters;
        InputParameters inputParameters;

        static void *decodeThread(void *args);
        void bufferData();

        int openOutput(int bufferSize, int channel, SampleFormat sampleFmt, int sampleRate);
        int openInput(int bufferSize, int channel, SampleFormat sampleFmt, int sampleRate);

        NativeAudioTrack *addTrack();

        void link(int nb, ...);

        ~NativeAudio();
    private:
        struct NativeAudioTracks {
            NativeAudioTrack *curr;

            NativeAudioTracks *next;
            NativeAudioTracks *prev;
        };

        PaStream *inputStream;
        PaStream *outputStream;

        pthread_cond_t bufferNotEmpty;
        pthread_mutex_t decodeLock;

        int *filterList;
        NativeAudioTracks *tracks;
        int tracksCount;

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

class NativeAudioTrack
{
    public:
        NativeAudioTrack(NativeAudio::OutputParameters *outputParameters);

        NativeAudio::OutputParameters *outputParameters;

        PaUtilRingBuffer rBufferIn;
        PaUtilRingBuffer rBufferOut;

        void *rBufferOutData;
        void *rBufferInData;
        unsigned char *avioBuffer;

	    AVFormatContext *container;
        AVFrame *tmpFrame;
        bool frameConsumed;
        int audioStream;

        bool opened;
        bool playing;
        bool paused;

        int init();
        int open(void *buffer, int size);
        void play();
        void pause();
        void stop();

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

        BufferReader *br;

        void close(bool reset);
};

#endif
