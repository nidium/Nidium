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
#if 0
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
    : timerIdx(0), lastTimer(0),
      net(n), audioSource(NULL), frameCbk(NULL), frameCbkArg(NULL), shutdown(false),
      frameTimer(0), lastPts(0), videoClock(0), playing(false), stoped(false),
      m_Width(0), m_Height(0), swsCtx(NULL), codecCtx(NULL),
      videoStream(-1), audioStream(-1), rBuff(NULL), buff(NULL), avioBuffer(NULL),
      m_FramesIdx(NULL), decodedFrame(NULL), convertedFrame(NULL),
      reader(NULL), buffering(false), m_ThreadCreated(false), m_SourceNeedWork(false),
      m_DoSetSize(false), m_NewWidth(0), m_NewHeight(0), m_NoDisplay(false), m_InDisplay(false)
{
    NATIVE_PTHREAD_VAR_INIT(&this->bufferCond);
    NATIVE_PTHREAD_VAR_INIT(&this->m_NotInDisplay);

    pthread_mutex_init(&this->audioLock, NULL);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&this->decodeThreadLock, &mta);

    this->audioQueue = new PacketQueue();
    this->videoQueue = new PacketQueue();

    this->doSeek = false;
    this->seeking = false;

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        this->timers[i] = new TimerItem();
        m_Frames[i] = NULL;
    }
}

#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
this->closeInternal(true);\
return err;

int NativeVideo::open(void *buffer, int size)
{
    if (this->opened) {
        this->closeInternal(true);
    }

    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->reader = new NativeAVBufferReader((uint8_t *)buffer, size);
    this->container = avformat_alloc_context();
    if (!this->container || !this->reader) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE,
        0, this->reader, NativeAVBufferReader::read, NULL, NativeAVBufferReader::seek);
    if (!this->container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }
    if (this->openInit() == 0) {
        if (pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this) != 0) {
            RETURN_WITH_ERROR(ERR_INTERNAL);
        }
        m_ThreadCreated = true;
        NATIVE_PTHREAD_SIGNAL(&this->bufferCond);
    }

    return 0;
}

int NativeVideo::open(const char *src)
{
    DPRINT("Open %s\n", src);
    if (this->avioBuffer != NULL) {
        this->closeInternal(true);
    }

    this->mainCoro = Coro_new();
    this->coro = Coro_new();
    Coro_initializeMainCoro(this->mainCoro);

    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->reader = new NativeAVStreamReader(src, NativeVideo::sourceNeedWork, this, this, this->net);
    this->container = avformat_alloc_context();
    if (!this->container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE,
        0, this->reader, NativeAVStreamReader::read, NULL, NativeAVStreamReader::seek);
    if (!this->container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    if (pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this) != 0) {
        RETURN_WITH_ERROR(ERR_INTERNAL);
    }
    m_ThreadCreated = true;

    return 0;
}
#undef RETURN_WITH_ERROR

int NativeVideo::openInit()
{
    DPRINT("openInit()\n");
    this->m_SourceDoOpen = true;
    NATIVE_PTHREAD_SIGNAL(&this->bufferCond);
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
    Coro_switchTo_(thiz->coro, thiz->mainCoro);
}

