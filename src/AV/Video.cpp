/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Video.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <pa_ringbuffer.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __ANDROID__
#include <libavcodec/jni.h>
#endif
}

#ifdef __ANDROID__
#include <SDL.h>
#include <jni.h>
#endif

using Nidium::Core::PthreadAutoLock;

namespace Nidium {
namespace AV {

#undef DPRINT
#if 0
#define DEBUG_PRINT
#ifdef __ANDROID__
#define DPRINT(...)\
    __android_log_print(ANDROID_LOG_INFO, "Nidium", __VA_ARGS__)
#else
#define DPRINT(...)\
    fprintf(stdout, ">[%d]%lld / ", (unsigned int)pthread_self(), av_gettime() / 1000); \
    fprintf(stdout, __VA_ARGS__)
#endif
#else
#define DPRINT(...) (void)0
#endif

// XXX : Well, Video need a better interaction with Audio.
// There's a lot of little hack to work nicely with it.

Video::Video(ape_global *n)
    : m_TimerIdx(0), m_LastTimer(0), m_TimersDelay(0), m_Net(n),
      m_AudioSource(NULL), m_FrameCbk(NULL), m_FrameCbkArg(NULL),
      m_Shutdown(false), m_TmpFrame(0), m_FrameBuffer(0), m_FrameTimer(0),
      m_LastPts(0), m_VideoClock(0.0f), m_AudioClock(0.0f), m_LastDelay(0),
      m_Playing(false), m_Stopped(false), m_Width(0), m_Height(0),
      m_SwsCtx(NULL), m_CodecCtx(NULL), m_VideoStream(-1), m_AudioStream(-1),
      m_rBuff(NULL), m_Buff(NULL), m_AvioBuffer(NULL), m_FramesIdx(0),
      m_DecodedFrame(NULL), m_ConvertedFrame(NULL), m_Reader(NULL),
      m_Audio(NULL), m_Buffering(false), m_ThreadCreated(false),
      m_SourceNeedWork(false), m_DoSetSize(false), m_NewWidth(0),
      m_NewHeight(0), m_NoDisplay(false), m_InDisplay(false)
{
    NIDIUM_PTHREAD_VAR_INIT(&m_BufferCond);
    NIDIUM_PTHREAD_VAR_INIT(&m_NotInDisplay);

    pthread_mutex_init(&m_AudioLock, NULL);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_DecodeThreadLock, &mta);

    m_AudioQueue = new PacketQueue();
    m_VideoQueue = new PacketQueue();

    m_DoSemek   = false;
    m_Seeking   = false;
    m_SeekFlags = 0;

