#ifndef nativevideo_h__
#define nativevideo_h__

#include <pthread.h>
#include <stdint.h>
#include <native_netlib.h>
#include "NativeAudio.h"

#define NATIVE_VIDEO_BUFFER_SAMPLES 16
#define NATIVE_VIDEO_SYNC_THRESHOLD 0.01 
#define NATIVE_VIDEO_NOSYNC_THRESHOLD 10.0

typedef void (*VideoCallback)(int width, int height, uint8_t *data, void *custom);
struct PaUtilRingBuffer;

struct SwsContext;
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacketList;

class NativeVideo
{
    public :
        NativeVideo(NativeAudio *audio, ape_global *n);

        struct TimerItem {
            int id;
            int delay;
            TimerItem() : id(-1), delay(-1) {};
        };

        struct AudioQueue {
            AVPacketList *head, *tail;
            int count;
            int size;
            AudioQueue() : head(NULL), tail(NULL), count(0), size(0) {};
        };

        struct Frame {
            uint8_t *data;
            double pts;
            Frame() : data(NULL), pts(0) {};
        };

        TimerItem *timers[NATIVE_VIDEO_BUFFER_SAMPLES];
        AudioQueue *audioQueue;
        int timerIdx;
        int lastTimer;
        int timersDelay;

        ape_global *net;
        NativeAudio *audio;
        NativeAudioTrack *track;
        VideoCallback cbk;
        void *cbkArg;

        bool shutdown;
        bool eof;

        uint8_t *tmpFrame;
        uint8_t *frameBuffer;
        int frameSize;
        double lastPts;
        static uint64_t pktPts;
        double videoClock;
        double audioClock;
        double frameTimer;
        double lastDelay;

        bool playing;
        bool stoped;

        SwsContext *swsCtx;
	    AVFormatContext *container;
        AVCodecContext *codecCtx;
        int videoStream;

        PaUtilRingBuffer *rBuff;
        uint8_t *buff;
        unsigned char *avioBuffer;
        AVFrame *decodedFrame; 
        AVFrame *convertedFrame;

        pthread_t threadDecode;
        pthread_mutex_t buffLock;
        pthread_cond_t buffNotEmpty;

        int open(void *buffer, int size);
        void setCallback(VideoCallback cbk, void *arg);
        void play();
        void pause();
        NativeAudioTrack *getAudio();
        static void* decode(void *args);
        static int display(void *custom);

        ~NativeVideo();
    private :
        NativeAVBufferReader *br;

        double syncVideo(double pts);
        double getClock();
        void scheduleDisplay(int delay);
        int addTimer(int delay);

        static int getBuffer(struct AVCodecContext *c, AVFrame *pic);
        static void releaseBuffer(struct AVCodecContext *c, AVFrame *pic);

};

#endif