int NativeVideo::openInitInternal()
{
    // FFmpeg stuff
    AVCodec *codec;

    av_register_all();

    int ret = avformat_open_input(&this->container, "In memory video file", NULL, NULL);

    if (ret != 0) {
        char error[1024];
        av_strerror(ret, error, 1024);
        fprintf(stderr, "Couldn't open file : %s\n", error);
        return ERR_INTERNAL;
    }

    NativePthreadAutoLock lock(&NativeAVSource::ffmpegLock);
    if (avformat_find_stream_info(this->container, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information");
        return ERR_NO_INFORMATION;
    }

    av_dump_format(this->container, 0, "Memory input", 0);

    for (unsigned int i = 0; i < this->container->nb_streams; i++) {
        if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && this->videoStream == -1) {
            this->videoStream = i;
        } else if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && this->audioStream == -1) {
            this->audioStream = i;
        }
    }

    if (this->videoStream == -1) {
        return ERR_NO_VIDEO;
    }

    this->codecCtx = this->container->streams[this->videoStream]->codec;
    //this->codecCtx->get_buffer = NativeVideo::getBuffer;
    //this->codecCtx->release_buffer = NativeVideo::releaseBuffer;

    codec = avcodec_find_decoder(this->codecCtx->codec_id);
    if (!codec) {
        return ERR_NO_CODEC;
    }

    if (avcodec_open2(this->codecCtx, codec, NULL) < 0) {
        fprintf(stderr, "Could not find or open the needed codec\n");
        return ERR_NO_CODEC;
    }

    // NativeAV stuff
    this->lastDelay = 40e-3; // 40ms, default delay between frames a 30fps

    // Ringbuffer that hold reference to decoded frames
    this->rBuff = new PaUtilRingBuffer();
    this->buff = (uint8_t*) malloc(sizeof(NativeVideo::Frame) * NATIVE_VIDEO_BUFFER_SAMPLES);

    if (this->buff == NULL) {
        fprintf(stderr, "Failed to alloc buffer\n");
        return ERR_OOM;
    }

    if (0 > PaUtil_InitializeRingBuffer(this->rBuff,
            sizeof(NativeVideo::Frame),
            NATIVE_VIDEO_BUFFER_SAMPLES,
            this->buff)) {
        fprintf(stderr, "Failed to init ringbuffer\n");
        return ERR_OOM;
    }

    int err;
    if ((err = this->setSizeInternal()) < 0) {
        return err;
    }

    this->opened = true;

    this->sendEvent(SOURCE_EVENT_READY, 0, false);

    return 0;
}

#undef RETURN_WITH_ERROR

void NativeVideo::frameCallback(VideoCallback cbk, void *arg) {
    this->frameCbk = cbk;
    this->frameCbkArg = arg;
}

void NativeVideo::play() {
    if (!this->opened || this->playing) {
        return;
    }

    this->playing = true;
    this->frameTimer = 0;

    bool haveTimer = false;
    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        if (this->timers[i]->id == -1 && this->timers[i]->delay != -1) {
            haveTimer = true;
            this->timers[i]->id = this->addTimer(this->timers[i]->delay);
        }
    }

    if (!haveTimer) {
        this->scheduleDisplay(1);
    }

    this->sendEvent(SOURCE_EVENT_PLAY, 0, false);
}

