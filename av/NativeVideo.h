#ifndef nativevideo_h__
#define nativevideo_h__

#include <pthread.h>
#include <stdint.h>
#include <native_netlib.h>
#include "NativeAudio.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#define NATIVE_VIDEO_BUFFER_SAMPLES 8
#define NATIVE_VIDEO_SYNC_THRESHOLD 0.01 
#define NATIVE_VIDEO_NOSYNC_THRESHOLD 10.0

typedef void (*VideoCallback)(uint8_t *data, void *custom);
struct PaUtilRingBuffer;

struct SwsContext;
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;

class NativeVideo : public NativeAVSource
{
    public :
        NativeVideo(ape_global *n);

        struct TimerItem {
            int id;
            int delay;
            TimerItem() : id(-1), delay(-1) {};
        };

        struct Packet {
            AVPacket curr;
            Packet *prev;
            Packet *next;
            
            Packet() 
                : prev(NULL), next(NULL) {}
        };

        struct PacketQueue {
            Packet *head;
            Packet *tail;
            int count;
            PacketQueue() : head(NULL), tail(NULL), count(0) {}
        };

        struct Frame {
            uint8_t *data;
            double pts;
            Frame() : data(NULL), pts(0) {};
        };

        TimerItem *timers[NATIVE_VIDEO_BUFFER_SAMPLES];
        PacketQueue *audioQueue;
        PacketQueue *videoQueue;
        Packet *freePacket;
        int timerIdx;
        int lastTimer;
        int timersDelay;

        ape_global *net;
        NativeAudioTrack *track;

        VideoCallback frameCbk;
        void *frameCbkArg;

        bool shutdown;
        bool eof;

        uint8_t *tmpFrame;
        uint8_t *frameBuffer;
        int frameSize;
        double lastPts;
        double videoClock;
        double audioClock;
        double frameTimer;
        double lastDelay;

        bool playing;
        bool stoped;
        bool doSeek;
        double doSeekTime;

        int width;
        int height;

        SwsContext *swsCtx;
        AVCodecContext *codecCtx;
        int videoStream;
        int audioStream;

        PaUtilRingBuffer *rBuff;
        uint8_t *buff;
        unsigned char *avioBuffer;
        AVFrame *decodedFrame; 
        AVFrame *convertedFrame;

        pthread_t threadDecode;
        pthread_mutex_t bufferLock;
        pthread_cond_t bufferCond;

        void play();
        void pause();
        void stop();
        int open(const char *src);
        int open(void *buffer, int size);
        int openInit();
        int openInitInternal();
        static void openInitCoro(void *arg);
        double getClock();
        void seek(double time);

        void frameCallback(VideoCallback cbk, void *arg);

        NativeAudioTrack *getAudioNode(NativeAudio *audio);
        static void* decode(void *args);
        static int display(void *custom);
        void stopAudio();

        ~NativeVideo();
    private :
        NativeAVReader *reader;
        NativeAudio *audio;
        int error;
        bool buffering;
        bool seeking;
        pthread_mutex_t audioLock;

        void close(bool reset);
        static void seekCoro(void *arg);
        void seekInternal(double time);

        void buffer();
        static void bufferCoro(void *arg);
        void bufferInternal();

        bool processAudio();
        bool processVideo();

        double syncVideo(double pts);
        void scheduleDisplay(int delay);
        void scheduleDisplay(int delay, bool force);
        int addTimer(int delay);
        bool addPacket(PacketQueue *queue, AVPacket *pkt);
        Packet *getPacket(PacketQueue *queue);
        void clearTimers(bool reset);
        void clearAudioQueue();
        void clearVideoQueue();
        void flushBuffers();

        static int getBuffer(struct AVCodecContext *c, AVFrame *pic);
        static void releaseBuffer(struct AVCodecContext *c, AVFrame *pic);

};

#endif
