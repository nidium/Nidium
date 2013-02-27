#ifndef nativevideo_h__
#define nativevideo_h__

#include <pthread.h>
#include <stdint.h>
#include <native_netlib.h>
#include "NativeAudio.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#define NATIVE_VIDEO_BUFFER_SAMPLES 16
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
        NativeVideo(NativeAudio *audio, ape_global *n);

        struct TimerItem {
            int id;
            int delay;
            int64_t time;
            TimerItem() : id(-1), delay(-1) {};
        };

        struct AudioPacket {
            AVPacket curr;
            AudioPacket *prev;
            AudioPacket *next;
            
            AudioPacket() 
                : prev(NULL), next(NULL) {}
        };

        struct AudioPacketQueue {
            AudioPacket *head;
            AudioPacket *tail;
            AudioPacketQueue() : head(NULL), tail(NULL) {}
        };

        struct Frame {
            uint8_t *data;
            double pts;
            Frame() : data(NULL), pts(0) {};
        };

        TimerItem *timers[NATIVE_VIDEO_BUFFER_SAMPLES];
        AudioPacketQueue *audioQueue;
        AudioPacket *freePacket;
        int timerIdx;
        int lastTimer;
        int timersDelay;

        ape_global *net;
        NativeAudio *audio;
        NativeAudioTrack *track;

        VideoCallback frameCbk;
        void *frameCbkArg;

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

        int width;
        int height;

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
        pthread_mutex_t buffLock, waitLock;
        pthread_cond_t buffNotEmpty, wait;

        int open(void *buffer, int size);
        void frameCallback(VideoCallback cbk, void *arg);
        void play();
        void pause();
        void stop();
        NativeAudioTrack *getAudio();
        static void* decode(void *args);
        static int display(void *custom);

        ~NativeVideo();
    private :
        bool opened;
        NativeAVBufferReader *br;

        void close(bool reset);

        double syncVideo(double pts);
        double getClock();
        void scheduleDisplay(int delay);
        int addTimer(int delay);

        static int getBuffer(struct AVCodecContext *c, AVFrame *pic);
        static void releaseBuffer(struct AVCodecContext *c, AVFrame *pic);

};

#endif
