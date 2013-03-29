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
      eof(false), lastPts(0), playing(false), stoped(false), width(-1), height(-1),
      swsCtx(NULL), videoStream(-1), audioStream(-1), rBuff(NULL),
      reader(NULL), error(0), buffering(false)
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
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->reader, NativeAVBufferReader::read, NULL, NativeAVBufferReader::seek);

    if (this->openInit() == 0) {
        pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);
        pthread_cond_signal(&this->bufferCond);
    }

    return 0;
}

int NativeVideo::open(const char *src) 
{
    if (this->opened) {
        this->close(true);
    } 

    this->mainCoro = Coro_new();
    Coro_initializeMainCoro(this->mainCoro);

    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->reader = new NativeAVFileReader(src, &this->bufferCond, this, this->net);
    this->container = avformat_alloc_context();
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->reader, NativeAVFileReader::read, NULL, NativeAVFileReader::seek);

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

void NativeVideo::seek(double time)
{
    SPAM(("Seek called\n"));
    if (!this->opened || this->doSeek) {
        SPAM(("not seeking cause already seeking\n"));
        return;
    }

    this->doSeekTime = time < 0 ? 0 : time;
    this->doSeek = true;

    this->clearTimers(true);
    this->flushBuffers();

    pthread_cond_signal(&this->bufferCond);

    this->scheduleDisplay(1, true);
}

void NativeVideo::seekCoro(void *arg) 
{
    NativeVideo *source = static_cast<NativeVideo *>(arg);
    source->seeking = true;
    source->seekInternal(source->doSeekTime);
    source->seeking = false;
    Coro_switchTo_(source->coro, source->mainCoro);
}

