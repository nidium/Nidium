#include <stdio.h>
#include "NativeVideo.h"
#include "NativeAudioNode.h"
#include "pa_ringbuffer.h"
#include <native_netlib.h>
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

uint64_t NativeVideo::pktPts = AV_NOPTS_VALUE;

NativeVideo::NativeVideo(NativeAudio *audio, ape_global *n) 
    : freePacket(NULL), pendingFrame(NULL), timerIdx(0), lastTimer(0),
      net(n), audio(audio), track(NULL), frameCbk(NULL), frameCbkArg(NULL), shutdown(false), 
      eof(false), lastPts(0), playing(false), stoped(false), doSeek(false), width(-1), height(-1),
      swsCtx(NULL), videoStream(-1), rBuff(NULL) 
{
    pthread_cond_init(&this->buffNotEmpty, NULL);
    pthread_cond_init(&this->resetWait, NULL);
    pthread_cond_init(&this->seekCond, NULL);

    pthread_mutex_init(&this->buffLock, NULL);
    pthread_mutex_init(&this->resetWaitLock, NULL);
    pthread_mutex_init(&this->seekLock, NULL);

    this->audioQueue = new PacketQueue();
    this->videoQueue = new PacketQueue();

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        this->timers[i] = new TimerItem();
    }
}

// TODO : Cleanup after when calling RETURN_WITH_ERROR
int NativeVideo::open(void *buffer, int size) {
#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
return err;

    if (this->opened) {
        this->close(true);
    }

    // FFmpeg stuff
    AVCodec *codec;

    av_register_all();

    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    this->br = new NativeAVBufferReader((uint8_t *)buffer, size);
    this->container = avformat_alloc_context();
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->br, this->br->read, NULL, this->br->seek);

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

    int audioStream = -1;
	for (unsigned int i = 0; i < this->container->nb_streams; i++) {
		if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && this->videoStream == -1) {
			this->videoStream = i;
		} else if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStream == -1) {
            audioStream = i;
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

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
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
        printf("Failed to alloc buffer\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    if (0 > PaUtil_InitializeRingBuffer(this->rBuff, 
            sizeof(NativeVideo::Frame), 
            //frameSize,
            NATIVE_VIDEO_BUFFER_SAMPLES,
            this->buff)) {
        printf("Failed to init ringbuffer\n");
        RETURN_WITH_ERROR(ERR_OOM);
    }

    if (audioStream != -1) {
        this->track = new NativeAudioTrack(2, this->audio, true);
        this->track->audioStream = audioStream;
        this->track->container = this->container;
        this->track->eventCallback(NULL, NULL);
        if (0 != this->track->openInit()) {
            RETURN_WITH_ERROR(ERR_INIT_VIDEO_AUDIO);
        }
    }

    this->opened = true;

    pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);

    this->sendEvent(SOURCE_EVENT_READY, 0, false);

    return 0;
#undef RETURN_WITH_ERROR
}

void NativeVideo::frameCallback(VideoCallback cbk, void *arg) {
    this->frameCbk = cbk;
    this->frameCbkArg = arg;
}

void NativeVideo::play() {
    if (!this->opened) return;
    if (this->playing) return;

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

    if (this->track != NULL) {
        this->track->pause();

        for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
            if (this->timers[i]->id != -1) {
                clear_timer_by_id(&this->net->timersng, this->timers[i]->id, 1);
                this->timers[i]->id = -1;
                this->timers[i]->delay = -1;
            }
        }
    }

    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void NativeVideo::stop() {
    this->playing = false;
    this->videoClock = 0;
    this->lastDelay = 40e-3;

    if (this->track != NULL) {
        this->track->stop();
        avformat_seek_file(this->container, this->track->audioStream, 0, 0, 0, 0);
    }

    avformat_seek_file(this->container, this->videoStream, 0, 0, 0, 0);

    this->flushBuffers();

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        if (this->timers[i]->id != -1 && this->timers[i]->delay != -1) {
            clear_timer_by_id(&this->net->timersng, this->timers[i]->id, 1);
        }
        this->timers[i]->id = -1;
        this->timers[i]->delay = -1;
    }

    pthread_cond_signal(&this->resetWait);
    pthread_cond_signal(&this->buffNotEmpty);
}

