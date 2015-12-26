#include "NativeVideo.h"

#include <stdio.h>

#include "native_netlib.h"
#include <pa_ringbuffer.h>
#include <Coro.h>
#include <NativeUtils.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


#include "NativeAudioNode.h"

#undef DPRINT
#if DEBUG && 0
  #define DEBUG_PRINT
  #define DPRINT(...) \
    printf(">%lld / ", av_gettime()/1000); \
    printf(__VA_ARGS__)
#else
  #define DPRINT(...) (void)0
#endif

// XXX : Well, NativeVideo need a better interaction with NativeAudio.
// There's a lot of little hack to work nicely with it.

NativeVideo::NativeVideo(ape_global *n)
    : m_TimerIdx(0), m_LastTimer(0),
      m_Net(n), m_AudioSource(NULL), m_FrameCbk(NULL), m_FrameCbkArg(NULL), m_Shutdown(false),
      m_FrameTimer(0), m_LastPts(0), m_VideoClock(0), m_Playing(false), m_Stopped(false),
      m_Width(0), m_Height(0), m_SwsCtx(NULL), m_CodecCtx(NULL),
      m_VideoStream(-1), m_AudioStream(-1), m_rBuff(NULL), m_Buff(NULL), m_AvioBuffer(NULL),
      m_FramesIdx(0), m_DecodedFrame(NULL), m_ConvertedFrame(NULL),
      m_Reader(NULL), m_Buffering(false), m_ThreadCreated(false), m_SourceNeedWork(false),
      m_DoSetSize(false), m_NewWidth(0), m_NewHeight(0), m_NoDisplay(false), m_InDisplay(false)
{
    NATIVE_PTHREAD_VAR_INIT(&m_BufferCond);
    NATIVE_PTHREAD_VAR_INIT(&m_NotInDisplay);

    pthread_mutex_init(&m_AudioLock, NULL);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_DecodeThreadLock, &mta);

    m_AudioQueue = new PacketQueue();
    m_VideoQueue = new PacketQueue();

    m_DoSemek = false;
    m_Seeking = false;

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        m_Timers[i] = new TimerItem();
        m_Frames[i] = NULL;
    }
}

#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
this->closeInternal(true);\
return err;

