/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef av_video_h__
#define av_video_h__

#include <stdint.h>

#include <ape_netlib.h>

#include "Audio.h"
#include "AudioNode.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

struct PaUtilRingBuffer;
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct SwsContext;

namespace Nidium {
namespace AV {

#define NIDIUM_VIDEO_BUFFER_SAMPLES 16
#define NIDIUM_VIDEO_AUDIO_SYNC_THRESHOLD 0.1
#define NIDIUM_VIDEO_SYNC_THRESHOLD 0.01
#define NIDIUM_VIDEO_NOSYNC_THRESHOLD 10.0
#define NIDIUM_VIDEO_PACKET_BUFFER 64

#define NIDIUM_VIDEO_SEEK_KEYFRAME 0x1
#define NIDIUM_VIDEO_SEEK_PREVIOUS 0x2

typedef void (*VideoCallback)(uint8_t *data, void *custom);

// {{{ Video
class VideoAudioSource;
class Video : public AVSource
{
    friend class VideoAudioSource;

public:
    Video(ape_global *n);

    struct TimerItem
    {
        int id;
        int delay;
        TimerItem() : id(-1), delay(-1){};
    };

    struct Packet
    {
        AVPacket curr;
        Packet *next;

        Packet() : next(NULL)
        {
        }
    };

    struct PacketQueue
    {
        Packet *head;
        Packet *tail;
        int count;
        PacketQueue() : head(NULL), tail(NULL), count(0)
        {
        }
    };

    struct Frame
    {
        uint8_t *data;
        double pts;
        Frame() : data(NULL), pts(0){};
    };

    TimerItem *m_Timers[NIDIUM_VIDEO_BUFFER_SAMPLES];
    PacketQueue *m_AudioQueue;
    PacketQueue *m_VideoQueue;
    int m_TimerIdx;
    int m_LastTimer;
    int m_TimersDelay;

    ape_global *m_Net;
    VideoAudioSource *m_AudioSource;

    VideoCallback m_FrameCbk;
    void *m_FrameCbkArg;

    bool m_Shutdown;

    uint8_t *m_TmpFrame;
    uint8_t *m_FrameBuffer;
    double m_FrameTimer;
    double m_LastPts;
    double m_VideoClock;
    double m_AudioClock;
    double m_LastDelay;

    bool m_Playing;
    bool m_PlayWhenReady = false;
    bool m_Stopped;

    int m_Width;
    int m_Height;

    SwsContext *m_SwsCtx;
    AVCodecContext *m_CodecCtx;
    int m_VideoStream;
    int m_AudioStream;

    PaUtilRingBuffer *m_rBuff;
    uint8_t *m_Buff;
    unsigned char *m_AvioBuffer;
    uint8_t *m_Frames[NIDIUM_VIDEO_BUFFER_SAMPLES];
    int m_FramesIdx;
    AVFrame *m_DecodedFrame;
    AVFrame *m_ConvertedFrame;

    pthread_t m_ThreadDecode;
    NIDIUM_PTHREAD_VAR_DECL(m_BufferCond);

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

    VideoAudioSource *getAudioNode(Audio *audio);
    static void *decode(void *args);
    static int display(void *custom);
    void setSize(int width, int height);
    int setSizeInternal();
    void releaseAudioNode(bool del = true);

    static void sourceNeedWork(void *ptr);

    ~Video();

private:
    AVReader *m_Reader;
    Audio *m_Audio;
    bool m_Buffering;
    bool m_ThreadCreated;
    bool m_SourceNeedWork;
    bool m_DoSetSize;
    int m_NewWidth;
    int m_NewHeight;
    bool m_NoDisplay;
    bool m_InDisplay;
    pthread_mutex_t m_AudioLock;
    NIDIUM_PTHREAD_VAR_DECL(m_NotInDisplay);
    pthread_mutex_t m_DecodeThreadLock;

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

    int64_t syncVideo(int64_t pts);
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
// }}}

// {{{ VideoAudioSource
class VideoAudioSource : public AudioSource
{
public:
    VideoAudioSource(int out, Video *video, bool external)
        : AudioSource(out, video->m_Audio, external), m_Video(video),
          m_FreePacket(NULL){};

    bool buffer();

    ~VideoAudioSource()
    {
        if (m_FreePacket != NULL) {
            if (!m_PacketConsumed) {
                // Free the packet here, otherwise the source destructor
                // will do it after we delete it.
                av_packet_unref(m_TmpPacket);
                m_PacketConsumed = true;
            }
            delete m_FreePacket;
        }
    }

private:
    Video *m_Video;
    Video::Packet *m_FreePacket;
};
// }}}

} // namespace AV
} // namespace Nidium

#endif