double NativeVideo::getClock() 
{
    return this->lastPts;
}

void NativeVideo::seek(double time)
{
    this->doSeekTime = time;
    this->doSeek = true;

    pthread_cond_signal(&this->buffNotEmpty);
    if (this->eof) {
        pthread_cond_signal(&this->resetWait);
    }
    // Thread might have already seeked
    if (this->doSeek == true) {
        pthread_cond_wait(&this->seekCond, &this->seekLock);
    }
    
    this->scheduleDisplay(1, true);
}

void NativeVideo::seekMethod(double time) 
{
    int64_t target = 0;
    int flags = 0;
    double clock = this->lastPts;

    flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;

    target = time * AV_TIME_BASE;

    if (time < 0) {
        time = 0;
        target = 0;
    } else if (target > this->container->duration) {
        target = this->container->duration;
    }

    target = av_rescale_q(target, AV_TIME_BASE_Q, this->container->streams[this->videoStream]->time_base);

    if (av_seek_frame(this->container, this->videoStream, target, flags) >= 0) {
        this->clearTimers(true);

        this->clearAudioQueue();
        this->clearVideoQueue();

        avcodec_flush_buffers(this->codecCtx);
        this->flushBuffers();

        this->lastPts = 0;

        this->eof = false;

        if (this->track != NULL) {
            avcodec_flush_buffers(this->track->codecCtx);
            PaUtil_FlushRingBuffer(this->track->rBufferOut);
            //PaUtil_FlushRingBuffer(this->track->audio->rBufferOut);
            this->track->resetFrames();
            this->track->eof = false;
        }
    } else {
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
    }

    this->doSeek = false;
    
    pthread_cond_signal(&this->seekCond);
}

NativeAudioTrack *NativeVideo::getAudio() {
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

    if (v->track != NULL) {
        diff = pts - v->track->getClock();

        SPAM(("Clocks audio=%f / video=%f / diff = %f\n", v->track->getClock(), pts, diff));

        if (diff > 0.15 && v->track->avail() > 0) {
            // Diff is too big an will be noticed
            // Let's drop some audio sample
            SPAM(("Dropping audio\n"));
            v->track->sync(pts);
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
        v->scheduleDisplay(((int)(actualDelay * 1000 + 0.5)));
        // Call the frame callback
        if (v->frameCbk != NULL) {
            v->frameCbk(frame->data, v->frameCbkArg);
        }
    } else {
        //v->scheduleDisplay(1);
        ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(v->rBuff);
        SPAM(("SKIPING VIDEO FRAME %lu\n", avail));
        if (avail > 0) {
            PaUtil_AdvanceRingBufferReadIndex(v->rBuff, 1);
        }
        v->scheduleDisplay(1);
    }

    free(frame->data);
    delete frame;

    // Wakup decode thread
    //if (!v->eof && PaUtil_GetRingBufferWriteAvailable(v->rBuff) > NATIVE_VIDEO_BUFFER_SAMPLES/2) {
        pthread_cond_signal(&v->buffNotEmpty);
    //}

    return 0;
}

void *NativeVideo::decode(void *args) {
#define WAIT_FOR_RESET if (v->shutdown) break;\
pthread_cond_wait(&v->resetWait, &v->resetWaitLock);\
if (v->shutdown) break;

    NativeVideo *v = static_cast<NativeVideo *>(args);
    AVPacket packet;

    for (;;) {
        if (v->shutdown) break;

        int ret = av_read_frame(v->container, &packet);

        if (packet.stream_index == v->videoStream) {
            if (ret < 0) {
                if (ret == AVERROR_EOF || (v->container->pb && v->container->pb->eof_reached)) {
                    v->eof = true;
                    if (v->track != NULL) {
                        v->track->eof = true;
                    }
                    WAIT_FOR_RESET
                } else if (ret != AVERROR(EAGAIN)) {
                    v->sendEvent(SOURCE_EVENT_ERROR, ERR_DECODING, true);
                    WAIT_FOR_RESET
                }
            } else {
                v->addPacket(v->videoQueue, &packet);
            }
        } else if (v->track != NULL && packet.stream_index == v->track->audioStream) {
            if (ret < 0) {
                // No more audio data, let's output silence
                v->track->resetFrames();
            } else {
                //printf("decoding audio, packet pts is at %f\n", packet.pts * av_q2d(v->container->streams[v->track->audioStream]->time_base));
                v->addPacket(v->audioQueue, &packet);
            }
        } else {
            av_free_packet(&packet);
        }

        bool audioFailed = !v->processAudio();
        bool videoFailed = !v->processVideo();

        if (audioFailed && videoFailed) {
            if (v->shutdown) break;
            if (!v->doSeek) {
                pthread_cond_wait(&v->buffNotEmpty, &v->buffLock);
            } 
            if (v->shutdown) break;
        }

        if (v->doSeek) {
            v->seekMethod(v->doSeekTime);
        }
    }

    return NULL;
#undef WAIT_FOR_RESET
}

bool NativeVideo::processAudio() 
{
    bool audioFailed = false;
    bool wakup = false;

    if (this->track != NULL && (this->audioQueue->count > 0 || !this->track->packetConsumed)) {
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
                        break;
                    }
                } else {
                    audioFailed = true;
                    break;
                }
                
            } else {
                wakup = true;
            }
        }
    }

    if (wakup) {
        pthread_cond_signal(&this->audio->queueHaveData);
    }

    return !audioFailed;
}

