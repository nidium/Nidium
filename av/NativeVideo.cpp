#include <stdio.h>
#include "NativeVideo.h"
#include "NativeAudioNode.h"
#include "native_netlib.h"

#include "pa_ringbuffer.h"
#include "Coro.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#undef SPAM
#if 0
  #define SPAM(a) printf a
#else
  #define SPAM(a) (void)0
#endif

// XXX : Well, NativeVideo need a better interaction with NativeAudi. 
// There's a lot of little hack to work nicely with it.

NativeVideo::NativeVideo(ape_global *n) 
    : freePacket(NULL), timerIdx(0), lastTimer(0),
      net(n), track(NULL), frameCbk(NULL), frameCbkArg(NULL), shutdown(false), 
      lastPts(0), playing(false), stoped(false), width(-1), height(-1),
      swsCtx(NULL), codecCtx(NULL), videoStream(-1), audioStream(-1), 
      rBuff(NULL), buff(NULL), avioBuffer(NULL),
      decodedFrame(NULL), convertedFrame(NULL),
      reader(NULL), buffering(false)
{
    pthread_cond_init(&this->bufferCond, NULL);
    pthread_mutex_init(&this->bufferLock, NULL);
    pthread_mutex_init(&this->audioLock, NULL);

    this->audioQueue = new PacketQueue();
    this->videoQueue = new PacketQueue();

    this->doSeek = false;
    this->seeking = false;

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        this->timers[i] = new TimerItem();
    }
}

#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
this->close(true);\
return err;

int NativeVideo::open(void *buffer, int size) 
{
    if (this->opened) {
        this->close(true);
    }

    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->reader = new NativeAVBufferReader((uint8_t *)buffer, size);
    this->container = avformat_alloc_context();
    if (!this->container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->reader, NativeAVBufferReader::read, NULL, NativeAVBufferReader::seek);
    if (!this->container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }
    if (this->openInit() == 0) {
        pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);
        pthread_cond_signal(&this->bufferCond);
    }

    return 0;
}

int NativeVideo::open(const char *src) 
{
    if (this->avioBuffer != NULL) {
        this->close(true);
    } 

    this->mainCoro = Coro_new();
    Coro_initializeMainCoro(this->mainCoro);

    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->reader = new NativeAVFileReader(src, &this->bufferCond, this, this->net);
    this->container = avformat_alloc_context();
    if (!this->container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->reader, NativeAVFileReader::read, NULL, NativeAVFileReader::seek);
    if (!this->container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    if (!this->container->pb) {
        this->close(false);
        RETURN_WITH_ERROR(ERR_OOM);
    }

    pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);

    return 0;
}

int NativeVideo::openInit() 
{
    if (this->reader->async) {
        this->coro = Coro_new();
        Coro_startCoro_(this->mainCoro, this->coro, this, NativeVideo::openInitCoro);
    } else {
        return this->openInitInternal();
    }
    return 0;
}