void NativeVideo::pause()
{
    this->playing = false;

    this->clearTimers(true);

    if (this->audioSource != NULL) {
        this->audioSource->pause();
    }

    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void NativeVideo::close()
{
    this->playing = false;
    this->videoClock = 0;
    this->lastDelay = 40e-3;
    this->error = 0;

    this->closeInternal(true);
}

void NativeVideo::stop() {
    this->playing = false;
    this->videoClock = 0;
    this->lastDelay = 40e-3;
    this->error = 0;

    if (this->audioSource != NULL) {
        this->audioSource->stop();
    }

    this->seek(0);

    this->flushBuffers();
    this->clearTimers(true);

    NATIVE_PTHREAD_SIGNAL(&this->bufferCond);

    this->sendEvent(SOURCE_EVENT_STOP, 0, false);
}

double NativeVideo::getClock()
{
    return this->lastPts;
}

void NativeVideo::seek(double time, uint32_t flags)
{
    DPRINT("Seek called\n");
    if (!this->opened || this->doSeek) {
        DPRINT("not seeking cause already seeking\n");
        return;
    }

    this->doSeekTime = time < 0 ? 0 : time;
    this->doSeek = true;
    this->seekFlags = flags;

    this->clearTimers(true);
    this->flushBuffers();

    NATIVE_PTHREAD_SIGNAL(&this->bufferCond);

    this->scheduleDisplay(1, true);
}

void NativeVideo::seekCoro(void *arg)
{
    NativeVideo *v = static_cast<NativeVideo *>(arg);
    v->seeking = true;
    v->flushBuffers();
    v->seekInternal(v->doSeekTime);
    v->doSeek = false;
    v->seeking = false;
    Coro_switchTo_(v->coro, v->mainCoro);
}

bool NativeVideo::seekMethod(int64_t target, int flags)
{
    DPRINT("av_seek_frame\n");
    if (!(this->seekFlags & NATIVE_VIDEO_SEEK_KEYFRAME)) {
        flags |= AVSEEK_FLAG_ANY;
    }
    int ret = av_seek_frame(this->container, this->videoStream, target, flags);
    DPRINT("av_seek_frame done ret=%d\n", ret);
    if (ret >= 0) {
        this->error = 0;

        // FFMPEG can success seeking even if seek is after the end
        // (but decoder/demuxer fail). So fix that.
        if (this->doSeekTime > this->getDuration()) {
            this->lastPts = this->getDuration();
        } else {
            this->lastPts = this->doSeekTime;
        }

        // We need to update the videoClock in case of seek @ 0
        // because syncVideo() will get a packet with PTS @ 0
        // and it will not update the clock
        if (target == 0) {
            this->videoClock = 0;
        }

        return true;
    } else {
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
        return false;
    }
}

int64_t NativeVideo::seekTarget(double time, int *flags)
{
    double clock = this->lastPts;
    int64_t target = 0;

    *flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;
    target = time * AV_TIME_BASE;

    return av_rescale_q(target, AV_TIME_BASE_Q, this->container->streams[this->videoStream]->time_base);
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
    if (diff > SEEK_THRESHOLD || diff <= 0 || (this->seekFlags & NATIVE_VIDEO_SEEK_KEYFRAME)) {
        // Flush all buffers
        this->clearAudioQueue();
        this->clearVideoQueue();

        if (this->audioSource != NULL) {
            avcodec_flush_buffers(this->audioSource->codecCtx);
            PaUtil_FlushRingBuffer(this->audioSource->rBufferOut);
            this->audioSource->resetFrames();
        }

        if (this->seekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) {
            flags |= AVSEEK_FLAG_BACKWARD;
        }

        // Seek to desired TS
        if (!this->seekMethod(target, flags)) {
            return;
        }

        avcodec_flush_buffers(this->codecCtx);

        if (this->seekFlags & NATIVE_VIDEO_SEEK_KEYFRAME) {
            return;
        }
    } else {
        // Seeking forward less than SEEK_THRESHOLD
        // instead of looking for a keyframe, we just
        // read packet until we hit the desired time
        keyframe = true;
        if (audioSource != NULL) {
            double tmp;
            // "in memory" seeking, we need to drop audio packet
            for (;;) {
                p = this->getPacket(this->audioQueue);

                if (p == NULL) {
                    break;
                }

                tmp = av_q2d(this->container->streams[this->audioStream]->time_base) * p->curr.pts;
                DPRINT("[SEEK] Dropping audio packet @ %f\n", tmp);

                av_free_packet(&p->curr);
                delete p;

                if (tmp >= time) {
                    break;
                }
            }
        }
    }

    if (this->eof && flags == AVSEEK_FLAG_BACKWARD) {
        this->eof = false;
        if (this->audioSource != NULL) {
            this->audioSource->eof = false;
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
            avcodec_flush_buffers(this->codecCtx);
            keyframe = false;
            frame = false;
            pts = 0;
            flags = 0;
            this->clearVideoQueue();
        }

        if (this->videoQueue->count == 0) {
            DPRINT("[SEEK] av_read_frame\n");
            int count = 0;
            while (count < SEEK_BUFFER_PACKET) {
                int err = av_read_frame(this->container, &packet);
                if (err < 0) {
                    av_free_packet(&packet);
                    err = this->readError(err);
                    if (err == AVERROR_EOF && this->videoQueue->count > 0) {
                        break;
                    } else if (err < 0) {
                        DPRINT("[SEEK] Got fatal error %d\n", err);
                        return;
                    }
                    continue;
                }
                if (packet.stream_index == this->videoStream) {
                    double tmp = this->getPts(&packet);
                    DPRINT("[SEEK] Got packet @ %f\n", tmp);
                    this->addPacket(this->videoQueue, &packet);
                    count++;
                    if ((tmp > seekTime + SEEK_STEP || tmp > time) && !keyframe) {
                        pts = tmp;
                        break;
                    }
                } else {
                    av_free_packet(&packet);
                }
            }
            p = this->getPacket(this->videoQueue);
            if (p == NULL) {
                continue;
            }
            packet = p->curr;
        } else {
            DPRINT("[SEEK] getPacket\n");
            p = this->getPacket(this->videoQueue);
            if (p == NULL) {
                continue;
            }
            packet = p->curr;
        }

        DPRINT("[SEEK] reading packet stream = %d, pts = %lld/%lld\n", packet.stream_index, packet.pts, packet.dts);

        if (packet.stream_index == this->videoStream) {
            pts = this->getPts(&packet);
            frame = true;

            DPRINT("[SEEK] got video frame at = %f flags = %d\n", pts, packet.flags);


            if ((packet.flags & AV_PKT_FLAG_KEY) || keyframe) {
                // We have our frame!
                if ((pts >= time || seekTime == 0) && gotFrame) {
                    if ((this->seekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) && (pts != time && seekTime != 0)) {
                        // When seeking to the previous frame, we need to be at
                        // the exact frame. As it's not the case, seek backward
                        pts += 120;
                        keyframe = false;
                        continue;
                    }
                    DPRINT("[SEEK] got seek frame at = %f\n", pts);
                    Packet *tmp = this->videoQueue->head;
                    this->videoQueue->head = p;
                    if (tmp == NULL) {
                        this->videoQueue->tail = p;
                    }
                    p->next = tmp;
                    this->videoQueue->count++;

                    if (this->seekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) {
                        this->processFrame(this->decodedFrame);
                    }
                    break;
                }

                DPRINT("[SEEK]  its a keyframe\n");
                avcodec_decode_video2(this->codecCtx, this->decodedFrame, &gotFrame, &packet);
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

    this->frameTimer = 0; // frameTimer will be init at next display
    this->lastPts = time;
    this->doSeek = false;


#ifdef DEBUG_PRINT
    double end = av_gettime();
    DPRINT("[SEEK] seek took %f / firstFrame to end = %f \n", end-start, end-startFrame);
    DPRINT("Sending seekCond signal\n");
#endif
    this->processVideo();
    NATIVE_PTHREAD_SIGNAL(&this->bufferCond);
}
#undef SEEK_THRESHOLD
#undef SEEK_STEP
#undef SEEK_BUFFER_PACKET

void NativeVideo::nextFrame()
{
    if (this->playing) {
        return;
    }
    this->scheduleDisplay(1, true);
}

void NativeVideo::prevFrame()
{
    if (this->playing) {
        return;
    }
    this->seek(this->getClock(), NATIVE_VIDEO_SEEK_PREVIOUS);
}

void NativeVideo::frameAt(double time, bool keyframe)
{
    if (this->playing) {
        return;
    }

    this->seek(time, keyframe ? NATIVE_VIDEO_SEEK_KEYFRAME : 0);
}

NativeAudioSource *NativeVideo::getAudioNode(NativeAudio *audio)
{

    if (this->audioSource) {
        return this->audioSource;
    }

    this->audio = audio;

    if (this->audioStream != -1 && audio) {
        this->audioSource = new NativeVideoAudioSource(2, this, true);
        audio->addSource(this->audioSource, true);
        this->audioSource->audioStream = this->audioStream;
        this->audioSource->container = this->container;
        this->audioSource->eventCallback(NULL, NULL); //Disable events callbacks
        if (0 != this->audioSource->initInternal()) {
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_INIT_VIDEO_AUDIO, false);
            delete this->audioSource;
            this->audioSource = NULL;
        }

        if (this->getClock() > 0) {
            this->clearAudioQueue();
        } else if (this->playing && this->audioSource) {
            this->audioSource->play();
        }
    }
    return this->audioSource;
}

int NativeVideo::display(void *custom) {
    NativeVideo *v = static_cast<NativeVideo *>(custom);

    // Set pthread cond to false, so
    // setSize will always wait for the signal
    v->m_NotInDisplayCond = false;

    v->m_InDisplay = true;

    DPRINT("[DISPLAY] %d\n", v->m_NoDisplay);

    // Reset timer from queue
    v->timers[v->lastTimer]->id = -1;
    v->timers[v->lastTimer]->delay = -1;

    v->lastTimer++;
    if (v->lastTimer > NATIVE_VIDEO_BUFFER_SAMPLES-1) {
        v->lastTimer = 0;
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
    if (PaUtil_GetRingBufferReadAvailable(v->rBuff) < 1) {
        if (!v->buffering && !v->seeking) {
            NATIVE_PTHREAD_SIGNAL(&v->bufferCond);
        }
        if (v->eof) {
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
    PaUtil_ReadRingBuffer(v->rBuff, (void *)&frame, 1);

    double pts = frame.pts;
    double diff = 0;
    double delay, actualDelay, syncThreshold;

    if (v->frameTimer == 0) {
        v->frameTimer = (av_gettime() / 1000.0);
        if (v->audioSource != NULL) {
            v->audioSource->play();
        }
    }

    delay = pts - v->lastPts;
    DPRINT("DELAY=%f\n", delay);
    if (delay <= 0 || delay >= 1.0) {
        DPRINT("USING LAST DELAY %f\n", v->lastDelay);
        // Incorrect delay, use previous one
        delay = v->lastDelay;
    }

    v->lastDelay = delay;
    v->lastPts = pts;

    if (v->audioSource != NULL && v->audioSource->isConnected) {
        diff = pts - v->audioSource->getClock();

        DPRINT("Clocks audio=%f / video=%f / diff = %f\n", v->audioSource->getClock(), pts, diff);

        if (diff > NATIVE_VIDEO_AUDIO_SYNC_THRESHOLD && v->audioSource->avail() > 0) {
            // Diff is too big an will be noticed
            // Let's drop some audio sample
            DPRINT("Dropping audio before=%f ", diff);
            diff -= v->audioSource->drop(diff);
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

    DPRINT("frameTimer=%f delay=%f videoClock=%f\n", v->frameTimer, delay, pts);
    if (delay < 0) delay = 0;
    v->frameTimer += delay * 1000;
    actualDelay = (v->frameTimer - (av_gettime() / 1000.0)) / 1000;

    DPRINT("Using delay %f frameTimer=%f delay=%f\n", actualDelay, v->frameTimer, delay);

    if (v->playing) {
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

    if (actualDelay > NATIVE_VIDEO_SYNC_THRESHOLD || diff > 0 || !v->playing) {
        // If not playing, we can be here because user seeked
        // while the video is paused, so send the frame anyway

        // Call the frame callback
        if (v->frameCbk != NULL) {
            v->frameCbk(frame.data, v->frameCbkArg);
        }
    }

    if (v->playing) {
        NATIVE_PTHREAD_SIGNAL(&v->bufferCond);
    }

    v->m_InDisplay = false;
    NATIVE_PTHREAD_SIGNAL(&v->m_NotInDisplay);

    return 0;
}

int NativeVideo::setSizeInternal()
{
    m_NoDisplay = true;

    if (m_InDisplay) {
        NATIVE_PTHREAD_WAIT(&this->m_NotInDisplay);
    }

    // Flush buffers & timers to discard old frames
    this->flushBuffers();
    //this->clearTimers(true);

    int width = m_NewWidth == 0 ? this->codecCtx->width : m_NewWidth;
    int height = m_NewHeight == 0 ? this->codecCtx->height : m_NewHeight;

    if (!this->swsCtx) {
        // First call to setSizeInternal, init frames
        this->decodedFrame = av_frame_alloc();
        this->convertedFrame = av_frame_alloc();

        if (this->decodedFrame == NULL || this->convertedFrame == NULL) {
            fprintf(stderr, "Failed to alloc frame\n");
            m_NoDisplay = false;
            return ERR_OOM;
        }
    } else {
        sws_freeContext(this->swsCtx);
    }

    this->swsCtx = sws_getContext(this->codecCtx->width, this->codecCtx->height, this->codecCtx->pix_fmt,
                                  width, height, PIX_FMT_RGBA,
                                  SWS_BICUBIC, NULL, NULL, NULL);

    if (!this->swsCtx) {
        m_NoDisplay = false;
        return ERR_NO_VIDEO_CONVERTER;
    }

    // Update the size of the frames in the frame pool
    int frameSize = avpicture_fill((AVPicture *)this->convertedFrame, NULL, PIX_FMT_RGBA, width, height);
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
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(this->rBuff);

    int idx = m_FramesIdx;
    uint8_t *data;
    for (ring_buffer_size_t i = 0; i < avail; i++) {
        Frame frame;

        data = (uint8_t*) malloc(frameSize);
        PaUtil_ReadRingBuffer(this->rBuff, (void *)&frame, 1);

        this->convertFrame(&frame, data);

        free(m_Frames[m_FramesIdx].data);
        m_Frames[m_FramesIdx].data = data;

        PaUtil_WriteRingBuffer(this->rBuff, &frame, 1);

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
    if (width == m_Width && height == m_Height && this->swsCtx) {
        return;
    }

    m_NewWidth = width;
    m_NewHeight = height;

    m_DoSetSize = true;

    //NATIVE_PTHREAD_SIGNAL(&this->bufferCond);
}

void NativeVideo::buffer()
{
    if (this->error != 0) {
        DPRINT("=> Not buffering cause of error\n");
        return;
    }

    if (this->reader->async) {
        if (this->buffering) {
            DPRINT("=> PENDING\n");
            // Reader already trying to get data
            return;
        }
        this->buffering = true;
        DPRINT("buffer coro start\n");
        Coro_startCoro_(this->mainCoro, this->coro, this, NativeVideo::bufferCoro);
    } else {
        this->bufferInternal();
    }
}

void NativeVideo::bufferCoro(void *arg)
{
   NativeVideo *v = static_cast<NativeVideo*>(arg);
   v->bufferInternal();

    if (!v->reader->pending) {
        v->buffering = false;
    }

    Coro_switchTo_(v->coro, v->mainCoro);

    return;
}

void NativeVideo::bufferInternal()
{
    DPRINT("hello buffer internal\n");
    AVPacket packet;

    bool loopCond = false;

    int needAudio = 0;
    int needVideo = 0;

    if (this->playing) {
        if (this->audioSource != NULL && this->audioSource->isConnected) {
            needAudio = NATIVE_VIDEO_PACKET_BUFFER - this->audioQueue->count;
        }
        needVideo = NATIVE_VIDEO_PACKET_BUFFER - this->videoQueue->count;
    } else {
        needVideo = 1;
    }


    if (needAudio > 0 || needVideo > 0) {
        loopCond = true;
    }

    while (loopCond) {
        DPRINT("    => buffering loop needAudio=%d / needVideo=%d\n", needAudio, needVideo);
        int ret = av_read_frame(this->container, &packet);
        DPRINT("    -> post read frame\n");

        // If a seek is asked while buffering. Return.
        if (this->doSeek) {
            av_free_packet(&packet);
            break;
        }

        // Got a read error. Return.
        if (ret < 0) {
            av_free_packet(&packet);
            if (this->readError(ret) != 0) {
                printf("got error %d/%d\n", this->eof, this->error);
                return;
            }
            continue;
        }

        if (packet.stream_index == this->videoStream) {
            this->addPacket(this->videoQueue, &packet);
            needVideo--;
        } else if (packet.stream_index == this->audioStream &&
            ((this->audioSource != NULL && this->audioSource->isConnected) ||
              this->getClock() == 0)) {
                this->addPacket(this->audioQueue, &packet);
                needAudio--;
        } else {
            av_free_packet(&packet);
        }

        if ((needVideo <= 0 && needAudio <= 0) || this->error != 0) {
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

        if (v->shutdown) break;

        if (v->opened) {
            DPRINT("opened buffering=%d\n", v->buffering);
            if (!v->buffering && !v->seeking) {
                DPRINT("not buffering and seeking\n");
                if (v->doSeek == true) {
                    DPRINT("seeking\n");
                    if (!v->reader->async) {
                        v->seekInternal(v->doSeekTime);
                        v->doSeek = false;
                    } else {
                        DPRINT("    running seek coro\n");
                        Coro_startCoro_(v->mainCoro, v->coro, v, NativeVideo::seekCoro);
                    }
                    DPRINT("done seeking\n");
                } else {
                    DPRINT("buffering\n");
                    //DPRINT("Coro space main = %d coro = %d\n",
                    //    Coro_stackSpaceAlmostGone(v->mainCoro), Coro_stackSpaceAlmostGone(v->coro));
                    v->buffer();
                }
            }

            DPRINT("doSeek=%d readFlag=%d seeking=%d\n", v->doSeek, v->m_SourceNeedWork, v->seeking);
            if (!v->doSeek) {
                DPRINT("processing\n");
#ifdef DPRINT
                bool videoFailed = !v->processVideo();
                bool audioFailed = !v->processAudio();
#endif
                DPRINT("audioFailed=%d videoFailed=%d\n", audioFailed, videoFailed);
            }
        } else if (v->m_SourceDoOpen) {
            v->m_SourceDoOpen = false;
            if (v->reader->async) {
                Coro_startCoro_(v->mainCoro, v->coro, v, NativeVideo::openInitCoro);
            } else {
                v->openInitInternal();
            }
        }

        v->unlockDecodeThread();

        if (v->shutdown) break;

        if (v->m_SourceNeedWork) {
            DPRINT("readFlag, swithcing back to coro\n");
            v->lockDecodeThread();
            v->m_SourceNeedWork = false;
            Coro_switchTo_(v->mainCoro, v->coro);
            // Make sure another read call havn't been made
            if (!v->reader->pending) {
                v->buffering = false;
            }
            v->unlockDecodeThread();
        } else if (!v->doSeek) {
            DPRINT("wait bufferCond, no work needed\n");
            NATIVE_PTHREAD_WAIT(&v->bufferCond);
            DPRINT("Waked up from bufferCond!");
        }
    }

    v->shutdown = false;

    return NULL;
}

void NativeVideo::stopAudio()
{
    NativePthreadAutoLock lock(&this->audioLock);

    this->clearAudioQueue();

    this->audio = NULL;
    // NativeVideo::stopAudio() is called when the audio node
    // is being destructed, so we just set the audioSource to null
    this->audioSource = NULL;
}

void NativeVideo::sourceNeedWork(void *ptr)
{
    NativeVideo *thiz = static_cast<NativeVideo*>(ptr);
    thiz->m_SourceNeedWork = true;
    NATIVE_PTHREAD_SIGNAL(&thiz->bufferCond);
}

bool NativeVideo::processAudio()
{
    NativePthreadAutoLock lock(&this->audioLock);
    DPRINT("processing audio\n");

    if (this->audioSource == NULL) {
        return false;
    }

    if (!this->audioSource->isConnected) {
        return false;
    }

    while (this->audioSource->work()) {}

    // TODO : We should wakup the thread only
    // if source had processed data
    if (this->audio->canWriteFrame()) {
        DPRINT("Wakeup thread\n");
        NATIVE_PTHREAD_SIGNAL(&this->audio->queueHaveData);
    }

    return true;
}

bool NativeVideo::processVideo()
{
    if (PaUtil_GetRingBufferWriteAvailable(this->rBuff) < 1) {
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
    Packet *p = this->getPacket(this->videoQueue);

    if (p == NULL) {
        DPRINT("processVideo no more packet\n");
        if (this->error == AVERROR_EOF) {
            this->eof = true;
        }
        return false;
    }

    AVPacket packet = p->curr;

    avcodec_decode_video2(this->codecCtx, this->decodedFrame, &gotFrame, &packet);

    if (gotFrame) {
        this->processFrame(this->decodedFrame);
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
    PaUtil_WriteRingBuffer(this->rBuff, &frame, 1);

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
    sws_scale(this->swsCtx,
              avFrame->data, avFrame->linesize,
              0, this->codecCtx->height, out, this->convertedFrame->linesize);

    return true;
}

double NativeVideo::syncVideo(double pts)
{
    double frameDelay;

    if (pts != 0 && pts != AV_NOPTS_VALUE) {
        this->videoClock = pts;
    } else {
        pts = this->videoClock;
    }

    frameDelay = av_q2d(this->container->streams[this->videoStream]->time_base);
    frameDelay += this->decodedFrame->repeat_pict * (frameDelay);
    this->videoClock += frameDelay;

    return pts;
}

double NativeVideo::getPts(AVFrame *frame)
{
    double pts;

    pts = av_frame_get_best_effort_timestamp(frame);

    return this->syncVideo(pts) * av_q2d(this->container->streams[this->videoStream]->time_base);
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

    return this->syncVideo(pts) * av_q2d(this->container->streams[this->videoStream]->time_base);
}

void NativeVideo::scheduleDisplay(int delay)
{
    this->scheduleDisplay(delay, false);
}

void NativeVideo::scheduleDisplay(int delay, bool force) {
    this->timers[this->timerIdx]->delay = delay;
    this->timers[this->timerIdx]->id = -1;

    DPRINT("scheduleDisplay in %d\n", delay);

    if (this->playing || force) {
        this->timers[this->timerIdx]->id = this->addTimer(delay);
    }

    this->timerIdx++;

    if (this->timerIdx > NATIVE_VIDEO_BUFFER_SAMPLES-1) {
        this->timerIdx = 0;
    }
}

int NativeVideo::addTimer(int delay)
{
    ape_timer *timer;
    timer = add_timer(&this->net->timersng, delay, NativeVideo::display, this);
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
        if (this->timers[i]->id != -1 && this->timers[i]->delay != -1) {
            clear_timer_by_id(&this->net->timersng, this->timers[i]->id, 1);
        }
        if (reset) {
            this->timers[i]->id = -1;
            this->timers[i]->delay = -1;
        } else {
            delete this->timers[i];
            this->timers[i] = NULL;
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

    Packet *pkt = this->audioQueue->head;
    Packet *next;
    while (pkt != NULL) {
        next = pkt->next;
        av_free_packet(&pkt->curr);
        delete pkt;
        pkt = next;
    }
    this->audioQueue->head = NULL;
    this->audioQueue->tail = NULL;
    this->audioQueue->count = 0;
}

void NativeVideo::clearVideoQueue()
{
    Packet *pkt = this->videoQueue->head;
    Packet *next;
    while (pkt != NULL) {
        next = pkt->next;
        av_free_packet(&pkt->curr);
        delete pkt;
        pkt = next;
    }
    this->videoQueue->head = NULL;
    this->videoQueue->tail = NULL;
    this->videoQueue->count = 0;
}

void NativeVideo::flushBuffers()
{
    if (this->rBuff == NULL) return;

    m_FramesIdx = 0;

    PaUtil_FlushRingBuffer(this->rBuff);
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
    pthread_mutex_lock(&this->decodeThreadLock);
}

void NativeVideo::unlockDecodeThread()
{
    pthread_mutex_unlock(&this->decodeThreadLock);
}

void NativeVideo::closeFFMpeg()
{
    if (this->opened) {
        NativePthreadAutoLock lock(&NativeAVSource::ffmpegLock);
        avcodec_close(this->codecCtx);
        av_free(this->container->pb);
        avformat_close_input(&this->container);
    } else {
        if (this->container) {
            av_free(this->container->pb);
            avformat_free_context(this->container);
        }
    }
}

void NativeVideo::closeInternal(bool reset)
{
    this->clearTimers(reset);

    if (m_ThreadCreated) {
        // Finish the reader first, any pending call
        // will be finished (seek, read) so we can lock the thread
        this->reader->finish();

        this->lockDecodeThread();

        this->shutdown = true;
        this->closeFFMpeg();

        this->unlockDecodeThread();

        NATIVE_PTHREAD_SIGNAL(&this->bufferCond);
        pthread_join(this->threadDecode, NULL);
        m_ThreadCreated = false;

        this->shutdown = false;
    } else {
        this->closeFFMpeg();
    }

    delete this->reader;
    this->reader = NULL;

    this->flushBuffers();

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        free(m_Frames[i]);
        m_Frames[i] = NULL;
    }

    this->clearAudioQueue();
    this->clearVideoQueue();

    if (!reset) {
        delete this->audioQueue;
        delete this->videoQueue;

        this->audioQueue = NULL;
        this->videoQueue = NULL;
    }

    if (this->mainCoro != NULL) {
        Coro_free(this->mainCoro);
        Coro_free(this->coro);
        this->mainCoro = NULL;
        this->coro = NULL;
    }

    delete this->rBuff;

    av_free(this->convertedFrame);
    av_free(this->decodedFrame);

    sws_freeContext(this->swsCtx);

    free(this->buff);

    this->rBuff = NULL;
    this->codecCtx = NULL;
    this->convertedFrame = NULL;
    this->decodedFrame = NULL;
    this->swsCtx = NULL;
    this->buff = NULL;
    this->container = NULL;
    this->reader = NULL;
    this->avioBuffer = NULL;

    if (this->audioSource != NULL) {
        delete this->audioSource;
        this->audioSource = NULL;
    }

    this->opened = false;
    this->m_SourceDoOpen = false;
}

NativeVideo::~NativeVideo() {
    this->closeInternal(false);
}

bool NativeVideoAudioSource::buffer()
{
    if (m_Video->audioQueue->count > 0) {
        if (m_FreePacket != NULL) {
            delete m_FreePacket;
            m_FreePacket = NULL;
            // Note : av_free_packet is called by the audioSource
        }

        NativeVideo::Packet *p = m_Video->getPacket(m_Video->audioQueue);
        this->tmpPacket = &p->curr;
        m_FreePacket = p;
        this->packetConsumed = false;

        return true;
    } else {
        return false;
    }
}