bool NativeVideo::processVideo()
{
    if (this->pendingFrame != NULL) {
        if (PaUtil_GetRingBufferWriteAvailable(this->rBuff) < 1) {
            return false;
        } else {
            PaUtil_WriteRingBuffer(this->rBuff, pendingFrame, 1);
            delete pendingFrame;
            this->pendingFrame = NULL;
        }
        
    }

    int gotFrame;
    Packet *p = this->getPacket(this->videoQueue);

    if (p == NULL) {
        return true;
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

        av_free_packet(&packet);

        Frame *frame = new Frame();
        frame->data = (uint8_t *)malloc(this->frameSize);

        frame->pts = this->syncVideo(pts) * av_q2d(this->container->streams[this->videoStream]->time_base);

        uint8_t *tmp[1];
        tmp[0] = (uint8_t *)frame->data;
        uint8_t * const *out = (uint8_t * const*)tmp;

        // Convert the image from its native format to RGBA 
        sws_scale(this->swsCtx,
                  this->decodedFrame->data, this->decodedFrame->linesize,
                  0, this->codecCtx->height, out, this->convertedFrame->linesize);


        // Check ringbuffer space and wait or continue
        if (PaUtil_GetRingBufferWriteAvailable(this->rBuff) < 1) {
            this->pendingFrame = frame;
            return false;
        } else {
            PaUtil_WriteRingBuffer(this->rBuff, frame, 1);
            delete frame;
        }
    } 

    return true;
}

double NativeVideo::syncVideo(double pts) {
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

int NativeVideo::addTimer(int delay) {
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
        pthread_cond_signal(&this->buffNotEmpty);
        pthread_cond_signal(&this->resetWait);
        pthread_join(this->threadDecode, NULL);
    }

    this->clearAudioQueue();
    this->clearVideoQueue();

    if (!reset) {
        delete this->audioQueue;
    }

    this->clearTimers(reset);

    if (this->opened) {
        av_free(this->convertedFrame);
        av_free(this->decodedFrame);
        avcodec_close(this->codecCtx);
        avformat_close_input(&(this->container));
        sws_freeContext(this->swsCtx);

        delete this->rBuff;
        delete this->br;

        free(this->buff);
        free(this->tmpFrame);
        free(this->frameBuffer);

        if (this->track != NULL && reset) {
            this->track->close(true);
        }
    }
}

NativeVideo::~NativeVideo() {
    this->close(false);
}
