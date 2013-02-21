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

uint64_t NativeVideo::pktPts = AV_NOPTS_VALUE;

NativeVideo::NativeVideo(NativeAudio *audio, ape_global *n) 
    : timerIdx(0), lastTimer(0), timersDelay(0),
      net(n), audio(audio), track(NULL), cbk(NULL), cbkArg(NULL), shutdown(false), 
      eof(false), lastPts(0), playing(false), container(NULL),
      videoStream(-1)
{
    pthread_cond_init(&this->buffNotEmpty, NULL);
    pthread_mutex_init(&this->buffLock, NULL);

    this->audioQueue = new AudioQueue();

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        this->timers[i] = new TimerItem();
    }
}

int NativeVideo::open(void *buffer, int size) {
/*if (this->cbk != NULL) { \
    TrackEvent *ev = new TrackEvent(this, TRACK_EVENT_ERROR, err, this->cbkCustom, false);\
    this->cbk(ev);\
}*/
#define RETURN_WITH_ERROR(err) \
return err;

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
    this->codecCtx->get_buffer = NativeVideo::getBuffer;
    this->codecCtx->release_buffer = NativeVideo::releaseBuffer;

    codec = avcodec_find_decoder(this->codecCtx->codec_id);

    if (avcodec_open2(this->codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Could not find or open the needed codec\n");
		RETURN_WITH_ERROR(ERR_NO_CODEC);
    }

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
    this->lastDelay = 40e-3;// Still don't know why I am using this constant 

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
        if (0 != this->track->openInit()) {
            RETURN_WITH_ERROR(ERR_INIT_VIDEO_AUDIO);
        }
    }

    pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);

    return 0;
#undef RETURN_WITH_ERROR
}

void NativeVideo::setCallback(VideoCallback cbk, void *arg) {
    this->cbk = cbk;
    this->cbkArg = arg;
}

void NativeVideo::play() {
    this->playing = true;
    this->track->play();

    this->scheduleDisplay(50);
    /*
    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        if (this->timers[i]->id == -1 && this->timers[i]->delay != -1) {
            this->timers[i]->id = this->addTimer(this->timers[i]->delay);
        }
    }
    */
}

void NativeVideo::pause() {
    this->playing = false;

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        if (this->timers[i]->id != -1) {
            clear_timer_by_id(&this->net->timersng, this->timers[i]->id, 1);
            this->timers[i]->id = -1;
            // TODO : Need updated delay
        }
    }
}

