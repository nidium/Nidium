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
#define NATIVE_VIDEO_AUDIO_SYNC_THRESHOLD 0.05 
#define NATIVE_VIDEO_SYNC_THRESHOLD 0.01 
#define NATIVE_VIDEO_NOSYNC_THRESHOLD 10.0
#define NATIVE_VIDEO_PACKET_BUFFER 256

#define NATIVE_VIDEO_SEEK_KEYFRAME 0x1
#define NATIVE_VIDEO_SEEK_PREVIOUS 0x2

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
            Packet *next;
            
            Packet() 
                : next(NULL) {}
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
        NativeAudioSource *audioSource;

        VideoCallback frameCbk;
        void *frameCbkArg;

        bool shutdown;

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
        int seekFlags;

        int width;
        int height;

        SwsContext *swsCtx;
        AVCodecContext *codecCtx;
        int videoStream;
        int audioStream;

        PaUtilRingBuffer *rBuff;
        uint8_t *buff;
        unsigned char *avioBuffer;
        uint8_t *m_Frames[NATIVE_VIDEO_BUFFER_SAMPLES];
        int m_FramesIdx;
        AVFrame *decodedFrame; 
        AVFrame *convertedFrame;

        pthread_t threadDecode;
        NATIVE_PTHREAD_VAR_DECL(bufferCond);

        void play();
        void pause();
        void stop();
        void close();
        int open(const char *chroot, const char *src);
        int open(void *buffer, int size);
        int openInit();
        int openInitInternal();
        static void openInitCoro(void *arg);
        double getClock();
        void seek(double time) 
        {
            this->seek(time, 0);
        }
        void seek(double time, uint32_t flags);

        void nextFrame();
        void prevFrame();
        void frameAt(double time, bool keyframe);

        void frameCallback(VideoCallback cbk, void *arg);

        NativeAudioSource *getAudioNode(NativeAudio *audio);
        static void* decode(void *args);
        static int display(void *custom);
        void stopAudio();

        static void sourceNeedWork(void *ptr);

        ~NativeVideo();
    private :
        NativeAVReader *reader;
        NativeAudio *audio;
        bool buffering;
        bool seeking;
        bool m_ThreadCreated;
        bool m_SourceNeedWork;
        pthread_mutex_t audioLock;

        void closeInternal(bool reset);
        static void seekCoro(void *arg);
        int64_t seekTarget(double time, int *flags);
        bool seekMethod(int64_t target, int flags);
        void seekInternal(double time);

        void buffer();
        static void bufferCoro(void *arg);
        void bufferInternal();

        bool processAudio();
        bool processVideo();
        bool processFrame(AVFrame *frame);

        double syncVideo(double pts);
        double getPts(AVPacket *packet);
        double getPts(AVFrame *frame);
        double getSyncedPts(AVPacket *packet);
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