void NativeVideo::openInitCoro(void *arg) 
{
    NativeVideo *thiz = (static_cast<NativeVideo*>(arg));
    thiz->openInitInternal();
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
        RETURN_WITH_ERROR(ERR_INTERNAL);
	}

    if (avformat_find_stream_info(this->container, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information");
        RETURN_WITH_ERROR(ERR_NO_INFORMATION);
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
        fprintf(stderr, "No video stream");
		RETURN_WITH_ERROR(ERR_NO_VIDEO);
    }

    this->codecCtx = this->container->streams[this->videoStream]->codec;
    //this->codecCtx->get_buffer = NativeVideo::getBuffer;
    //this->codecCtx->release_buffer = NativeVideo::releaseBuffer;

    codec = avcodec_find_decoder(this->codecCtx->codec_id);

    if (avcodec_open2(this->codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Could not find or open the needed codec\n");
		RETURN_WITH_ERROR(ERR_NO_CODEC);
    }

    this->width = this->codecCtx->width;
    this->height = this->codecCtx->height;

    this->swsCtx = sws_getContext(this->codecCtx->width, this->codecCtx->height, this->codecCtx->pix_fmt,
                                  this->codecCtx->width, this->codecCtx->height, PIX_FMT_RGBA,
                                  SWS_BICUBIC, NULL, NULL, NULL);

    if (!this->swsCtx) {
        fprintf(stderr, "Failed to init video converter!\n");
		RETURN_WITH_ERROR(ERR_NO_VIDEO_CONVERTER);
    }

    // Allocate video frame
    this->decodedFrame = avcodec_alloc_frame();

    // Allocate an AVFrame structure
    this->convertedFrame = avcodec_alloc_frame();
    if (this->decodedFrame == NULL || this->convertedFrame == NULL) {
        fprintf(stderr, "Failed to alloc frame\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    // Determine required buffer size and allocate buffer
    int frameSize = avpicture_get_size(PIX_FMT_RGBA, codecCtx->width, codecCtx->height);
    this->frameSize = frameSize;
    this->frameBuffer = (uint8_t *)av_malloc(frameSize * sizeof(uint8_t));
    if (frameBuffer == NULL) {
        fprintf(stderr, "Failed to alloc buffer\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    avpicture_fill((AVPicture *)this->convertedFrame, this->frameBuffer, PIX_FMT_RGBA, this->codecCtx->width, this->codecCtx->height);

    // NativeAV stuff
    this->frameTimer = (av_gettime() / 1000000.0);
    this->lastDelay = 40e-3;// Still don't know why I am using this constant to init lastDelay

    this->tmpFrame = (uint8_t *) malloc(frameSize);
    if (this->tmpFrame == NULL) {
        fprintf(stderr, "Failed to alloc tmp frame\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->rBuff = new PaUtilRingBuffer();
    this->buff = (uint8_t*) malloc(frameSize * NATIVE_VIDEO_BUFFER_SAMPLES);

    if (this->buff == NULL) {
        fprintf(stderr, "Failed to alloc buffer\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    if (0 > PaUtil_InitializeRingBuffer(this->rBuff, 
            sizeof(NativeVideo::Frame), 
            //frameSize,
            NATIVE_VIDEO_BUFFER_SAMPLES,
            this->buff)) {
        fprintf(stderr, "Failed to init ringbuffer\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    //pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);

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

    this->frameTimer = (av_gettime() / 1000000.0);
    this->playing = true;

    if (this->track != NULL) {
        this->track->play();
    }

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

void NativeVideo::pause() {
    this->playing = false;

    this->clearTimers(true);

    if (this->track != NULL) {
        this->track->pause();
    }

    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void NativeVideo::stop() {
    this->playing = false;
    this->videoClock = 0;
    this->lastDelay = 40e-3;
    this->error = 0;

    if (this->track != NULL) {
        this->track->stop();
    }

    this->seek(0);

    this->flushBuffers();
    this->clearTimers(true);

    pthread_cond_signal(&this->bufferCond);
}

double NativeVideo::getClock() 
{
    return this->lastPts;
}

void NativeVideo::seek(double time, uint32_t flags)
{
    SPAM(("Seek called\n"));
    if (!this->opened || this->doSeek) {
        SPAM(("not seeking cause already seeking\n"));
        return;
    }

    this->doSeekTime = time < 0 ? 0 : time;
    this->doSeek = true;
    this->seekFlags = flags;

    this->clearTimers(true);
    this->flushBuffers();

    pthread_cond_signal(&this->bufferCond);

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
    SPAM(("av_seek_frame\n"));
    if (!(this->seekFlags & NATIVE_VIDEO_SEEK_KEYFRAME)) {
        flags |= AVSEEK_FLAG_ANY;
    }
    int ret = av_seek_frame(this->container, this->videoStream, target, flags);
    SPAM(("av_seek_frame done ret=%d\n", ret));
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
    double start = av_gettime()/1000;
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

    SPAM(("SeekInternal\n"));

    if (time > this->getDuration()) {
        time = this->getDuration();
        time -= 0.1;
    }

    seekTime = time;
    diff = time - this->getClock();
    target = this->seekTarget(time, &flags);

    SPAM(("[SEEK] diff = %f\n", diff));
    if (diff > SEEK_THRESHOLD || diff <= 0 || this->seekFlags & NATIVE_VIDEO_SEEK_KEYFRAME) {
        // Flush all buffers
        this->clearAudioQueue();
        this->clearVideoQueue();

        if (this->track != NULL) {
            avcodec_flush_buffers(this->track->codecCtx);
            PaUtil_FlushRingBuffer(this->track->rBufferOut);
            this->track->resetFrames();
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
        if (track != NULL) {
            double tmp;
            // "in memory" seeking, we need to drop audio packet
            for (;;) {
                p = this->getPacket(this->audioQueue);

                if (p == NULL) {
                    break;
                }

                tmp = av_q2d(this->container->streams[this->audioStream]->time_base) * p->curr.pts;
                SPAM(("[SEEK] Dropping audio packet @ %f\n", tmp));

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
        if (this->track != NULL) {
            this->track->eof = false;
        }
    }

    flags = 0;

    for (;;) {
        SPAM(("[SEEK] loop gotFrame=%d pts=%f seekTime=%f time=%f\n", gotFrame, pts, seekTime, time));
        if ((pts > seekTime + SEEK_STEP || pts > time) && !keyframe) {
            seekTime = seekTime - SEEK_STEP;
            if (seekTime < 0) seekTime = 0;
            target = this->seekTarget(seekTime, &flags);
            SPAM(("    => seeking backward to %f/%lld\n", seekTime, target));
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
            SPAM(("[SEEK] av_read_frame\n"));
            int count = 0;
            while (count < SEEK_BUFFER_PACKET) {
                int err = av_read_frame(this->container, &packet);
                if (err < 0) {
                    av_free_packet(&packet);
                    err = this->readError(err);
                    if (err == AVERROR_EOF && this->videoQueue->count > 0) {
                        break;
                    } else if (err < 0) {
                        SPAM(("[SEEK] Got fatal error %d\n", err));
                        return;
                    }
                    continue;
                }
                if (packet.stream_index == this->videoStream) {
                    double tmp = this->getPts(&packet);
                    SPAM(("[SEEK] Got packet @ %f\n", tmp));
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
            SPAM(("[SEEK] getPacket\n"));
            p = this->getPacket(this->videoQueue);
            if (p == NULL) {
                continue;
            }
            packet = p->curr;
        }

        SPAM(("[SEEK] reading packet stream=%d, pts=%lld/%lld\n", packet.stream_index, packet.pts, packet.dts));

        if (packet.stream_index == this->videoStream) {
            double prevPts = pts;
            pts = this->getPts(&packet);
            frame = true;

            SPAM(("[SEEK] got video frame at=%f flags=%d\n", pts, packet.flags));


            if (packet.flags & AV_PKT_FLAG_KEY || keyframe) {
                // We have our frame!
                if ((pts >= time || seekTime == 0) && gotFrame) {
                    if (this->seekFlags & NATIVE_VIDEO_SEEK_PREVIOUS && (pts != time && seekTime != 0)) {
                        // When seeking to the previous frame, we need to be at
                        // the exact frame. As it's not the case, seek backward
                        pts += 120; 
                        keyframe = false;
                        continue;
                    }
                    SPAM(("[SEEK] got seek frame at=%f\n", pts));
                    Packet *tmp = this->videoQueue->head;
                    this->videoQueue->head = p;
                    if (tmp == NULL) {
                        this->videoQueue->tail = p;
                    }
                    p->next = tmp;
                    this->videoQueue->count++;

                    if (this->seekFlags & NATIVE_VIDEO_SEEK_PREVIOUS) {
                        this->processFrame(this->decodedFrame, prevPts);
                    } 
                    break;
                }

                SPAM(("[SEEK]  its a keyframe\n"));
                avcodec_decode_video2(this->codecCtx, this->decodedFrame, &gotFrame, &packet);
                if (gotFrame) {
                    SPAM(("[SEEK] ==== GOT FRAME\n"));
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

    this->frameTimer = (av_gettime() / 1000000.0);
    this->doSeek = false;
    
    double end = av_gettime()/1000;
    SPAM(("[SEEK] seek took %f / firstFrame to end = %f \n", end-start, end-startFrame));
    SPAM(("Sending seekCond signal\n"));
    pthread_cond_signal(&this->bufferCond);
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

NativeAudioTrack *NativeVideo::getAudioNode(NativeAudio *audio) 
{

    if (this->track) {
        return this->track;
    }

    this->audio = audio;

    if (this->audioStream != -1 && audio) {
        this->track = audio->addTrack(2, true);
        this->track->audioStream = this->audioStream;
        this->track->container = this->container;
        this->track->eventCallback(NULL, NULL); //Disable events callbacks
        if (0 != this->track->initInternal()) {
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_INIT_VIDEO_AUDIO, false);
            audio->removeTrack(track);
            this->track = NULL;
        }

        if (this->playing) {
            this->track->play();
        }

        if (this->getClock() > 0) {
            this->clearAudioQueue();
        }
    }
    return this->track;
}

int NativeVideo::display(void *custom) {
    NativeVideo *v = (NativeVideo *)custom;

    // Reset timer from queue
    v->timers[v->lastTimer]->id = -1;
    v->timers[v->lastTimer]->delay = -1;

    v->lastTimer++;
    if (v->lastTimer > NATIVE_VIDEO_BUFFER_SAMPLES-1) {
        v->lastTimer = 0;
    }
    // Read frame from ring buffer
    if (PaUtil_GetRingBufferReadAvailable(v->rBuff) < 1) {
        if (!v->buffering && !v->seeking) {
            pthread_cond_signal(&v->bufferCond);
        } 
        if (v->eof) {
            SPAM(("No frame, eof reached\n"));
            v->sendEvent(SOURCE_EVENT_EOF, 0, false);
            return 0;
        } else {
            SPAM(("No frame, try again in 5ms\n"));
            return 5;
        }
    }

    Frame *frame = new Frame();
    PaUtil_ReadRingBuffer(v->rBuff, (void *)frame, 1);

    double pts = frame->pts;
    double delay, actualDelay, diff, syncThreshold;


    delay = pts - v->lastPts;
    if (delay <= 0 || delay >= 1.0) {
        // Incorrect delay, use previous one
        delay = v->lastDelay;
    }

    v->lastDelay = delay;
    v->lastPts = pts;

    if (v->track != NULL && v->track->isConnected()) {
        diff = pts - v->track->getClock();

        SPAM(("Clocks audio=%f / video=%f / diff = %f\n", v->track->getClock(), pts, diff));

        if (diff > NATIVE_VIDEO_AUDIO_SYNC_THRESHOLD && v->track->avail() > 0) {
            // Diff is too big an will be noticed
            // Let's drop some audio sample
            v->track->drop(diff);
        } else {
            syncThreshold = (delay > NATIVE_VIDEO_SYNC_THRESHOLD) ? delay : NATIVE_VIDEO_SYNC_THRESHOLD;

            if (fabs(diff) < NATIVE_VIDEO_NOSYNC_THRESHOLD) {
                if (diff <= -syncThreshold) {
                    SPAM((" (diff < syncThreshold) "));
                    delay = 0;
                } else if (diff >= syncThreshold) {
                    SPAM((" (diff > syncThreshold) "));
                    delay = 2 * delay;
                }
            }
            SPAM((" | after  %f\n", delay));
        }
    } 

    v->frameTimer += delay;
    actualDelay = v->frameTimer - (av_gettime() / 1000000.0);

    SPAM(("Using delay %f\n", actualDelay));

    if (actualDelay > NATIVE_VIDEO_SYNC_THRESHOLD || diff > 0 || !v->playing) {
        // If not playing, we can be here because user seeked 
        // while the video is paused, so send the frame anyway

        // Call the frame callback
        if (v->frameCbk != NULL) {
            v->frameCbk(frame->data, v->frameCbkArg);
        }
    } 

    free(frame->data);
    delete frame;

    if (v->playing) {
        if (actualDelay <= NATIVE_VIDEO_SYNC_THRESHOLD && diff <= 0) {
            if (int ret = v->display(v) != 0) {
                return ret;
            }
        } else {
            v->scheduleDisplay(((int)(actualDelay * 1000 + 0.5)));
        }
        pthread_cond_signal(&v->bufferCond);
    }

    return 0;
}

void NativeVideo::buffer()
{
    if (this->error != 0) {
        SPAM(("=> Not buffering cause of error\n"));
        return;
    }
    if (this->reader->async) {
        if (this->buffering) {
            SPAM(("=> PENDING\n"));
            // Reader already trying to get data
            return;
        }
        this->buffering = true;
        SPAM(("buffer coro start\n"));
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
    SPAM(("hello buffer internal\n"));
    AVPacket packet;

    bool loopCond = false;

    int needAudio = 0;
    int needVideo = 0;

    if (this->playing) {
        if (this->track != NULL && this->track->isConnected()) {
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
        SPAM(("    => buffernig loop needAudio=%d / needVideo=%d\n", needAudio, needVideo));
        int ret = av_read_frame(this->container, &packet);
        SPAM(("    -> post read frame\n"));

        if (this->doSeek) {
            av_free_packet(&packet);
            break;
        }

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
        } else if (packet.stream_index == this->audioStream && ((this->track != NULL && this->track->isConnected()) || this->getClock() == 0)) {
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
        if (v->opened) {
            if (!v->buffering && !v->seeking) {
                if (v->doSeek == true) {
                    SPAM(("seeking\n"));
                    if (!v->reader->async) {
                        v->seekInternal(v->doSeekTime);
                        v->doSeek = false;
                    } else {
                        SPAM(("    running seek coro\n"));
                        Coro_startCoro_(v->mainCoro, v->coro, v, NativeVideo::seekCoro);
                    } 
                    if (v->doSeek == true) { 
                        SPAM(("     Waiting for seekCond\n"));
                        pthread_cond_wait(&v->bufferCond, &v->bufferLock);
                        SPAM(("     seekCond go\n"));
                    }
                    SPAM(("done seeking\n"));
                } else {
                    SPAM(("buffering\n"));
                    //SPAM(("Coro space main=%d coro=%d\n", Coro_stackSpaceAlmostGone(v->mainCoro), Coro_stackSpaceAlmostGone(v->coro)));
                    v->buffer();
                }
            }

            if (v->error != 0) {
                if (v->shutdown) break;
                SPAM(("Waiting for reset cause of error %d\n", v->reader->pending));
                if (!v->doSeek && !v->reader->needWakup) {
                    pthread_cond_wait(&v->bufferCond, &v->bufferLock);
                }
                SPAM(("reset go\n"));
                if (v->shutdown) break;
            }

            SPAM(("doSeek=%d needWakup=%d seeking=%d\n", v->doSeek, v->reader->needWakup, v->seeking));
            if (!v->doSeek) {
                SPAM(("processing\n"));
                bool audioFailed = !v->processAudio();
                bool videoFailed = !v->processVideo();
                SPAM(("audioFailed=%d videoFailed=%d\n", audioFailed, videoFailed));

                if (audioFailed && videoFailed) {
                    if (v->shutdown) break;
                    if (!v->doSeek && !v->reader->needWakup) {
                        SPAM(("Waiting for buffNotEmpty cause of audio and video failed\n"));
                        pthread_cond_wait(&v->bufferCond, &v->bufferLock);
                        SPAM(("buffNotEmpty go\n"));
                    } 
                }
            } else if (!v->reader->needWakup) {
                pthread_cond_wait(&v->bufferCond, &v->bufferLock);
            }
        }

        if (v->shutdown) break;

        if (v->reader->needWakup) {
            Coro_switchTo_(v->mainCoro, v->coro);
            // Make sure another read call havn't been made
            if (!v->reader->pending) {
                v->buffering = false;
            }
        }
    }

    return NULL;
#undef WAIT_FOR_RESET
}

void NativeVideo::stopAudio()
{
    pthread_mutex_lock(&this->audioLock);

    this->clearAudioQueue();

    this->audio = NULL;
    this->track = NULL; 

    pthread_mutex_unlock(&this->audioLock);
}

bool NativeVideo::processAudio() 
{
    bool audioFailed = false;
    bool wakeup = false;

    pthread_mutex_lock(&this->audioLock);

    if (this->track == NULL || (this->track != NULL && !this->track->isConnected())) {
        pthread_mutex_unlock(&this->audioLock);
        return false;
    }

    if (this->audioQueue->count > 0 || !this->track->packetConsumed) {
        for (;;) {
            // Decode audio
            if (!this->track->work()) {
                if (this->track->packetConsumed && this->freePacket != NULL) {
                    delete this->freePacket;
                    this->freePacket = NULL;
                    // Note : av_free_packet is called by the track
                }
                if (this->track->packetConsumed) {
                    Packet *p = this->getPacket(this->audioQueue);
                    if (p != NULL) {
                        this->track->buffer(&p->curr);
                        this->freePacket = p;
                    } else {
                        // No more packet, no more to decode
                        audioFailed = true;
                        break;
                    }
                } else {
                    // Packet not consumed, but audio work failed
                    audioFailed = true;
                    break;
                }
            } else {
                wakeup = true;
            }
        }
    } else {
        // No track || no packet || packet not consumed (CHECKME)
        audioFailed = true;
    }

    pthread_mutex_unlock(&this->audioLock);

    if (wakeup) {
        pthread_cond_signal(&this->audio->queueHaveData);
    }

    return !audioFailed;
}

bool NativeVideo::processVideo()
{
    if (PaUtil_GetRingBufferWriteAvailable(this->rBuff) < 1) {
        SPAM(("processVideo not enought space to write data\n"));
        return false;
    }

    int gotFrame;
    Packet *p = this->getPacket(this->videoQueue);

    if (p == NULL) {
        SPAM(("processVideo no more packet\n"));
        if (this->error == AVERROR_EOF) {
            this->eof = true;
        }
        return false;
    }

    AVPacket packet = p->curr;

    avcodec_decode_video2(this->codecCtx, this->decodedFrame, &gotFrame, &packet);

    if (gotFrame) {
        //printf("decoding video, packet pts is at %f\n", packet.dts * av_q2d(stream->time_base));
        this->processFrame(this->decodedFrame, this->getPts(&packet));
    } 

    delete p;
    av_free_packet(&packet);

    return true;
}

bool NativeVideo::processFrame(AVFrame *avFrame, double pts)
{
    Frame *frame = new Frame();
    // TODO : Use a frame poll, instead of malloc each time.
    frame->data = (uint8_t *)malloc(this->frameSize);
    if (!frame->data) {
        printf("Malloc frame failed\n");
        exit(3);
        return false;
    }

    frame->pts = pts;

    uint8_t *tmp[1];
    tmp[0] = (uint8_t *)frame->data;
    uint8_t * const *out = (uint8_t * const*)tmp;

    // Convert the image from its native format to RGBA 
    sws_scale(this->swsCtx,
              avFrame->data, avFrame->linesize,
              0, this->codecCtx->height, out, this->convertedFrame->linesize);

    PaUtil_WriteRingBuffer(this->rBuff, frame, 1);

    delete frame;

    return true;
}

double NativeVideo::syncVideo(double pts) 
{
    double frameDelay;

    if (pts != 0) {
        this->videoClock = pts;
    } else {
        pts = this->videoClock;
    }

    frameDelay = av_q2d(this->container->streams[this->videoStream]->time_base);
    frameDelay += this->decodedFrame->repeat_pict * (frameDelay * 0.5); //wtf 0.5?!
    this->videoClock += frameDelay;

    return pts;
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
    if (queue->count == 0) {
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
        }
    }
}

void NativeVideo::clearAudioQueue() 
{
    if (this->freePacket != NULL) {
        if (this->track != NULL) {
            this->track->packetConsumed = true;
        }
        av_free_packet(&this->freePacket->curr);
        delete this->freePacket;
        this->freePacket = NULL;
    }

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

    Frame *frame = new Frame();

    while (PaUtil_ReadRingBuffer(this->rBuff, frame, 1) > 0) {
        free(frame->data);
    }

    delete frame;

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

void NativeVideo::close(bool reset) {
    if (this->reader != NULL) {
        this->shutdown = true;
        pthread_cond_signal(&this->bufferCond);
        pthread_join(this->threadDecode, NULL);
        this->shutdown = false;
    }

    
    this->clearTimers(reset);
    this->flushBuffers();

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

        this->mainCoro = NULL;
    }

    if (this->coro != NULL) {
        Coro_free(this->coro);

        this->coro = NULL;
    }

    if (this->opened) {
        delete this->rBuff;

        avcodec_close(this->codecCtx);
        avformat_close_input(&this->container);

        av_free(this->convertedFrame);
        av_free(this->decodedFrame);
        av_free(this->frameBuffer);

        sws_freeContext(this->swsCtx);
        
        free(this->buff);
        free(this->tmpFrame);

        this->rBuff = NULL;
        this->codecCtx = NULL;
        this->convertedFrame = NULL;
        this->decodedFrame = NULL;
        this->swsCtx = NULL;
        this->frameBuffer = NULL;
        this->buff = NULL;
        this->tmpFrame = NULL;
    }

    if (this->avioBuffer != NULL) {
        if (!this->opened) {
            if (this->container->pb != NULL) {
                av_free(this->container->pb);
            }
            avformat_free_context(this->container);
            av_free(this->avioBuffer);
        }

        delete this->reader;

        this->container = NULL;
        this->reader = NULL;
        this->avioBuffer = NULL;
    }

    if (this->track != NULL) {
        if (reset) {
            this->track->close(true);
        } else {
            delete this->track;
        }
    }
}

NativeVideo::~NativeVideo() {
    this->close(false);
}