int NativeVideo::open(void *buffer, int size)
{
    if (m_Opened) {
        this->closeInternal(true);
    }

    if (!(m_AvioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Reader = new NativeAVBufferReader((uint8_t *)buffer, size);
    m_Container = avformat_alloc_context();
    if (!m_Container || !m_Reader) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container->pb = avio_alloc_context(m_AvioBuffer, NATIVE_AVIO_BUFFER_SIZE,
        0, m_Reader, NativeAVBufferReader::read, NULL, NativeAVBufferReader::seek);
    if (!m_Container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }
    if (this->openInit() == 0) {
        if (pthread_create(&m_ThreadDecode, NULL, NativeVideo::decode, this) != 0) {
            RETURN_WITH_ERROR(ERR_INTERNAL);
        }
        m_ThreadCreated = true;
        NATIVE_PTHREAD_SIGNAL(&m_BufferCond);
    }

    return 0;
}

int NativeVideo::open(const char *src)
{
    DPRINT("Open %s\n", src);
    if (m_AvioBuffer != NULL) {
        this->closeInternal(true);
    }

    m_MainCoro = Coro_new();
    m_Coro = Coro_new();
    Coro_initializeMainCoro(m_MainCoro);

    if (!(m_AvioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Reader = new NativeAVStreamReader(src, NativeVideo::sourceNeedWork, this, this, m_Net);
    m_Container = avformat_alloc_context();
    if (!m_Container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container->pb = avio_alloc_context(m_AvioBuffer, NATIVE_AVIO_BUFFER_SIZE,
        0, m_Reader, NativeAVStreamReader::read, NULL, NativeAVStreamReader::seek);
    if (!m_Container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    if (pthread_create(&m_ThreadDecode, NULL, NativeVideo::decode, this) != 0) {
        RETURN_WITH_ERROR(ERR_INTERNAL);
    }
    m_ThreadCreated = true;

    return 0;
}
#undef RETURN_WITH_ERROR

int NativeVideo::openInit()
{
    DPRINT("openInit()\n");
    m_SourceDoOpen = true;
    NATIVE_PTHREAD_SIGNAL(&m_BufferCond);
    return 0;
}

void NativeVideo::openInitCoro(void *arg)
{
    NativeVideo *thiz = (static_cast<NativeVideo*>(arg));
    int ret = thiz->openInitInternal();
    if (ret != 0) {
        thiz->sendEvent(SOURCE_EVENT_ERROR, ret, false);
        // Send a message to close the source from the main thread
        // (As we can't close a source from a coroutine/thread)
        thiz->postMessage(thiz, NativeAVSource::MSG_CLOSE);
    }
    Coro_switchTo_(thiz->m_Coro, thiz->m_MainCoro);
}

int NativeVideo::openInitInternal()
{
    // FFmpeg stuff
    AVCodec *codec;

    av_register_all();

    int ret = avformat_open_input(&m_Container, "In memory video file", NULL, NULL);

    if (ret != 0) {
        char error[1024];
        av_strerror(ret, error, 1024);
        fprintf(stderr, "Couldn't open file : %s\n", error);
        return ERR_INTERNAL;
    }

    NativePthreadAutoLock lock(&NativeAVSource::m_FfmpegLock);
    if (avformat_find_stream_info(m_Container, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information");
        return ERR_NO_INFORMATION;
    }

    av_dump_format(m_Container, 0, "Memory input", 0);

    for (unsigned int i = 0; i < m_Container->nb_streams; i++) {
        if (m_Container->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && m_VideoStream == -1) {
            m_VideoStream = i;
        } else if (m_Container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && m_AudioStream == -1) {
            m_AudioStream = i;
        }
    }

    if (m_VideoStream == -1) {
        return ERR_NO_VIDEO;
    }

    m_CodecCtx = m_Container->streams[m_VideoStream]->codec;
    //this->codecCtx->get_buffer = NativeVideo::getBuffer;
    //this->codecCtx->release_buffer = NativeVideo::releaseBuffer;

    codec = avcodec_find_decoder(m_CodecCtx->codec_id);
    if (!codec) {
        return ERR_NO_CODEC;
    }

    if (avcodec_open2(m_CodecCtx, codec, NULL) < 0) {
        fprintf(stderr, "Could not find or open the needed codec\n");
        return ERR_NO_CODEC;
    }

    // NativeAV stuff
    m_LastDelay = 40e-3; // 40ms, default delay between frames a 30fps

    // Ringbuffer that hold reference to decoded frames
    m_rBuff = new PaUtilRingBuffer();
    m_Buff = (uint8_t*) malloc(sizeof(NativeVideo::Frame) * NATIVE_VIDEO_BUFFER_SAMPLES);

    if (m_Buff == NULL) {
        fprintf(stderr, "Failed to alloc buffer\n");
        return ERR_OOM;
    }

    if (0 > PaUtil_InitializeRingBuffer(m_rBuff,
            sizeof(NativeVideo::Frame),
            NATIVE_VIDEO_BUFFER_SAMPLES,
            m_Buff)) {
        fprintf(stderr, "Failed to init ringbuffer\n");
        return ERR_OOM;
    }

    int err;
    if ((err = this->setSizeInternal()) < 0) {
        return err;
    }

    m_Opened = true;

    this->sendEvent(SOURCE_EVENT_READY, 0, false);

    return 0;
}

#undef RETURN_WITH_ERROR

void NativeVideo::frameCallback(VideoCallback cbk, void *arg) {
    m_FrameCbk = cbk;
    m_FrameCbkArg = arg;
}

void NativeVideo::play() {
    if (!m_Opened || m_Playing) {
        return;
    }

    m_Playing = true;
    m_FrameTimer = 0;

    bool haveTimer = false;
    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        if (m_Timers[i]->id == -1 && m_Timers[i]->delay != -1) {
            haveTimer = true;
            m_Timers[i]->id = this->addTimer(m_Timers[i]->delay);
        }
    }

    if (!haveTimer) {
        this->scheduleDisplay(1);
    }

    this->sendEvent(SOURCE_EVENT_PLAY, 0, false);
}

void NativeVideo::pause()
{
    m_Playing = false;

    this->clearTimers(true);

    if (m_AudioSource != NULL) {
        m_AudioSource->pause();
    }

    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void NativeVideo::close()
{
    m_Playing = false;
    m_VideoClock = 0;
    m_LastDelay = 40e-3;
    m_Error = 0;

    this->closeInternal(true);
}

void NativeVideo::stop() {
    m_Playing = false;
    m_VideoClock = 0;
    m_LastDelay = 40e-3;
    m_Error = 0;

    if (m_AudioSource != NULL) {
        m_AudioSource->stop();
    }

    this->seek(0);

    this->flushBuffers();
    this->clearTimers(true);

    NATIVE_PTHREAD_SIGNAL(&m_BufferCond);

    this->sendEvent(SOURCE_EVENT_STOP, 0, false);
}

double NativeVideo::getClock()
{
    return m_LastPts;
}

void NativeVideo::seek(double time, uint32_t flags)
{
    DPRINT("Seek called\n");
    if (!m_Opened || m_DoSemek) {
        DPRINT("not seeking cause already seeking\n");
        return;
    }

    m_DoSeekTime = time < 0 ? 0 : time;
    m_DoSemek = true;
    m_SeekFlags = flags;

    this->clearTimers(true);
    this->flushBuffers();

    NATIVE_PTHREAD_SIGNAL(&m_BufferCond);

    this->scheduleDisplay(1, true);
}

void NativeVideo::seekCoro(void *arg)
{
    NativeVideo *v = static_cast<NativeVideo *>(arg);
    v->m_Seeking = true;
    v->flushBuffers();
    v->seekInternal(v->m_DoSeekTime);
    v->m_DoSemek = false;
    v->m_Seeking = false;
    Coro_switchTo_(v->m_Coro, v->m_MainCoro);
}

bool NativeVideo::seekMethod(int64_t target, int flags)
{
    DPRINT("av_seek_frame\n");
    if (!(m_SeekFlags & NATIVE_VIDEO_SEEK_KEYFRAME)) {
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

int64_t NativeVideo::seekTarget(double time, int *flags)
{
    double clock = m_LastPts;
    int64_t target = 0;

    *flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;
    target = time * AV_TIME_BASE;

    return av_rescale_q(target, AV_TIME_BASE_Q, m_Container->streams[m_VideoStream]->time_base);
}

#define SEEK_THRESHOLD 2
#define SEEK_BUFFER_PACKET 16
#define SEEK_STEP 2
void NativeVideo::seekInternal(double time)
{
#ifdef DEBUG_PRINT
    double start = av_gettime();
#endif
    double startFrame = 0;
    int flags;
    int64_t target;
    double seekTime;
    double diff;

    int gotFrame = 0;
    double pts = 0;
    bool keyframe = false;
    bool frame = false;
    Packet *p = NULL;
    AVPacket packet;

    DPRINT("SeekInternal\n");

    if (time > this->getDuration()) {
        time = this->getDuration();
        time -= 0.1;
    }

    seekTime = time;
    diff = time - this->getClock();
    target = this->seekTarget(time, &flags);

    DPRINT("[SEEK] diff = %f, time = %lld\n", diff, av_gettime());
    if (diff > SEEK_THRESHOLD || diff <= 0 || (m_SeekFlags & NATIVE_VIDEO_SEEK_KEYFRAME)) {
        // Flush all buffers
        this->clearAudioQueue();
        this->clearVideoQueue();

        if (m_AudioSource != NULL) {
            avcodec_flush_buffers(m_AudioSource->m_CodecCtx);
            PaUtil_FlushRingBuffer(m_AudioSource->m_rBufferOut);
            m_AudioSource->resetFrames();
        }

        if (m_SeekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) {
            flags |= AVSEEK_FLAG_BACKWARD;
        }

        // Seek to desired TS
        if (!this->seekMethod(target, flags)) {
            return;
        }

        avcodec_flush_buffers(m_CodecCtx);

        if (m_SeekFlags & NATIVE_VIDEO_SEEK_KEYFRAME) {
            return;
        }
    } else {
        // Seeking forward less than SEEK_THRESHOLD
        // instead of looking for a keyframe, we just
        // read packet until we hit the desired time
        keyframe = true;
        if (m_AudioSource != NULL) {
            double tmp;
            // "in memory" seeking, we need to drop audio packet
            for (;;) {
                p = this->getPacket(m_AudioQueue);

                if (p == NULL) {
                    break;
                }

                tmp = av_q2d(m_Container->streams[m_AudioStream]->time_base) * p->curr.pts;
                DPRINT("[SEEK] Dropping audio packet @ %f\n", tmp);

                av_free_packet(&p->curr);
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
        DPRINT("[SEEK] loop gotFrame = %d pts = %f seekTime = %f time = %f\n", gotFrame, pts, seekTime, time);
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
            frame = false;
            pts = 0;
            flags = 0;
            this->clearVideoQueue();
        }

        if (m_VideoQueue->count == 0) {
            DPRINT("[SEEK] av_read_frame\n");
            int count = 0;
            while (count < SEEK_BUFFER_PACKET) {
                int err = av_read_frame(m_Container, &packet);
                if (err < 0) {
                    av_free_packet(&packet);
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
                    if ((tmp > seekTime + SEEK_STEP || tmp > time) && !keyframe) {
                        pts = tmp;
                        break;
                    }
                } else {
                    av_free_packet(&packet);
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

        DPRINT("[SEEK] reading packet stream = %d, pts = %lld/%lld\n", packet.stream_index, packet.pts, packet.dts);

        if (packet.stream_index == m_VideoStream) {
            pts = this->getPts(&packet);
            frame = true;

            DPRINT("[SEEK] got video frame at = %f flags = %d\n", pts, packet.flags);


            if ((packet.flags & AV_PKT_FLAG_KEY) || keyframe) {
                // We have our frame!
                if ((pts >= time || seekTime == 0) && gotFrame) {
                    if ((m_SeekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) && (pts != time && seekTime != 0)) {
                        // When seeking to the previous frame, we need to be at
                        // the exact frame. As it's not the case, seek backward
                        pts += 120;
                        keyframe = false;
                        continue;
                    }
                    DPRINT("[SEEK] got seek frame at = %f\n", pts);
                    Packet *tmp = m_VideoQueue->head;
                    m_VideoQueue->head = p;
                    if (tmp == NULL) {
                        m_VideoQueue->tail = p;
                    }
                    p->next = tmp;
                    m_VideoQueue->count++;

                    if (m_SeekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) {
                        this->processFrame(m_DecodedFrame);
                    }
                    break;
                }

                DPRINT("[SEEK]  its a keyframe\n");
                avcodec_decode_video2(m_CodecCtx, m_DecodedFrame, &gotFrame, &packet);
                if (gotFrame) {
                    DPRINT("[SEEK] ==== GOT FRAME\n");
                    if (!keyframe) {
                        startFrame = av_gettime()/1000;
                    }
                }
                keyframe = true;
            }
        }

        av_free_packet(&packet);
        if (p != NULL) {
            delete p;
            p = NULL;
        }
    }

    m_FrameTimer = 0; // frameTimer will be init at next display
    m_LastPts = time;
    m_DoSemek = false;


#ifdef DEBUG_PRINT
    double end = av_gettime();
    DPRINT("[SEEK] seek took %f / firstFrame to end = %f \n", end-start, end-startFrame);
    DPRINT("Sending seekCond signal\n");
#endif
    this->processVideo();
    NATIVE_PTHREAD_SIGNAL(&m_BufferCond);
}
#undef SEEK_THRESHOLD
#undef SEEK_STEP
#undef SEEK_BUFFER_PACKET

void NativeVideo::nextFrame()
{
    if (m_Playing) {
        return;
    }
    this->scheduleDisplay(1, true);
}

void NativeVideo::prevFrame()
{
    if (m_Playing) {
        return;
    }
    this->seek(this->getClock(), NATIVE_VIDEO_SEEK_PREVIOUS);
}

void NativeVideo::frameAt(double time, bool keyframe)
{
    if (m_Playing) {
        return;
    }

    this->seek(time, keyframe ? NATIVE_VIDEO_SEEK_KEYFRAME : 0);
}

NativeAudioSource *NativeVideo::getAudioNode(NativeAudio *audio)
{

    if (m_AudioSource) {
        return m_AudioSource;
    }

    m_Audio = audio;

    if (m_AudioStream != -1 && audio) {
        m_AudioSource = new NativeVideoAudioSource(2, this, true);
        audio->addSource(m_AudioSource, true);
        m_AudioSource->m_AudioStream = m_AudioStream;
        m_AudioSource->m_Container = m_Container;
        m_AudioSource->eventCallback(NULL, NULL); //Disable events callbacks
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

int NativeVideo::display(void *custom) {
    NativeVideo *v = static_cast<NativeVideo *>(custom);

    // Set pthread cond to false, so
    // setSize will always wait for the signal
    v->m_NotInDisplayCond = false;

    v->m_InDisplay = true;

    DPRINT("[DISPLAY] %d\n", v->m_NoDisplay);

    // Reset timer from queue
    v->m_Timers[v->m_LastTimer]->id = -1;
    v->m_Timers[v->m_LastTimer]->delay = -1;

    v->m_LastTimer++;
    if (v->m_LastTimer > NATIVE_VIDEO_BUFFER_SAMPLES-1) {
        v->m_LastTimer = 0;
    }

    if (v->m_NoDisplay) {
        // Frames are going to be resized.
        // Don't do anything
        v->scheduleDisplay(1, true);
        v->m_InDisplay = false;
        NATIVE_PTHREAD_SIGNAL(&v->m_NotInDisplay);
        return 0;
    }

    // Read frame from ring buffer
    if (PaUtil_GetRingBufferReadAvailable(v->m_rBuff) < 1) {
        if (!v->m_Buffering && !v->m_Seeking) {
            NATIVE_PTHREAD_SIGNAL(&v->m_BufferCond);
        }
        if (v->m_Eof) {
            DPRINT("No frame, eof reached\n");
            v->sendEvent(SOURCE_EVENT_EOF, 0, false);
        } else {
            DPRINT("No frame, try again in 20ms\n");
            v->scheduleDisplay(1, true);
        }

        v->m_InDisplay = false;
        NATIVE_PTHREAD_SIGNAL(&v->m_NotInDisplay);

        return 0;
    }

    Frame frame;
    PaUtil_ReadRingBuffer(v->m_rBuff, (void *)&frame, 1);

    double pts = frame.pts;
    double diff = 0;
    double delay, actualDelay, syncThreshold;

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
    v->m_LastPts = pts;

    if (v->m_AudioSource != NULL && v->m_AudioSource->m_IsConnected) {
        diff = pts - v->m_AudioSource->getClock();

        DPRINT("Clocks audio=%f / video=%f / diff = %f\n", v->m_AudioSource->getClock(), pts, diff);

        if (diff > NATIVE_VIDEO_AUDIO_SYNC_THRESHOLD && v->m_AudioSource->avail() > 0) {
            // Diff is too big an will be noticed
            // Let's drop some audio sample
            DPRINT("Dropping audio before=%f ", diff);
            diff -= v->m_AudioSource->drop(diff);
            DPRINT("after=%f sec)\n", diff);
        } else {
            syncThreshold = (delay > NATIVE_VIDEO_SYNC_THRESHOLD) ? delay : NATIVE_VIDEO_SYNC_THRESHOLD;

            if (fabs(diff) < NATIVE_VIDEO_NOSYNC_THRESHOLD) {
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

    DPRINT("frameTimer=%f delay=%f videoClock=%f\n", v->m_FrameTimer, delay, pts);
    if (delay < 0) delay = 0;
    v->m_FrameTimer += delay * 1000;
    actualDelay = (v->m_FrameTimer - (av_gettime() / 1000.0)) / 1000;

    DPRINT("Using delay %f frameTimer=%f delay=%f\n", actualDelay, v->m_FrameTimer, delay);

    if (v->m_Playing) {
        if (actualDelay <= NATIVE_VIDEO_SYNC_THRESHOLD && diff <= 0) {
            DPRINT("Droping video frame\n");
            v->display(v);
            v->m_InDisplay = false;
            NATIVE_PTHREAD_SIGNAL(&v->m_NotInDisplay);
            return 0;
        } else {
            DPRINT("Next display in %d\n", (int)(actualDelay * 1000));
            v->scheduleDisplay(((int)(actualDelay * 1000)));
        }
    }

    if (actualDelay > NATIVE_VIDEO_SYNC_THRESHOLD || diff > 0 || !v->m_Playing) {
        // If not playing, we can be here because user seeked
        // while the video is paused, so send the frame anyway

        // Call the frame callback
        if (v->m_FrameCbk != NULL) {
            v->m_FrameCbk(frame.data, v->m_FrameCbkArg);
        }
    }

    if (v->m_Playing) {
        NATIVE_PTHREAD_SIGNAL(&v->m_BufferCond);
    }

    v->m_InDisplay = false;
    NATIVE_PTHREAD_SIGNAL(&v->m_NotInDisplay);

    return 0;
}

int NativeVideo::setSizeInternal()
{
    m_NoDisplay = true;

    if (m_InDisplay) {
        NATIVE_PTHREAD_WAIT(&m_NotInDisplay);
    }

    // Flush buffers & timers to discard old frames
    this->flushBuffers();
    //this->clearTimers(true);

    int width = m_NewWidth == 0 ? m_CodecCtx->width : m_NewWidth;
    int height = m_NewHeight == 0 ? m_CodecCtx->height : m_NewHeight;

    if (!m_SwsCtx) {
        // First call to setSizeInternal, init frames
        m_DecodedFrame = av_frame_alloc();
        m_ConvertedFrame = av_frame_alloc();

        if (m_DecodedFrame == NULL || m_ConvertedFrame == NULL) {
            fprintf(stderr, "Failed to alloc frame\n");
            m_NoDisplay = false;
            return ERR_OOM;
        }
    } else {
        sws_freeContext(m_SwsCtx);
    }

    m_SwsCtx = sws_getContext(m_CodecCtx->width, m_CodecCtx->height, m_CodecCtx->pix_fmt,
                                  width, height, PIX_FMT_RGBA,
                                  SWS_BICUBIC, NULL, NULL, NULL);

    if (!m_SwsCtx) {
        m_NoDisplay = false;
        return ERR_NO_VIDEO_CONVERTER;
    }

    // Update the size of the frames in the frame pool
    int frameSize = avpicture_fill((AVPicture *)m_ConvertedFrame, NULL, PIX_FMT_RGBA, width, height);
    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        free(m_Frames[i]);
        m_Frames[i] = (uint8_t*) malloc(frameSize);
    }

    printf("Setting size finished\n");

    m_Width = width;
    m_Height = height;

    m_NoDisplay = false;
    return 0;
#if 0
    // XXX : Nop
    // Determine required frameSize and allocate buffer frame pool
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(m_rBuff);

    int idx = m_FramesIdx;
    uint8_t *data;
    for (ring_buffer_size_t i = 0; i < avail; i++) {
        Frame frame;

        data = (uint8_t*) malloc(frameSize);
        PaUtil_ReadRingBuffer(m_rBuff, (void *)&frame, 1);

        this->convertFrame(&frame, data);

        free(m_Frames[m_FramesIdx].data);
        m_Frames[m_FramesIdx].data = data;

        PaUtil_WriteRingBuffer(m_rBuff, &frame, 1);

        if (idx == NATIVE_VIDEO_BUFFER_SAMPLES - 1) {
            idx = 0;
        } else {
            idx++;
        }
    }

    m_FramesIdx = idx;
#endif
}

void NativeVideo::setSize(int width, int height)
{
    if (width == m_Width && height == m_Height && m_SwsCtx) {
        return;
    }

    m_NewWidth = width;
    m_NewHeight = height;

    m_DoSetSize = true;

    //NATIVE_PTHREAD_SIGNAL(&this->bufferCond);
}

void NativeVideo::buffer()
{
    if (m_Error != 0) {
        DPRINT("=> Not buffering cause of error\n");
        return;
    }

    if (m_Reader->m_Async) {
        if (m_Buffering) {
            DPRINT("=> PENDING\n");
            // Reader already trying to get data
            return;
        }
        m_Buffering = true;
        DPRINT("buffer coro start\n");
        Coro_startCoro_(m_MainCoro, m_Coro, this, NativeVideo::bufferCoro);
    } else {
        this->bufferInternal();
    }
}

void NativeVideo::bufferCoro(void *arg)
{
   NativeVideo *v = static_cast<NativeVideo*>(arg);
   v->bufferInternal();

    if (!v->m_Reader->m_Pending) {
        v->m_Buffering = false;
    }

    Coro_switchTo_(v->m_Coro, v->m_MainCoro);

    return;
}

void NativeVideo::bufferInternal()
{
    DPRINT("hello buffer internal\n");
    AVPacket packet;

    bool loopCond = false;

    int needAudio = 0;
    int needVideo = 0;

    if (m_Playing) {
        if (m_AudioSource != NULL && m_AudioSource->m_IsConnected) {
            needAudio = NATIVE_VIDEO_PACKET_BUFFER - m_AudioQueue->count;
        }
        needVideo = NATIVE_VIDEO_PACKET_BUFFER - m_VideoQueue->count;
    } else {
        needVideo = 1;
    }


    if (needAudio > 0 || needVideo > 0) {
        loopCond = true;
    }

    while (loopCond) {
        DPRINT("    => buffering loop needAudio=%d / needVideo=%d\n", needAudio, needVideo);
        int ret = av_read_frame(m_Container, &packet);
        DPRINT("    -> post read frame\n");

        // If a seek is asked while buffering. Return.
        if (m_DoSemek) {
            av_free_packet(&packet);
            break;
        }

        // Got a read error. Return.
        if (ret < 0) {
            av_free_packet(&packet);
            if (this->readError(ret) != 0) {
                printf("got error %d/%d\n", m_Eof, m_Error);
                return;
            }
            continue;
        }

        if (packet.stream_index == m_VideoStream) {
            this->addPacket(m_VideoQueue, &packet);
            needVideo--;
        } else if (packet.stream_index == m_AudioStream &&
            ((m_AudioSource != NULL && m_AudioSource->m_IsConnected) ||
              this->getClock() == 0)) {
                this->addPacket(m_AudioQueue, &packet);
                needAudio--;
        } else {
            av_free_packet(&packet);
        }

        if ((needVideo <= 0 && needAudio <= 0) || m_Error != 0) {
            break;
        }
    }

}

void *NativeVideo::decode(void *args)
{
    NativeVideo *v = static_cast<NativeVideo *>(args);

    for (;;) {
        DPRINT("decode loop\n");
        v->lockDecodeThread();

        if (v->m_Shutdown) break;

        if (v->m_Opened) {
            DPRINT("opened buffering=%d\n", v->m_Buffering);
            if (!v->m_Buffering && !v->m_Seeking) {
                DPRINT("not buffering and seeking\n");
                if (v->m_DoSemek == true) {
                    DPRINT("seeking\n");
                    if (!v->m_Reader->m_Async) {
                        v->seekInternal(v->m_DoSeekTime);
                        v->m_DoSemek = false;
                    } else {
                        DPRINT("    running seek coro\n");
                        Coro_startCoro_(v->m_MainCoro, v->m_Coro, v, NativeVideo::seekCoro);
                    }
                    DPRINT("done seeking\n");
                } else {
                    DPRINT("buffering\n");
                    //DPRINT("Coro space main = %d coro = %d\n",
                    //    Coro_stackSpaceAlmostGone(v->mainCoro), Coro_stackSpaceAlmostGone(v->coro));
                    v->buffer();
                }
            }

            DPRINT("doSeek=%d readFlag=%d seeking=%d\n", v->m_DoSemek, v->m_SourceNeedWork, v->m_Seeking);
            if (!v->m_DoSemek) {
                DPRINT("processing\n");
#if defined(DPRINT) && 0
                bool videoFailed = !v->processVideo();
                bool audioFailed = !v->processAudio();
                DPRINT("audioFailed=%d videoFailed=%d\n", audioFailed, videoFailed);
#endif
            }
        } else if (v->m_SourceDoOpen) {
            v->m_SourceDoOpen = false;
            if (v->m_Reader->m_Async) {
                Coro_startCoro_(v->m_MainCoro, v->m_Coro, v, NativeVideo::openInitCoro);
            } else {
                v->openInitInternal();
            }
        }

        v->unlockDecodeThread();

        if (v->m_Shutdown) break;

        if (v->m_SourceNeedWork) {
            DPRINT("readFlag, swithcing back to coro\n");
            v->lockDecodeThread();
            v->m_SourceNeedWork = false;
            Coro_switchTo_(v->m_MainCoro, v->m_Coro);
            // Make sure another read call havn't been made
            if (!v->m_Reader->m_Pending) {
                v->m_Buffering = false;
            }
            v->unlockDecodeThread();
        } else if (!v->m_DoSemek) {
            DPRINT("wait bufferCond, no work needed\n");
            NATIVE_PTHREAD_WAIT(&v->m_BufferCond);
            DPRINT("Waked up from bufferCond!");
        }
    }

    v->m_Shutdown = false;

    return NULL;
}

void NativeVideo::stopAudio()
{
    NativePthreadAutoLock lock(&m_AudioLock);

    this->clearAudioQueue();

    m_Audio = NULL;
    // NativeVideo::stopAudio() is called when the audio node
    // is being destructed, so we just set the audioSource to null
    m_AudioSource = NULL;
}

void NativeVideo::sourceNeedWork(void *ptr)
{
    NativeVideo *thiz = static_cast<NativeVideo*>(ptr);
    thiz->m_SourceNeedWork = true;
    NATIVE_PTHREAD_SIGNAL(&thiz->m_BufferCond);
}

bool NativeVideo::processAudio()
{
    NativePthreadAutoLock lock(&m_AudioLock);
    DPRINT("processing audio\n");

    if (m_AudioSource == NULL) {
        return false;
    }

    if (!m_AudioSource->m_IsConnected) {
        return false;
    }

    while (m_AudioSource->work()) {}

    // TODO : We should wakup the thread only
    // if source had processed data
    if (m_Audio->canWriteFrame()) {
        DPRINT("Wakeup thread\n");
        NATIVE_PTHREAD_SIGNAL(&m_Audio->m_QueueHaveData);
    }

    return true;
}

bool NativeVideo::processVideo()
{
    if (PaUtil_GetRingBufferWriteAvailable(m_rBuff) < 1) {
        DPRINT("processVideo not enought space to write data\n");
        return false;
    }

    if (m_DoSetSize) {
        m_DoSetSize = false;
        int ret = this->setSizeInternal();

        if (ret < 0) {
            return false;
        }
    }

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

    avcodec_decode_video2(m_CodecCtx, m_DecodedFrame, &gotFrame, &packet);

    if (gotFrame) {
        this->processFrame(m_DecodedFrame);
    }

    delete p;
    av_free_packet(&packet);

    return true;
}

bool NativeVideo::processFrame(AVFrame *avFrame)
{
    Frame frame;
    frame.data = m_Frames[m_FramesIdx];
    frame.pts = this->getPts(avFrame);

    bool ret = this->convertFrame(avFrame, (uint8_t*) frame.data);

    // Write frame to rBuff, the frame will be consumed
    // later by NativeVideo::display() (UI Thread)
    PaUtil_WriteRingBuffer(m_rBuff, &frame, 1);

    if (m_FramesIdx == NATIVE_VIDEO_BUFFER_SAMPLES - 1) {
        m_FramesIdx = 0;
    } else {
        m_FramesIdx++;
    }

    return ret;
}
bool NativeVideo::convertFrame(AVFrame *avFrame, uint8_t *dst)
{
    // Format the frame for sws_scale
    uint8_t *tmp[1];
    tmp[0] = (uint8_t *)dst;
    uint8_t * const *out = (uint8_t * const*)tmp;

    // Convert the image from its native format to RGBA
    // TODO : Move this to a shader
    sws_scale(m_SwsCtx,
              avFrame->data, avFrame->linesize,
              0, m_CodecCtx->height, out, m_ConvertedFrame->linesize);

    return true;
}

double NativeVideo::syncVideo(double pts)
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

double NativeVideo::getPts(AVFrame *frame)
{
    double pts;

    pts = av_frame_get_best_effort_timestamp(frame);

    return this->syncVideo(pts) * av_q2d(m_Container->streams[m_VideoStream]->time_base);
}

double NativeVideo::getPts(AVPacket *packet)
{
    double pts;

    if (packet->pts != AV_NOPTS_VALUE) {
        pts = packet->pts;
    } else if (packet->dts != AV_NOPTS_VALUE) {
        pts = packet->dts;
    } else {
        pts = 0;
    }

    return this->syncVideo(pts) * av_q2d(m_Container->streams[m_VideoStream]->time_base);
}

void NativeVideo::scheduleDisplay(int delay)
{
    this->scheduleDisplay(delay, false);
}

void NativeVideo::scheduleDisplay(int delay, bool force) {
    m_Timers[m_TimerIdx]->delay = delay;
    m_Timers[m_TimerIdx]->id = -1;

    DPRINT("scheduleDisplay in %d\n", delay);

    if (m_Playing || force) {
        m_Timers[m_TimerIdx]->id = this->addTimer(delay);
    }

    m_TimerIdx++;

    if (m_TimerIdx > NATIVE_VIDEO_BUFFER_SAMPLES-1) {
        m_TimerIdx = 0;
    }
}

int NativeVideo::addTimer(int delay)
{
    ape_timer *timer;
    timer = add_timer(&m_Net->timersng, delay, NativeVideo::display, this);
    //timer->flags &= ~APE_TIMER_IS_PROTECTED; // XXX : Remove me
    return timer->identifier;
}

bool NativeVideo::addPacket(PacketQueue *queue, AVPacket *packet)
{
    av_dup_packet(packet);

    Packet *pkt = new Packet();
    pkt->curr = *packet;

    if (!queue->tail) {
        queue->head = pkt;
    } else {
        queue->tail->next = pkt;
    }

    queue->tail = pkt;
    queue->count++;

    return true;
}

NativeVideo::Packet *NativeVideo::getPacket(PacketQueue *queue)
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

void NativeVideo::clearTimers(bool reset)
{
    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        if (m_Timers[i]->id != -1 && m_Timers[i]->delay != -1) {
            clear_timer_by_id(&m_Net->timersng, m_Timers[i]->id, 1);
        }
        if (reset) {
            m_Timers[i]->id = -1;
            m_Timers[i]->delay = -1;
        } else {
            delete m_Timers[i];
            m_Timers[i] = NULL;
        }
    }
}

void NativeVideo::clearAudioQueue()
{
    /*
    if (this->freePacket != NULL) {
        if (this->audioSource != NULL) {
            this->audioSource->packetConsumed = true;
        }
        av_free_packet(&this->freePacket->curr);
        delete this->freePacket;
        this->freePacket = NULL;
    }
    */

    Packet *pkt = m_AudioQueue->head;
    Packet *next;
    while (pkt != NULL) {
        next = pkt->next;
        av_free_packet(&pkt->curr);
        delete pkt;
        pkt = next;
    }
    m_AudioQueue->head = NULL;
    m_AudioQueue->tail = NULL;
    m_AudioQueue->count = 0;
}

void NativeVideo::clearVideoQueue()
{
    Packet *pkt = m_VideoQueue->head;
    Packet *next;
    while (pkt != NULL) {
        next = pkt->next;
        av_free_packet(&pkt->curr);
        delete pkt;
        pkt = next;
    }
    m_VideoQueue->head = NULL;
    m_VideoQueue->tail = NULL;
    m_VideoQueue->count = 0;
}

void NativeVideo::flushBuffers()
{
    if (m_rBuff == NULL) return;

    m_FramesIdx = 0;

    PaUtil_FlushRingBuffer(m_rBuff);
}

#if 0
int NativeVideo::getBuffer(struct AVCodecContext *c, AVFrame *pic) {
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t *)av_malloc(sizeof(uint64_t));
    *pts = NativeVideo::pktPts;
    pic->opaque = pts;
    return ret;
}
void NativeVideo::releaseBuffer(struct AVCodecContext *c, AVFrame *pic) {
    if(pic) av_freep(&pic->opaque);
    avcodec_default_release_buffer(c, pic);
}
#endif

void NativeVideo::lockDecodeThread()
{
    pthread_mutex_lock(&m_DecodeThreadLock);
}

void NativeVideo::unlockDecodeThread()
{
    pthread_mutex_unlock(&m_DecodeThreadLock);
}

void NativeVideo::closeFFMpeg()
{
    if (m_Opened) {
        NativePthreadAutoLock lock(&NativeAVSource::m_FfmpegLock);
        avcodec_close(m_CodecCtx);
        av_free(m_Container->pb);
        avformat_close_input(&m_Container);
    } else {
        if (m_Container) {
            av_free(m_Container->pb);
            avformat_free_context(m_Container);
        }
    }
}

void NativeVideo::closeInternal(bool reset)
{
    this->clearTimers(reset);

    if (m_ThreadCreated) {
        // Finish the reader first, any pending call
        // will be finished (seek, read) so we can lock the thread
        m_Reader->finish();

        this->lockDecodeThread();

        m_Shutdown = true;
        this->closeFFMpeg();

        this->unlockDecodeThread();

        NATIVE_PTHREAD_SIGNAL(&m_BufferCond);
        pthread_join(m_ThreadDecode, NULL);
        m_ThreadCreated = false;

        m_Shutdown = false;
    } else {
        this->closeFFMpeg();
    }

    delete m_Reader;
    m_Reader = NULL;

    this->flushBuffers();

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
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

    if (m_MainCoro != NULL) {
        Coro_free(m_MainCoro);
        Coro_free(m_Coro);
        m_MainCoro = NULL;
        m_Coro = NULL;
    }

    delete m_rBuff;

    av_free(m_ConvertedFrame);
    av_free(m_DecodedFrame);

    sws_freeContext(m_SwsCtx);

    free(m_Buff);

    m_rBuff = NULL;
    m_CodecCtx = NULL;
    m_ConvertedFrame = NULL;
    m_DecodedFrame = NULL;
    m_SwsCtx = NULL;
    m_Buff = NULL;
    m_Container = NULL;
    m_Reader = NULL;
    m_AvioBuffer = NULL;

    if (m_AudioSource != NULL) {
        delete m_AudioSource;
        m_AudioSource = NULL;
    }

    m_Opened = false;
    m_SourceDoOpen = false;
}

NativeVideo::~NativeVideo() {
    this->closeInternal(false);
}

bool NativeVideoAudioSource::buffer()
{
    if (m_Video->m_AudioQueue->count > 0) {
        if (m_FreePacket != NULL) {
            delete m_FreePacket;
            m_FreePacket = NULL;
            // Note : av_free_packet is called by the audioSource
        }

        NativeVideo::Packet *p = m_Video->getPacket(m_Video->m_AudioQueue);
        m_TmpPacket = &p->curr;
        m_FreePacket = p;
        m_PacketConsumed = false;

        return true;
    } else {
        return false;
    }
}

