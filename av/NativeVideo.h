#ifndef nativevideo_h__
#define nativevideo_h__

#include <pthread.h>
#include <stdint.h>
#include <native_netlib.h>
#include "NativeAudio.h"
#include "NativeAudioNode.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#define NATIVE_VIDEO_BUFFER_SAMPLES 16
#define NATIVE_VIDEO_AUDIO_SYNC_THRESHOLD 0.5
#define NATIVE_VIDEO_SYNC_THRESHOLD 0.01
#define NATIVE_VIDEO_NOSYNC_THRESHOLD 10.0
#define NATIVE_VIDEO_PACKET_BUFFER 64 

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
    friend class NativeVideoAudioSource;

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
        double frameTimer;
        double lastPts;
        double videoClock;
        double audioClock;
        double lastDelay;

        bool playing;
        bool stoped;
        int seekFlags;

        int m_Width;
        int m_Height;

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
        int open(const char *src);
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
        void setSize(int width, int height);
        int setSizeInternal();
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
        bool m_DoSetSize;
        int m_NewWidth;
        int m_NewHeight;
        bool m_NoDisplay;
        bool m_InDisplay;
        pthread_mutex_t audioLock;
        NATIVE_PTHREAD_VAR_DECL(m_NotInDisplay);
        pthread_mutex_t decodeThreadLock;

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
        bool convertFrame(AVFrame *frame, uint8_t *dst);

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
        void lockDecodeThread();
        void unlockDecodeThread();
        void closeFFMpeg();
};

class NativeVideoAudioSource: public NativeAudioSource 
{
  public:
      NativeVideoAudioSource(int out, NativeVideo *video, bool external) :
          NativeAudioSource(out, video->audio, external), 
          m_Video(video), m_FreePacket(NULL) 
      {
      };

      bool buffer();

    ~NativeVideoAudioSource() {
        if (m_FreePacket != NULL) {
            if (!this->packetConsumed) {
                // Free the packet here, otherwise the source destructor
                // will do it after we delete it.
                av_free_packet(this->tmpPacket);
                this->packetConsumed = true;
            }
            delete m_FreePacket;
        }
    }
  private:
      NativeVideo *m_Video;
      NativeVideo::Packet *m_FreePacket;
};

#endif