NativeAudioTrack *NativeVideo::getAudio() {
    return this->track;
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

void NativeVideo::scheduleDisplay(int delay) {
    this->timers[this->timerIdx]->delay = delay;
    this->timers[this->timerIdx]->id = -1;
    this->timersDelay += delay;

    if (this->playing) {
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
    timer->flags &= ~APE_TIMER_IS_PROTECTED; // XXX : Remove me
    return timer->identifier;
}

int NativeVideo::display(void *custom) {
    NativeVideo *v = (NativeVideo *)custom;

    // Reset timer from queue
    v->timersDelay -= v->timers[v->lastTimer]->delay;
    v->timers[v->lastTimer]->id = -1;
    v->timers[v->lastTimer]->delay = -1;

    v->lastTimer++;
    if (v->lastTimer > NATIVE_VIDEO_BUFFER_SAMPLES-1) {
        v->lastTimer = 0;
    }

    Frame *frame = new Frame();
    // Read frame from ring buffer
    if (PaUtil_GetRingBufferReadAvailable(v->rBuff) < 1) {
        // TODO schedule wakeup
        return 0;
    }

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

    if (v->track != NULL) {
        diff = pts - v->track->getClock();
        SPAM(("Clocks audio=%f / video=%f / diff = %f\n", v->track->getClock(), pts, diff));
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

    v->frameTimer += delay;
    actualDelay = v->frameTimer - (av_gettime() / 1000000.0);

    SPAM(("Using delay %f\n", actualDelay));

    if (actualDelay > 0.010) {
        v->scheduleDisplay(((int)(actualDelay * 1000 + 0.5)));
    } else {
        v->scheduleDisplay(0.01);
        SPAM(("SKIPING FRAME\n"));
        /*
        // Read frame from ring buffer
        if (PaUtil_GetRingBufferReadAvailable(v->rBuff) < 1) {
            printf("Nothing in rbuff\n");
            return 0;
        }

        Frame *foo = new Frame();

        PaUtil_ReadRingBuffer(v->rBuff, (void *)foo, 1);
        free(foo->data);
        free(foo);
        */
    }

    // Call the frame callback
    if (v->cbk != NULL) {
        v->cbk(v->codecCtx->width, v->codecCtx->height, frame->data, v->cbkArg);
    }

    free(frame->data);
    free(frame);

    // Wakup decode thread
    //if (!v->eof && PaUtil_GetRingBufferWriteAvailable(v->rBuff) > NATIVE_VIDEO_BUFFER_SAMPLES/2) {
        pthread_cond_signal(&v->buffNotEmpty);
    //}

    return 0;
}

/*
void NativeVideo::setSize(int width, int height) {
    this->width = width;
    this->height = height;
}
*/

void *NativeVideo::decode(void *args) {
    NativeVideo *v = static_cast<NativeVideo *>(args);
    int             gotFrame;
    AVPacket        packet;
    AVStream *stream;
    double pts;

    pts = 0;
    stream = v->container->streams[v->videoStream];

    for (;;) {
        int ret = av_read_frame(v->container, &packet);
        if (packet.stream_index == v->videoStream) {
            if (ret < 0) {
                if (ret == AVERROR(EOF) || (v->container->pb && v->container->pb->eof_reached)) {
                    printf("eof\n");
                    v->eof = true;
                    // TODO : Fire EOF and wait for reset
                } else if (ret != AVERROR(EAGAIN)) {
                    printf("error\n");
                    // TODO : Fire Error and wait for reset
                }
            } else {
                NativeVideo::pktPts = packet.pts;

                avcodec_decode_video2(v->codecCtx, v->decodedFrame, &gotFrame, &packet);

                if (packet.dts != AV_NOPTS_VALUE) {
                    pts = packet.dts;
                } else {
                    pts = 0;
                }

                av_free_packet(&packet);

                pts *= av_q2d(stream->time_base);

                if (gotFrame) {
                    Frame *frame = new Frame();
                    frame->data = (uint8_t *)malloc(v->frameSize);
                    uint8_t *tmp[1];
                    tmp[0] = (uint8_t *)frame->data;
                    uint8_t * const *out = (uint8_t * const*)tmp;

                    // Convert the image from its native format to RGBA 
                    sws_scale(v->swsCtx,
                              v->decodedFrame->data, v->decodedFrame->linesize,
                              0, v->codecCtx->height, out, v->convertedFrame->linesize);

                    // Check ringbuffer space and wait or continue
                    if (PaUtil_GetRingBufferWriteAvailable(v->rBuff) < 1) {
                        if (v->shutdown) break;
                        pthread_cond_wait(&v->buffNotEmpty, &v->buffLock);
                        if (v->shutdown) break;
                    }

                    pts = v->syncVideo(pts);
                    frame->pts = pts;
                    PaUtil_WriteRingBuffer(v->rBuff, frame, 1);

                    //

                } else {
                    printf("no frame. WTF\n");
                }
            }
        } else if (v->track != NULL && packet.stream_index == v->track->audioStream) {
            // This check might not be necessary
            if (ret < 0) {
                if (ret == AVERROR(EOF) || (v->container->pb && v->container->pb->eof_reached)) {
                    printf("eof\n");
                    v->eof = true;
                    // TODO : Fire EOF and wait for reset
                } else if (ret != AVERROR(EAGAIN)) {
                    printf("Error?\n");
                    // TODO : Fire Error and wait for reset
                }
            } else {
                AVPacketList *pkt;

                av_dup_packet(&packet);

                pkt = (AVPacketList *)av_malloc(sizeof(AVPacketList));
                pkt->pkt = packet;
                pkt->next = NULL;

                if (!v->audioQueue->tail) {
                    v->audioQueue->head = pkt;
                } else {
                    v->audioQueue->tail->next = pkt;
                }

                v->audioQueue->tail = pkt;
                v->audioQueue->count++;
                v->audioQueue->size += pkt->pkt.size;

            }
        } else {
            av_free_packet(&packet);
        }

        bool wakup = false;
        for (;;) {
            // Decode audio
            if (!v->track->work()) {
                if (v->track->packetConsumed && v->audioQueue->head != NULL) {
                    AVPacketList *p = v->audioQueue->head;
                    v->audioQueue->head = p->next;
                    if (!v->audioQueue->head) {
                        v->audioQueue->tail = NULL;
                    }
                    v->audioQueue->count--;
                    //v->audioQueue->size 
                    v->track->buffer(&p->pkt);
                    //av_free(p); // TODO : Need to free list, but this shit free also the packet
                    // Note : packet is freed by the track
                } else {
                    break;
                }
            } else {
                wakup = true;
            }
        }
        if (wakup) {
            pthread_cond_signal(&v->audio->queueHaveData);
        }
    }

    return NULL;
}

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

NativeVideo::~NativeVideo() {
    this->shutdown = true;

    printf("Destruct\n");
    pthread_cond_signal(&this->buffNotEmpty);
    pthread_join(this->threadDecode, NULL);
    printf("shutdowned");

    for (int i = 0; i < NATIVE_VIDEO_BUFFER_SAMPLES; i++) {
        clear_timer_by_id(&this->net->timersng, this->timers[i]->id, 1);
        delete this->timers[i];
    }

    av_free(this->convertedFrame);
    av_free(this->decodedFrame);
    avcodec_close(this->codecCtx);
    avformat_close_input(&(this->container));

    free(this->tmpFrame);
    free(this->frameBuffer);

    delete this->rBuff;
    printf("Finished\n");
}