    for (int i = 0; i < NIDIUM_VIDEO_BUFFER_SAMPLES; i++) {
        m_Timers[i] = new TimerItem();
        m_Frames[i] = NULL;
    }
}

// {{{ Open
#define RETURN_WITH_ERROR(err)                       \
    this->sendEvent(SOURCE_EVENT_ERROR, err, false); \
    this->closeInternal(true);                       \
    return err;

int Video::open(void *buffer, int size)
{
    if (m_Container) {
        this->closeInternal(true);
    }

    if (!(m_AvioBuffer = static_cast<unsigned char *>(av_malloc(
              NIDIUM_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE)))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Reader    = new AVBufferReader((uint8_t *)buffer, size);
    m_Container = avformat_alloc_context();
    if (!m_Container || !m_Reader) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container->pb
        = avio_alloc_context(m_AvioBuffer, NIDIUM_AVIO_BUFFER_SIZE, 0, m_Reader,
                             AVBufferReader::read, NULL, AVBufferReader::seek);
    if (!m_Container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->openInit();

    if (pthread_create(&m_ThreadDecode, NULL, Video::decode, this) != 0) {
        RETURN_WITH_ERROR(ERR_INTERNAL);
    }
    m_ThreadCreated = true;
    NIDIUM_PTHREAD_SIGNAL(&m_BufferCond);

    return 0;
}

int Video::open(const char *src)
{
    DPRINT("Open %s\n", src);
    if (m_Container != NULL) {
        this->closeInternal(true);
    }

    m_Container = avformat_alloc_context();
    if (!m_Container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    Core::Path p = Core::Path(src);
    m_Filename = strdup(p.path());

    this->openInit();

    if (pthread_create(&m_ThreadDecode, NULL, Video::decode, this) != 0) {
        RETURN_WITH_ERROR(ERR_INTERNAL);
    }

    m_ThreadCreated = true;

    return 0;
}
#undef RETURN_WITH_ERROR

int Video::openInit()
{
    DPRINT("openInit()\n");
    m_SourceDoOpen = true;
    NIDIUM_PTHREAD_SIGNAL(&m_BufferCond);
    return 0;
}

int Video::openInitInternal()
{
    DPRINT("openInitInternal");
    // FFmpeg stuff
    AVCodec *codec;

    av_register_all();
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    JavaVM *vm;
    env->GetJavaVM(&vm);
    av_jni_set_java_vm(vm, nullptr);
#endif

    int ret
        = avformat_open_input(&m_Container, m_Filename, NULL, NULL);

    if (ret != 0) {
        char error[1024];
        av_strerror(ret, error, 1024);
        DPRINT("Couldn't open file : %s\n", error);
        return ERR_FAILED_OPEN;
    }

    PthreadAutoLock lock(&AVSource::m_FfmpegLock);
    if (avformat_find_stream_info(m_Container, NULL) < 0) {
        DPRINT("Couldn't find stream information\n");
        return ERR_NO_INFORMATION;
    }

    av_dump_format(m_Container, 0, m_Filename, 0);

    for (unsigned int i = 0; i < m_Container->nb_streams; i++) {
        if (m_Container->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && m_VideoStream == -1) {
            m_VideoStream = i;
        } else if (m_Container->streams[i]->codec->codec_type
                       == AVMEDIA_TYPE_AUDIO
                   && m_AudioStream == -1) {
            m_AudioStream = i;
        }
    }

    if (m_VideoStream == -1) {
        return ERR_NO_VIDEO;
    }

    m_CodecCtx = m_Container->streams[m_VideoStream]->codec;
    // this->codecCtx->get_buffer = Video::getBuffer;
    // this->codecCtx->release_buffer = Video::releaseBuffer;

    codec = avcodec_find_decoder(m_CodecCtx->codec_id);
    //codec = avcodec_find_decoder_by_name ("h264_mediacodec");
    if (!codec) {
        DPRINT("Codec not found");
        return ERR_NO_CODEC;
    }

    if (avcodec_open2(m_CodecCtx, codec, NULL) < 0) {
        DPRINT("Could not find or open the needed codec\n");
        return ERR_NO_CODEC;
    }

    // AV stuff
    m_LastDelay = 40e-3; // 40ms, default delay between frames a 30fps

    // Ringbuffer that hold reference to decoded frames
    m_rBuff = new PaUtilRingBuffer();
    m_Buff
        = (uint8_t *)malloc(sizeof(Video::Frame) * NIDIUM_VIDEO_BUFFER_SAMPLES);

    if (m_Buff == NULL) {
        DPRINT("Failed to alloc buffer\n");
        return ERR_OOM;
    }

    if (0 > PaUtil_InitializeRingBuffer(m_rBuff, sizeof(Video::Frame),
                                        NIDIUM_VIDEO_BUFFER_SAMPLES, m_Buff)) {
        DPRINT("Failed to init ringbuffer\n");
        return ERR_OOM;
    }

    int err;
    if ((err = this->setSizeInternal()) < 0) {
        return err;
    }

    m_Opened = true;

    if (m_PlayWhenReady) {
        this->play();
    }

    this->sendEvent(SOURCE_EVENT_READY, 0, false);

    return 0;
}
#undef RETURN_WITH_ERROR
// }}}

void Video::frameCallback(VideoCallback cbk, void *arg)
{
    m_FrameCbk    = cbk;
    m_FrameCbkArg = arg;
}

void Video::play()
{
    if (m_Playing) {
        return;
    }

    if (!m_Opened) {
        m_PlayWhenReady = true;
        return;
    }

    m_Playing       = true;
    m_PlayWhenReady = false;
    m_FrameTimer    = 0;

    bool haveTimer = false;
    for (int i = 0; i < NIDIUM_VIDEO_BUFFER_SAMPLES; i++) {
        if (m_Timers[i]->id == -1 && m_Timers[i]->delay != -1) {
            haveTimer       = true;
            m_Timers[i]->id = this->addTimer(m_Timers[i]->delay);
        }
    }

    if (!haveTimer) {
        this->scheduleDisplay(1);
    }

    this->sendEvent(SOURCE_EVENT_PLAY, 0, false);
}

void Video::pause()
{
    m_Playing = false;

    this->clearTimers(true);

    if (m_AudioSource != NULL) {
        m_AudioSource->pause();
    }

    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void Video::close()
{
    m_Playing    = false;
    m_VideoClock = 0;
    m_LastDelay  = 40e-3;
    m_Error      = 0;

    this->closeInternal(true);
}

void Video::stop()
{
    m_Playing    = false;
    m_VideoClock = 0;
    m_LastDelay  = 40e-3;
    m_Error      = 0;

    if (m_AudioSource != NULL) {
        m_AudioSource->stop();
    }

    this->seek(0);

    this->flushBuffers();
    this->clearTimers(true);

    NIDIUM_PTHREAD_SIGNAL(&m_BufferCond);

    this->sendEvent(SOURCE_EVENT_STOP, 0, false);
}

double Video::getClock()
{
    return m_LastPts;
}

// {{{ Seek
void Video::seek(double time, uint32_t flags)
{
    DPRINT("Seek called\n");
    if (!m_Opened || m_DoSemek) {
        DPRINT("not seeking cause already seeking\n");
        return;
    }

    m_DoSeekTime = time < 0 ? 0 : time;
    m_DoSemek    = true;
    m_SeekFlags  = flags;

    this->clearTimers(true);
    this->flushBuffers();

    NIDIUM_PTHREAD_SIGNAL(&m_BufferCond);

    this->scheduleDisplay(1, true);
}

bool Video::seekMethod(int64_t target, int flags)
{
    DPRINT("av_seek_frame\n");
    if (!(m_SeekFlags & NIDIUM_VIDEO_SEEK_KEYFRAME) && !(flags & AVSEEK_FLAG_BACKWARD)) {
        flags |= AVSEEK_FLAG_ANY;
    }
    int ret = av_seek_frame(m_Container, m_VideoStream, target, flags);
    DPRINT("av_seek_frame done ret=%d\n", ret);
    if (ret >= 0) {
        m_Error = 0;

        // FFMPEG can success seeking even if seek is after the end
        // (but decoder/demuxer fail). So fix that.
        if (m_DoSeekTime > this->getDuration()) {
            m_LastPts = this->getDuration();
        } else {
            m_LastPts = m_DoSeekTime;
        }

        // We need to update the videoClock in case of seek @ 0
        // because syncVideo() will get a packet with PTS @ 0
        // and it will not update the clock
        if (target == 0) {
            m_VideoClock = 0;
        }

        return true;
    } else {
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
        return false;
    }
}

int64_t Video::seekTarget(double time, int *flags)
{
    double clock   = m_LastPts;
    int64_t target = 0;

    *flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;
    target = time * AV_TIME_BASE;
    return av_rescale_q(target, AV_TIME_BASE_QQ,
                        m_Container->streams[m_VideoStream]->time_base);
}

#define SEEK_THRESHOLD 2
#define SEEK_BUFFER_PACKET 16
#define SEEK_STEP 2
void Video::seekInternal(double time)
{
#ifdef DEBUG_PRINT
    double start = av_gettime();
#endif
    double startFrame = 0;
    int flags;
    int64_t target;
    double seekTime;
    double diff;

    int gotFrame  = 0;
    double pts    = 0;
    bool keyframe = false;
    bool frame    = false;
    Packet *p     = NULL;
    AVPacket packet;

    DPRINT("SeekInternal\n");

    if (time > this->getDuration()) {
        time = this->getDuration();
        time -= 0.1;
    }

    seekTime = time;
    diff     = time - this->getClock();
    target   = this->seekTarget(time, &flags);

    DPRINT("[SEEK] diff = %f, time = %lld\n", diff, av_gettime());
    if (diff > SEEK_THRESHOLD || diff <= 0
        || (m_SeekFlags & NIDIUM_VIDEO_SEEK_KEYFRAME)) {
        // Flush all buffers
        this->clearAudioQueue();
        this->clearVideoQueue();

        if (m_AudioSource != NULL) {
            avcodec_flush_buffers(m_AudioSource->m_CodecCtx);
            PaUtil_FlushRingBuffer(m_AudioSource->m_rBufferOut);
            m_AudioSource->resetFrames();
        }

        if (m_SeekFlags & NIDIUM_VIDEO_SEEK_PREVIOUS) {
            flags |= AVSEEK_FLAG_BACKWARD;
        }

        // Seek to desired TS
        if (!this->seekMethod(target, flags)) {
            return;
        }

        avcodec_flush_buffers(m_CodecCtx);

        if (m_SeekFlags & NIDIUM_VIDEO_SEEK_KEYFRAME) {
            return;
        }
    } else {
        // Seeking forward less than SEEK_THRESHOLD
        // instead of looking for a keyframe, we just
        // read packet until we hit the desired time
        keyframe = true;
        if (m_AudioSource != NULL) {
            // "in memory" seeking, we need to drop audio packet
            for (;;) {
                double tmp;

                p = this->getPacket(m_AudioQueue);
                if (p == NULL) {
                    break;
                }

                tmp = av_q2d(m_Container->streams[m_AudioStream]->time_base)
                      * p->curr.pts;
                DPRINT("[SEEK] Dropping audio packet @ %f\n", tmp);

                av_packet_unref(&p->curr);
                delete p;

                if (tmp >= time) {
                    break;
                }
            }
        }
    }

    if (m_Eof && flags == AVSEEK_FLAG_BACKWARD) {
        m_Eof = false;
        if (m_AudioSource != NULL) {
            m_AudioSource->m_Eof = false;
        }
    }

    flags = 0;

    for (;;) {
        DPRINT("[SEEK] loop gotFrame = %d pts = %f seekTime = %f time = %f\n",
               gotFrame, pts, seekTime, time);
        if ((pts > seekTime + SEEK_STEP || pts > time) && !keyframe) {
            seekTime = seekTime - SEEK_STEP;
            if (seekTime < 0) seekTime = 0;
            target = this->seekTarget(seekTime, &flags);
            DPRINT("    => seeking backward to %f/%lld\n", seekTime, target);
            if (!this->seekMethod(target, flags)) {
                return;
            }
            avcodec_flush_buffers(m_CodecCtx);
            keyframe = false;
            frame    = false;
            pts      = 0;
            flags = 0;
            this->clearVideoQueue();
        }

        if (m_VideoQueue->count == 0) {
            DPRINT("[SEEK] av_read_frame\n");
            int count = 0;
            while (count < SEEK_BUFFER_PACKET) {
                int err = av_read_frame(m_Container, &packet);
                if (err < 0) {
                    av_packet_unref(&packet);
                    err = this->readError(err);
                    if (err == AVERROR_EOF && m_VideoQueue->count > 0) {
                        break;
                    } else if (err < 0) {
                        DPRINT("[SEEK] Got fatal error %d\n", err);
                        return;
                    }
                    continue;
                }
                if (packet.stream_index == m_VideoStream) {
                    double tmp = this->getPts(&packet);
                    DPRINT("[SEEK] Got packet @ %f\n", tmp);
                    this->addPacket(m_VideoQueue, &packet);
                    count++;
                    if ((tmp > seekTime + SEEK_STEP || tmp > time)
                        && !keyframe) {
                        pts = tmp;
                        break;
                    }
                } else {
                    av_packet_unref(&packet);
                }
            }
            p = this->getPacket(m_VideoQueue);
            if (p == NULL) {
                continue;
            }
            packet = p->curr;
        } else {
            DPRINT("[SEEK] getPacket\n");
            p = this->getPacket(m_VideoQueue);
            if (p == NULL) {
                continue;
            }
            packet = p->curr;
        }

        DPRINT("[SEEK] reading packet stream = %d, pts = %lld/%lld\n",
               packet.stream_index, packet.pts, packet.dts);

        if (packet.stream_index == m_VideoStream) {
            pts   = this->getPts(&packet);
            frame = true;

            DPRINT("[SEEK] got video frame at = %f flags = %d\n", pts,
                   packet.flags);


            if ((packet.flags & AV_PKT_FLAG_KEY) || keyframe) {
                // We have our frame!
                if ((pts >= time || seekTime == 0) && gotFrame) {
                    if ((m_SeekFlags & NIDIUM_VIDEO_SEEK_PREVIOUS)
                        && (pts != time && seekTime != 0)) {
                        DPRINT("[SEEK] Seeked too far, rewind\n");
                        // When seeking to the previous frame, we need to be at
                        // the exact frame. As it's not the case, seek backward
                        pts += 120;
                        keyframe = false;
                        continue;
                    }
                    DPRINT("[SEEK] got seek frame at = %f\n", pts);
                    Packet *tmp        = m_VideoQueue->head;
                    m_VideoQueue->head = p;
                    if (tmp == NULL) {
                        m_VideoQueue->tail = p;
                    }
                    p->next = tmp;
                    m_VideoQueue->count++;

                    if (m_SeekFlags & NIDIUM_VIDEO_SEEK_PREVIOUS) {
                        this->processFrame(m_DecodedFrame);
                    }
                    break;
                }

                DPRINT("[SEEK]  its a keyframe\n");
                avcodec_decode_video2(m_CodecCtx, m_DecodedFrame, &gotFrame,
                                      &packet);
                if (gotFrame) {
                    DPRINT("[SEEK] = = = = GOT FRAME\n");
                    if (!keyframe) {
                        startFrame = av_gettime() / 1000;
                    }
                }
                keyframe = true;
            }
        }

        av_packet_unref(&packet);
        if (p != NULL) {
            delete p;
            p = NULL;
        }
    }

    m_FrameTimer = 0; // frameTimer will be init at next display
    m_LastPts    = time;
    m_DoSemek    = false;


#ifdef DEBUG_PRINT
    double end = av_gettime();
    DPRINT("[SEEK] seek took %f / firstFrame to end = %f \n", end - start,
           end - startFrame);
    DPRINT("Sending seekCond signal\n");
#endif
    this->processVideo();
    NIDIUM_PTHREAD_SIGNAL(&m_BufferCond);
}
#undef SEEK_THRESHOLD
#undef SEEK_STEP
#undef SEEK_BUFFER_PACKET
/// }}}

// {{{ Frame
void Video::nextFrame()
{
    if (m_Playing || !m_Opened) {
        return;
    }
    this->scheduleDisplay(1, true);
}

void Video::prevFrame()
{
    if (m_Playing || !m_Opened) {
        return;
    }
    this->seek(this->getClock(), NIDIUM_VIDEO_SEEK_PREVIOUS);
}

void Video::frameAt(double time, bool keyframe)
{
    if (m_Playing || !m_Opened) {
        return;
    }

    this->seek(time, keyframe ? NIDIUM_VIDEO_SEEK_KEYFRAME : 0);
}
// }}}

VideoAudioSource *Video::getAudioNode(Audio *audio)
{

    if (m_AudioSource) {
        return m_AudioSource;
    }

    m_Audio = audio;

    if (m_AudioStream != -1 && audio) {
        m_AudioSource = new VideoAudioSource(2, this, true);
        audio->addSource(m_AudioSource, true);
        m_AudioSource->m_AudioStream = m_AudioStream;
        m_AudioSource->m_Container = m_Container;
        m_AudioSource->eventCallback(NULL, NULL); // Disable events callbacks
        if (0 != m_AudioSource->initInternal()) {
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_INIT_VIDEO_AUDIO, false);
            delete m_AudioSource;
            m_AudioSource = NULL;
        }

        if (this->getClock() > 0) {
            this->clearAudioQueue();
        } else if (m_Playing && m_AudioSource) {
            m_AudioSource->play();
        }
    }
    return m_AudioSource;
}

int Video::display(void *custom)
{
    Video *v = static_cast<Video *>(custom);

    // Set pthread cond to false, so
    // setSize will always wait for the signal
    v->m_NotInDisplayCond = false;

    v->m_InDisplay = true;

    DPRINT("[DISPLAY] %d\n", v->m_NoDisplay);

    // Reset timer from queue
    v->m_Timers[v->m_LastTimer]->id    = -1;
    v->m_Timers[v->m_LastTimer]->delay = -1;

    v->m_LastTimer++;
    if (v->m_LastTimer > NIDIUM_VIDEO_BUFFER_SAMPLES - 1) {
        v->m_LastTimer = 0;
    }

    if (v->m_NoDisplay) {
        // Frames are going to be resized.
        // Don't do anything
        v->scheduleDisplay(1, true);
        v->m_InDisplay = false;
        NIDIUM_PTHREAD_SIGNAL(&v->m_NotInDisplay);
        return 0;
    }

    // Read frame from ring buffer
    if (PaUtil_GetRingBufferReadAvailable(v->m_rBuff) < 1) {
        if (!v->m_Buffering && !v->m_Seeking) {
            NIDIUM_PTHREAD_SIGNAL(&v->m_BufferCond);
        }
        if (v->m_Eof) {
            DPRINT("No frame, eof reached\n");
            v->sendEvent(SOURCE_EVENT_EOF, 0, false);
        } else {
            DPRINT("No frame, try again in 20ms\n");
            v->scheduleDisplay(1, true);
        }

        v->m_InDisplay = false;
        NIDIUM_PTHREAD_SIGNAL(&v->m_NotInDisplay);

        return 0;
    }

    Frame frame;
    PaUtil_ReadRingBuffer(v->m_rBuff, (void *)&frame, 1);

    double pts  = frame.pts;
    double diff = 0;
    double delay, actualDelay;

    if (v->m_FrameTimer == 0) {
        v->m_FrameTimer = (av_gettime() / 1000.0);
        if (v->m_AudioSource != NULL) {
            v->m_AudioSource->play();
        }
    }

    delay = pts - v->m_LastPts;
    DPRINT("DELAY=%f\n", delay);
    if (delay <= 0 || delay >= 1.0) {
        DPRINT("USING LAST DELAY %f\n", v->m_LastDelay);
        // Incorrect delay, use previous one
        delay = v->m_LastDelay;
    }

    v->m_LastDelay = delay;
    v->m_LastPts   = pts;

    if (v->m_AudioSource != NULL && v->m_AudioSource->m_IsConnected) {
        diff = pts - v->m_AudioSource->getClock();

        DPRINT("Clocks audio=%f / video=%f / diff = %f\n",
               v->m_AudioSource->getClock(), pts, diff);

        if (diff > NIDIUM_VIDEO_AUDIO_SYNC_THRESHOLD
            && v->m_AudioSource->avail() > 0) {
            // Diff is too big an will be noticed
            // Let's drop some audio sample
            DPRINT("Dropping audio before=%f ", diff);
            diff -= v->m_AudioSource->drop(diff);
            DPRINT("after=%f sec)\n", diff);
        } else {
            double syncThreshold;

            syncThreshold = (delay >= NIDIUM_VIDEO_SYNC_THRESHOLD)
                                ? delay
                                : NIDIUM_VIDEO_SYNC_THRESHOLD;

            if (fabs(diff) < NIDIUM_VIDEO_NOSYNC_THRESHOLD) {
                if (diff <= -syncThreshold) {
                    DPRINT(" (diff < syncThreshold) ");
                    delay = 0;
                } else if (diff >= syncThreshold) {
                    DPRINT(" (diff > syncThreshold) ");
                    delay = 2 * delay;
                }
            }
            DPRINT(" | after  %f\n", delay);
        }
    }

    DPRINT("frameTimer=%f delay=%f videoClock=%f\n", v->m_FrameTimer, delay,
           pts);
    if (delay < 0) delay = 0;
    v->m_FrameTimer += delay * 1000;
    actualDelay = (v->m_FrameTimer - (av_gettime() / 1000.0)) / 1000;

    DPRINT("Using delay %f diff=%f delay=%f\n", actualDelay, diff, delay);

    if (v->m_Playing) {
        if (actualDelay <= NIDIUM_VIDEO_SYNC_THRESHOLD && diff <= 0) {
            DPRINT("Droping video frame\n");
            v->display(v);
            v->m_InDisplay = false;
            NIDIUM_PTHREAD_SIGNAL(&v->m_NotInDisplay);
            return 0;
        } else {
            DPRINT("Next display in %d\n", (int)(actualDelay * 1000));
            v->scheduleDisplay(((int)(actualDelay * 1000)));
        }
    }

    if (actualDelay > NIDIUM_VIDEO_SYNC_THRESHOLD || diff > 0
        || !v->m_Playing) {
        // If not playing, we can be here because user seeked
        // while the video is paused, so send the frame anyway

        // Call the frame callback
        if (v->m_FrameCbk != NULL) {
            v->m_FrameCbk(frame.data, v->m_FrameCbkArg);
        }
    }

    if (v->m_Playing) {
        NIDIUM_PTHREAD_SIGNAL(&v->m_BufferCond);
    }

    v->m_InDisplay = false;
    NIDIUM_PTHREAD_SIGNAL(&v->m_NotInDisplay);

    return 0;
}

int Video::setSizeInternal()
{
    m_NoDisplay = true;

    if (m_InDisplay) {
        NIDIUM_PTHREAD_WAIT(&m_NotInDisplay);
    }

    int width  = m_NewWidth == 0 ? m_CodecCtx->width : m_NewWidth;
    int height = m_NewHeight == 0 ? m_CodecCtx->height : m_NewHeight;

    if (!m_SwsCtx) {
        // First call to setSizeInternal, init frames
        m_DecodedFrame   = av_frame_alloc();
        m_ConvertedFrame = av_frame_alloc();

        if (m_DecodedFrame == NULL || m_ConvertedFrame == NULL) {
            DPRINT("Failed to alloc frame\n");
            m_NoDisplay = false;
            return ERR_OOM;
        }

        m_SwsCtx = sws_getContext(m_CodecCtx->width, m_CodecCtx->height,
                                  m_CodecCtx->pix_fmt, m_CodecCtx->width, m_CodecCtx->height,
                                  AV_PIX_FMT_RGBA,
                                  SWS_BICUBIC, NULL, NULL, NULL);

        if (!m_SwsCtx) {
            m_NoDisplay = false;
            return ERR_NO_VIDEO_CONVERTER;
        }

        AVPicture *picture = reinterpret_cast<AVPicture *>(m_ConvertedFrame);
        int frameSize = av_image_fill_arrays(picture->data, picture->linesize,
                        NULL, AV_PIX_FMT_RGBA, m_CodecCtx->width, m_CodecCtx->height, 1);

        for (int i = 0; i < NIDIUM_VIDEO_BUFFER_SAMPLES; i++) {
            free(m_Frames[i]);
            m_Frames[i] = static_cast<uint8_t *>(malloc(frameSize));
        }
    }

    m_Width  = width;
    m_Height = height;

    m_NoDisplay = false;
    return 0;
}

void Video::setSize(int width, int height)
{
    if (width == m_Width && height == m_Height && m_SwsCtx) {
        return;
    }

    m_NewWidth  = width;
    m_NewHeight = height;

    m_DoSetSize = true;

    // NIDIUM_PTHREAD_SIGNAL(&this->bufferCond);
}

// {{{ Buffer
void Video::buffer()
{
    if (m_Error != 0) {
        DPRINT("=> Not buffering cause of error\n");
        return;
    }

    this->bufferInternal();
}


void Video::bufferInternal()
{
    DPRINT("hello buffer internal\n");
    AVPacket packet;

    bool loopCond = false;

    int needAudio = 0;
    int needVideo = 0;

    if (m_Playing) {
        if (m_AudioSource != NULL && m_AudioSource->m_IsConnected) {
            needAudio = NIDIUM_VIDEO_PACKET_BUFFER - m_AudioQueue->count;
        }
        needVideo = NIDIUM_VIDEO_PACKET_BUFFER - m_VideoQueue->count;
    } else {
        needVideo = 1;
    }


    if (needAudio > 0 || needVideo > 0) {
        loopCond = true;
    }

    while (loopCond) {
        DPRINT("    => buffering loop needAudio=%d / needVideo=%d\n", needAudio,
               needVideo);
        int ret = av_read_frame(m_Container, &packet);
        DPRINT("    -> post read frame\n");

        // If a seek is asked while buffering. Return.
        if (m_DoSemek) {
            av_packet_unref(&packet);
            break;
        }

        // Got a read error. Return.
        if (ret < 0) {
            av_packet_unref(&packet);
            if (this->readError(ret) != 0) {
                return;
            }
            continue;
        }

        if (packet.stream_index == m_VideoStream) {
            this->addPacket(m_VideoQueue, &packet);
            needVideo--;
        } else if (packet.stream_index == m_AudioStream
                   && ((m_AudioSource != NULL && m_AudioSource->m_IsConnected)
                       || this->getClock() == 0)) {
            this->addPacket(m_AudioQueue, &packet);
            needAudio--;
        } else {
            av_packet_unref(&packet);
        }

        if ((needVideo <= 0 && needAudio <= 0) || m_Error != 0) {
            DPRINT("=> Bufffering loop finished\n");
            break;
        }
    }
}
// }}}

void *Video::decode(void *args)
{
    Video *v = static_cast<Video *>(args);

    for (;;) {
        DPRINT("decode loop\n");
        v->lockDecodeThread();

        if (v->m_Shutdown) {
            v->unlockDecodeThread();
            break;
        }

        if (v->m_Opened) {
            DPRINT("opened buffering=%d\n", v->m_Buffering);
            if (!v->m_Buffering && !v->m_Seeking) {
                DPRINT("not buffering and seeking\n");
                if (v->m_DoSemek == true) {
                    DPRINT("seeking\n");
                    v->seekInternal(v->m_DoSeekTime);
                    v->m_DoSemek = false;
                    DPRINT("done seeking\n");
                } else {
                    DPRINT("buffering\n");
                    v->buffer();
                }
            }

            DPRINT("doSeek=%d readFlag=%d seeking=%d\n", v->m_DoSemek,
                   v->m_SourceNeedWork, v->m_Seeking);
            if (!v->m_DoSemek) {
                DPRINT("processing\n");
                bool videoFailed = !v->processVideo();
                bool audioFailed = !v->processAudio();
#ifdef DEBUG_PRINT
                DPRINT("audioFailed=%d videoFailed=%d videoAvail=%ld\n", audioFailed,
                       videoFailed, PaUtil_GetRingBufferWriteAvailable(v->m_rBuff));
#else
                (void)videoFailed;
                (void)audioFailed;
#endif
            }
        } else if (v->m_SourceDoOpen) {
            v->m_SourceDoOpen = false;
            int ret = v->openInitInternal();
            if (ret != 0) {
                v->sendEvent(SOURCE_EVENT_ERROR, ret, true /* threaded */);
                v->postMessage(v, AVSource::MSG_CLOSE);
            }
        }

        v->unlockDecodeThread();

        if (v->m_Shutdown) break;

        if (!v->m_DoSemek) {
            DPRINT("wait bufferCond, no work needed\n");
            NIDIUM_PTHREAD_WAIT(&v->m_BufferCond);
            DPRINT("Waked up from bufferCond!");
        }
    }

    v->m_Shutdown = false;

    return NULL;
}

void Video::releaseAudioNode(bool del)
{
    if (!m_AudioSource) return;

    PthreadAutoLock lock(&m_AudioLock);

    this->clearAudioQueue();

    if (del) {
        delete m_AudioSource;
    }

    m_Audio = NULL;
    m_AudioSource = NULL;
}

void Video::sourceNeedWork(void *ptr)
{
    DPRINT("m_SourceNeedWork=true\n");
    Video *thiz            = static_cast<Video *>(ptr);
    thiz->m_SourceNeedWork = true;
    NIDIUM_PTHREAD_SIGNAL(&thiz->m_BufferCond);
}

// {{{ process
bool Video::processAudio()
{
    PthreadAutoLock lock(&m_AudioLock);
    DPRINT("processing audio\n");

    if (m_AudioSource == NULL) {
        return false;
    }

    if (!m_AudioSource->m_IsConnected) {
        return false;
    }

    while (m_AudioSource->work()) {
    }

    // TODO : We should wakup the thread only
    // if source had processed data
    if (m_Audio->canWriteFrame()) {
        DPRINT("Wakeup thread\n");
        NIDIUM_PTHREAD_SIGNAL(&m_Audio->m_QueueHaveData);
    }

    return true;
}

bool Video::processVideo()
{
    DPRINT("process video\n");
    if (m_DoSetSize) {
        m_DoSetSize = false;
        int ret     = this->setSizeInternal();

        if (ret < 0) {
            return false;
        }
    }

    while (PaUtil_GetRingBufferWriteAvailable(m_rBuff) > 0) {
        //DPRINT("in loop i=%d avail=%ld", i, PaUtil_GetRingBufferWriteAvailable(m_rBuff));
        int gotFrame;
        Packet *p = this->getPacket(m_VideoQueue);

        if (p == NULL) {
            DPRINT("processVideo no more packet\n");
            if (m_Error == AVERROR_EOF) {
                m_Eof = true;
            }
            return false;
        }

        AVPacket packet = p->curr;

        //DPRINT("decode video");
        avcodec_decode_video2(m_CodecCtx, m_DecodedFrame, &gotFrame, &packet);
        //DPRINT("end decode video");

        if (gotFrame) {
            this->processFrame(m_DecodedFrame);
            //DPRINT("end process frame");
        }

        delete p;
        av_packet_unref(&packet);
        //i++;
    }

    //DPRINT("process video over");
    return PaUtil_GetRingBufferWriteAvailable(m_rBuff) != 0;
}

bool Video::processFrame(AVFrame *avFrame)
{
    DPRINT("Processing video frame\n");
    Frame frame;
    frame.data = m_Frames[m_FramesIdx];
    frame.pts  = this->getPts(avFrame);

    bool ret = this->convertFrame(avFrame, (uint8_t *)frame.data);

    // Write frame to rBuff, the frame will be consumed
    // later by Video::display() (UI Thread)
    PaUtil_WriteRingBuffer(m_rBuff, &frame, 1);

    if (m_FramesIdx == NIDIUM_VIDEO_BUFFER_SAMPLES - 1) {
        m_FramesIdx = 0;
    } else {
        m_FramesIdx++;
    }

    return ret;
}
// }}}

bool Video::convertFrame(AVFrame *avFrame, uint8_t *dst)
{
    // Format the frame for sws_scale
    uint8_t *tmp[1];
    tmp[0]              = (uint8_t *)dst;
    uint8_t *const *out = (uint8_t * const *)tmp;

    // Convert the image from its native format to RGBA
    // TODO : Move this to a shader
    sws_scale(m_SwsCtx, avFrame->data, avFrame->linesize, 0, m_CodecCtx->height,
              out, m_ConvertedFrame->linesize);

    return true;
}

int64_t Video::syncVideo(int64_t pts)
{
    double frameDelay;

    if (pts != 0 && pts != AV_NOPTS_VALUE) {
        m_VideoClock = pts;
    } else {
        pts = m_VideoClock;
    }

    frameDelay = av_q2d(m_Container->streams[m_VideoStream]->time_base);
    frameDelay += m_DecodedFrame->repeat_pict * (frameDelay);
    m_VideoClock += frameDelay;

    return pts;
}

double Video::getPts(AVFrame *frame)
{
    uint64_t pts = av_frame_get_best_effort_timestamp(frame);

    return this->syncVideo(pts)
           * av_q2d(m_Container->streams[m_VideoStream]->time_base);
}

double Video::getPts(AVPacket *packet)
{
    double pts;

    if (packet->pts != AV_NOPTS_VALUE) {
        pts = packet->pts;
    } else if (packet->dts != AV_NOPTS_VALUE) {
        pts = packet->dts;
    } else {
        pts = 0;
    }

    return this->syncVideo(pts)
           * av_q2d(m_Container->streams[m_VideoStream]->time_base);
}

void Video::scheduleDisplay(int delay)
{
    this->scheduleDisplay(delay, false);
}

void Video::scheduleDisplay(int delay, bool force)
{
    if (!m_Opened) {
        return;
    }

    m_Timers[m_TimerIdx]->delay = delay;
    m_Timers[m_TimerIdx]->id    = -1;

    DPRINT("scheduleDisplay in %d\n", delay);

    if (m_Playing || force) {
        m_Timers[m_TimerIdx]->id = this->addTimer(delay);
    }

    m_TimerIdx++;

    if (m_TimerIdx > NIDIUM_VIDEO_BUFFER_SAMPLES - 1) {
        m_TimerIdx = 0;
    }
}

int Video::addTimer(int delay)
{
    /* XXX timer is protected by default and will not be cleared upon refresh.
        Is that the desired behaviour ? */
    return APE_timer_getid(
        APE_timer_create(m_Net, delay, Video::display, this));
}

bool Video::addPacket(PacketQueue *queue, AVPacket *packet)
{
    Packet *pkt = new Packet();
    if (packet->buf) {
        pkt->curr = *packet;
    } else {
        AVPacket *dst = av_packet_clone(packet);
        pkt->curr = *dst;
    }

    if (!queue->tail) {
        queue->head = pkt;
    } else {
        queue->tail->next = pkt;
    }

    queue->tail = pkt;
    queue->count++;

    return true;
}

Video::Packet *Video::getPacket(PacketQueue *queue)
{
    if (queue->count == 0 || queue->head == NULL) {
        return NULL;
    }

    Packet *pkt = queue->head;

    queue->head = pkt->next;

    if (!queue->head) {
        queue->tail = NULL;
    }

    if (pkt != NULL) {
        queue->count--;
    }

    return pkt;
}

void Video::clearTimers(bool reset)
{
    for (int i = 0; i < NIDIUM_VIDEO_BUFFER_SAMPLES; i++) {
        if (m_Timers[i]->id != -1 && m_Timers[i]->delay != -1) {
            APE_timer_clearbyid(m_Net, m_Timers[i]->id, 1);
        }
        if (reset) {
            m_Timers[i]->id    = -1;
            m_Timers[i]->delay = -1;
        } else {
            delete m_Timers[i];
            m_Timers[i] = NULL;
        }
    }
}

void Video::clearAudioQueue()
{
    /*
    if (this->freePacket != NULL) {
        if (this->audioSource != NULL) {
            this->audioSource->packetConsumed = true;
        }
        av_packet_unref(&this->freePacket->curr);
        delete this->freePacket;
        this->freePacket = NULL;
    }
    */

    Packet *pkt = m_AudioQueue->head;
    Packet *next;
    while (pkt != NULL) {
        next = pkt->next;
        av_packet_unref(&pkt->curr);
        delete pkt;
        pkt = next;
    }
    m_AudioQueue->head  = NULL;
    m_AudioQueue->tail  = NULL;
    m_AudioQueue->count = 0;
}

void Video::clearVideoQueue()
{
    Packet *pkt = m_VideoQueue->head;
    Packet *next;
    while (pkt != NULL) {
        next = pkt->next;
        av_packet_unref(&pkt->curr);
        delete pkt;
        pkt = next;
    }
    m_VideoQueue->head  = NULL;
    m_VideoQueue->tail  = NULL;
    m_VideoQueue->count = 0;
}

void Video::flushBuffers()
{
    if (m_rBuff == NULL) return;

    m_FramesIdx = 0;

    PaUtil_FlushRingBuffer(m_rBuff);
}

#if 0
int Video::getBuffer(struct AVCodecContext *c, AVFrame *pic) {
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t *)av_malloc(sizeof(uint64_t));
    *pts = Video::pktPts;
    pic->opaque = pts;
    return ret;
}
void Video::releaseBuffer(struct AVCodecContext *c, AVFrame *pic) {
    if(pic) av_freep(&pic->opaque);
    avcodec_default_release_buffer(c, pic);
}
#endif

void Video::lockDecodeThread()
{
    pthread_mutex_lock(&m_DecodeThreadLock);
}

void Video::unlockDecodeThread()
{
    pthread_mutex_unlock(&m_DecodeThreadLock);
}

void Video::closeFFMpeg()
{
    if (m_Opened) {
        PthreadAutoLock lock(&AVSource::m_FfmpegLock);
        avcodec_close(m_CodecCtx);
        avformat_close_input(&m_Container);
    } else {
        if (m_Container) {
            av_free(m_Container->pb);
            avformat_free_context(m_Container);
        }
    }
}

void Video::closeInternal(bool reset)
{
    this->clearTimers(reset);

    if (m_ThreadCreated) {
        this->lockDecodeThread();

        m_Shutdown = true;
        this->closeFFMpeg();

        this->unlockDecodeThread();

        NIDIUM_PTHREAD_SIGNAL(&m_BufferCond);
        pthread_join(m_ThreadDecode, NULL);
        m_ThreadCreated = false;

        m_Shutdown = false;
    } else {
        this->closeFFMpeg();
    }

    if (m_Reader) {
        delete m_Reader;
        m_Reader = NULL;
    }

    this->flushBuffers();

    for (int i = 0; i < NIDIUM_VIDEO_BUFFER_SAMPLES; i++) {
        free(m_Frames[i]);
        m_Frames[i] = NULL;
    }

    this->clearAudioQueue();
    this->clearVideoQueue();

    if (!reset) {
        delete m_AudioQueue;
        delete m_VideoQueue;

        m_AudioQueue = NULL;
        m_VideoQueue = NULL;
    }

    delete m_rBuff;

    av_free(m_ConvertedFrame);
    av_free(m_DecodedFrame);

    sws_freeContext(m_SwsCtx);

    free(m_Buff);
    free(m_Filename);

    m_rBuff          = NULL;
    m_CodecCtx       = NULL;
    m_ConvertedFrame = NULL;
    m_DecodedFrame   = NULL;
    m_SwsCtx         = NULL;
    m_Buff           = NULL;
    m_Container      = NULL;
    m_Reader         = NULL;
    m_AvioBuffer     = NULL;
    m_Filename       = NULL;

    if (m_AudioSource != NULL) {
        delete m_AudioSource;
        m_AudioSource = NULL;
    }

    m_Opened        = false;
    m_Playing       = false;
    m_SourceDoOpen  = false;
    m_PlayWhenReady = false;
}

Video::~Video()
{
    this->closeInternal(false);
}

bool VideoAudioSource::buffer()
{
    if (m_Video->m_AudioQueue->count > 0) {
        if (m_FreePacket != NULL) {
            delete m_FreePacket;
            m_FreePacket = NULL;
            // Note : av_packet_unref is called by the audioSource
        }

        Video::Packet *p = m_Video->getPacket(m_Video->m_AudioQueue);
        m_TmpPacket      = &p->curr;
        m_FreePacket     = p;
        m_PacketConsumed = false;

        return true;
    } else {
        return false;
    }
}

} // namespace AV
} // namespace Nidium
