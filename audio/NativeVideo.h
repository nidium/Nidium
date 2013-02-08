#ifndef nativevideo_h__
#define nativevideo_h__

#include <pthread.h>
#include <stdint.h>
#include <native_netlib.h>

typedef void (*VideoCallback)(int width, int height, uint8_t *data, void *custom);
struct PaUtilRingBuffer;

struct SwsContext;
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;

class NativeVideo
{
    public :
        NativeVideo(ape_global *n, char *file);

        ape_global *net;
        VideoCallback cbk;
        void *cbkArg;

        bool shutdown;

        uint8_t *tmpFrame;
        double lastPts;
        static uint64_t pktPts;
        double videoClock;
        double frameTimer;

        int timer;
        SwsContext *swsCtx;
        AVFormatContext *pFormatCtx;
        AVCodecContext *pCodecCtx;
        int videoStream;

        PaUtilRingBuffer *rBuff;
        uint8_t *buff;
        AVFrame         *pFrame; 
        AVFrame         *pFrameRGB;


        pthread_t threadDecode;
        pthread_mutex_t buffLock;
        pthread_cond_t buffNotEmpty;

        void setCallback(VideoCallback cbk, void *arg);
        void play();
        static void* decode(void *args);
        static int display(void *custom);
        static int getBuffer(struct AVCodecContext *c, AVFrame *pic);
        static void releaseBuffer(struct AVCodecContext *c, AVFrame *pic);

        ~NativeVideo();
};

#endif