void NativeVideo::seekInternal(double time) 
{
    SPAM(("SeekInternal\n"));
    int64_t target = 0;
    int flags = 0;
    double clock = this->lastPts;

    /*
    if (time > this->getDuration()) {
        printf("fixing seek time\n");
        time = this->getDuration();
        time -= 1;
        printf("time = %f\n", time);
    }
    */

    flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;

    target = time * AV_TIME_BASE;

    target = av_rescale_q(target, AV_TIME_BASE_Q, this->container->streams[this->videoStream]->time_base);


    SPAM(("av_seek_frame\n"));
    int ret = av_seek_frame(this->container, this->videoStream, target, flags/*|AVSEEK_FLAG_ANY*/);
    SPAM(("av_seek_frame done ret=%d\n", ret));

    if (ret >= 0) {
        this->clearAudioQueue();
        this->clearVideoQueue();

        avcodec_flush_buffers(this->codecCtx);

        // FFMPEG can success seeking even if seek is after the end
        // (but decoder/demuxer fail). So fix that.
        if (this->doSeekTime > this->getDuration()) {
            this->lastPts = this->getDuration();
        } else {
            this->lastPts = this->doSeekTime;
        }

        if (this->track != NULL) {
            avcodec_flush_buffers(this->track->codecCtx);
            PaUtil_FlushRingBuffer(this->track->rBufferOut);
            //PaUtil_FlushRingBuffer(this->track->audio->rBufferOut);
            this->track->resetFrames();
            if (this->eof && flags == AVSEEK_FLAG_BACKWARD) {
                this->track->eof = false;
            }
        }

        if (this->eof && flags == AVSEEK_FLAG_BACKWARD) {
            this->eof = false;
        }
        this->error = 0;
    } else {
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
    }

    this->frameTimer = (av_gettime() / 1000000.0);
    this->doSeek = false;
    
    SPAM(("Sending seekCond signal\n"));
    pthread_cond_signal(&this->bufferCond);
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
        this->track->eventCallback(NULL, NULL);
        if (0 != this->track->initInternal()) {
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_INIT_VIDEO_AUDIO, false);
            audio->removeTrack(track);
            this->track = NULL;
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
        pthread_cond_signal(&v->bufferCond);
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
    ring_buffer_size_t read = PaUtil_ReadRingBuffer(v->rBuff, (void *)frame, 1);

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

        if (diff > 0.05 && v->track->avail() > 0) {
            // Diff is too big an will be noticed
            // Let's drop some audio sample
            SPAM(("Dropping audio\n"));
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

    if (actualDelay > 0.010 || !v->playing) {
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
        if (actualDelay <= 0.010) {
            int ret = v->display(v);
            if (ret != 0) {
                return ret;
            }
        } else {
            v->scheduleDisplay(((int)(actualDelay * 1000 + 0.5)));
        }
    }

    // Wakup decode thread
    //if (!v->eof && PaUtil_GetRingBufferWriteAvailable(v->rBuff) > NATIVE_VIDEO_BUFFER_SAMPLES/2) {
    if (v->playing) {
        pthread_cond_signal(&v->bufferCond);
    }
    //}

    return 0;
}

void NativeVideo::buffer()
{
    if (!this->playing) {
        // XXX : Don't buffer if video is not playing
        // because, right after opening the video file audio migt not 
        // be connect and thus, the audio packet will not be buffered
        return;
    }

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
}

void NativeVideo::bufferInternal()
{
    SPAM(("hello buffer internal\n"));
    AVPacket packet;

    bool loopCond = false;

    int needAudio = 0;
    int needVideo = 0;

    if (this->track != NULL && this->track->isConnected()) {
        if (this->audioQueue->count < 2) {
            needAudio = 2;
        }
    }

    if (this->videoQueue->count < 2) {
        needVideo = 2;
    }

    if (needAudio > 0 || needVideo > 0) {
        loopCond = true;
    }

    do {
        SPAM(("    => buffernig loop\n"));
        int ret = av_read_frame(this->container, &packet);
        SPAM(("    -> post read frame\n"));

        if (packet.stream_index == this->videoStream) {
            if (ret < 0) {
                if (ret == AVERROR_EOF || (this->container->pb && this->container->pb->eof_reached)) {
                    this->eof = true;
                    if (this->track != NULL) {
                        //this->track->eof = true; 
                        // XXX : Need to find out why when setting EOF, track sometimes fail to play when seeking backward
                    }
                } else if (ret != AVERROR(EAGAIN)) {
                    this->sendEvent(SOURCE_EVENT_ERROR, ERR_DECODING, true);
                }
                this->error = AVERROR_EOF;
            } else {
                this->addPacket(this->videoQueue, &packet);
                needVideo--;
            }
        } else if (packet.stream_index == this->audioStream && this->track != NULL && this->track->isConnected()) {
            if (ret >= 0) {
                this->addPacket(this->audioQueue, &packet);
                needAudio--;
            }
        } else {
            av_free_packet(&packet);
        }

        if (needVideo <= 0 && needAudio <= 0 && this->error == 0) {
            loopCond = false;
        }
        
        if (this->error) {
            loopCond = false;
        }
    } while (loopCond);

}

void *NativeVideo::decode(void *args) 
{
    NativeVideo *v = static_cast<NativeVideo *>(args);
    bool condWait = false;

    for (;;) {
        if (v->opened) {
            if (!v->buffering && !v->seeking) {
                if (v->doSeek == true) {
                    SPAM(("seeking\n"));
                    if (!v->reader->async) {
                        v->seekInternal(v->doSeekTime);
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
            if (!v->seeking) {
                SPAM(("processing\n"));
                bool audioFailed = !v->processAudio();
                bool videoFailed = !v->processVideo();
                SPAM(("audioFailed=%d videoFailed=%d\n", audioFailed, videoFailed));

                if (audioFailed && videoFailed) {
                    if (v->shutdown) break;
                    if ((!v->doSeek && !v->reader->needWakup)) {
                        SPAM(("Waiting for buffNotEmpty cause of audio and video failed\n"));
                        pthread_cond_wait(&v->bufferCond, &v->bufferLock);
                        SPAM(("buffNotEmpty go\n"));
                    } 
                }
            } else {
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

    if (this->track != NULL) {
        this->clearAudioQueue();
    }
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
        //pthread_cond_signal(&this->audio->queueHaveData);
    }

    return !audioFailed;
}

bool NativeVideo::processVideo()
{
    if (PaUtil_GetRingBufferWriteAvailable(this->rBuff) < 1) {
        return false;
    }

    int gotFrame;
    Packet *p = this->getPacket(this->videoQueue);

    if (p == NULL) {
        return false;
    }

    AVPacket packet = p->curr;

    avcodec_decode_video2(this->codecCtx, this->decodedFrame, &gotFrame, &packet);

    if (gotFrame) {
        double pts;
        //printf("decoding video, packet pts is at %f\n", packet.dts * av_q2d(stream->time_base));

        if (packet.pts != AV_NOPTS_VALUE) {
            pts = packet.pts;
        } else if (packet.dts != AV_NOPTS_VALUE) {
            pts = packet.dts;
        } else {
            pts = 0;
        }

        Frame *frame = new Frame();
        // TODO : Use a frame poll, instead of malloc each time.
        frame->data = (uint8_t *)malloc(this->frameSize);
        if (!frame->data) {
            printf("Malloc frame failed\n");
            exit(3);
        }

        frame->pts = this->syncVideo(pts) * av_q2d(this->container->streams[this->videoStream]->time_base);

        uint8_t *tmp[1];
        tmp[0] = (uint8_t *)frame->data;
        uint8_t * const *out = (uint8_t * const*)tmp;

        // Convert the image from its native format to RGBA 
        sws_scale(this->swsCtx,
                  this->decodedFrame->data, this->decodedFrame->linesize,
                  0, this->codecCtx->height, out, this->convertedFrame->linesize);

        PaUtil_WriteRingBuffer(this->rBuff, frame, 1);

        delete frame;
    } 

    delete p;
    av_free_packet(&packet);

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
    /*if (queue->count == NATIVE_VIDEO_PAQUET_QUEUE_SIZE) {
        return false;
    }*/

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
    if (this->track == NULL) return;

    if (this->freePacket != NULL) {
        this->track->packetConsumed = true;
        av_free_packet(this->track->tmpPacket);
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
    if (this->opened) {
        this->shutdown = true;
        pthread_cond_signal(&this->bufferCond);
        pthread_join(this->threadDecode, NULL);
    }

    
    this->clearTimers(reset);
    this->flushBuffers();

    this->clearAudioQueue();
    this->clearVideoQueue();

    if (!reset) {
        delete this->audioQueue;
        delete this->videoQueue;
    }

    if (this->coro != NULL) {
        Coro_free(this->coro);
    }

    // TODO : instead of checking for opened
    // check for non NULL variables and free them
    if (this->opened) {
        av_free(this->convertedFrame);
        av_free(this->decodedFrame);
        avcodec_close(this->codecCtx);
        avformat_close_input(&(this->container));
        sws_freeContext(this->swsCtx);

        delete this->rBuff;
        delete this->reader;

        free(this->buff);
        free(this->tmpFrame);
        free(this->frameBuffer);

        if (this->track != NULL) {
            if (reset) {
                this->track->close(true);
            } else {
                delete this->track;
            }
        }
    }
}

NativeVideo::~NativeVideo() {
    this->close(false);
}
