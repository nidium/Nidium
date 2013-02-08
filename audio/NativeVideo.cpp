#include <stdio.h>
#include "NativeVideo.h"
#include "pa_ringbuffer.h"
#include <native_netlib.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

uint64_t NativeVideo::pktPts = AV_NOPTS_VALUE;

NativeVideo::NativeVideo(ape_global *n, char *file) 
    : net(n), cbk(NULL), cbkArg(NULL), shutdown(false), lastPts(0), pFormatCtx(NULL)
{
    int             numBytes;
    uint8_t         *buffer;
    AVCodec         *pCodec;


    pthread_cond_init(&this->buffNotEmpty, NULL);
    pthread_mutex_init(&this->buffLock, NULL);

    // FFmpeg stuff

    av_register_all();

    if (avformat_open_input(&pFormatCtx, file, NULL, NULL) != 0) {
        fprintf(stderr, "Failed to open video file");
        exit(1);
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information");
        exit(1);
    }

    av_dump_format(pFormatCtx, 0, file, 0);

    // Find the first video stream
    videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        fprintf(stderr, "No video stream");
        exit(1);
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodecCtx->get_buffer = NativeVideo::getBuffer;
    pCodecCtx->release_buffer = NativeVideo::releaseBuffer;

    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        exit(1);
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        fprintf(stderr, "Failed to open codec!\n");
        exit(1);
    }

    this->swsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                  pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGBA,
                                  SWS_BICUBIC, NULL, NULL, NULL);

    if (!this->swsCtx) {
        fprintf(stderr, "Failed to init video converter!\n");
        exit(1);
    }

    // Allocate video frame
    pFrame = avcodec_alloc_frame();

    // Allocate an AVFrame structure
    pFrameRGB = avcodec_alloc_frame();
    if (pFrameRGB == NULL || pFrame == NULL) {
        fprintf(stderr, "Failed to alloc frame\n");
        exit(1);
    }

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    if (buffer == NULL) {
        fprintf(stderr, "Failed to alloc buffer\n");
        exit(1);
    }

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);

    // NativeAV stuff
    this->frameTimer = (av_gettime() / 1000000.0);

    int buffSampleSize = (sizeof(uint8_t) * pCodecCtx->width * pCodecCtx->height* 4);

    //int buffSampleSize = sizeof(AVFrame);
    this->tmpFrame = (uint8_t *) malloc(buffSampleSize);

    this->rBuff = new PaUtilRingBuffer();
    this->buff = (uint8_t*) malloc(buffSampleSize * 8);
    printf("Alloc of %d\n", buffSampleSize*8);

    if (0 < PaUtil_InitializeRingBuffer(this->rBuff, 
            buffSampleSize, 
            8,
            this->buff)) {
        printf("Failed to init ringbuffer\n");
        exit(1);
    }
}

void NativeVideo::setCallback(VideoCallback cbk, void *arg) {
    this->cbk = cbk;
    this->cbkArg = arg;
}

void NativeVideo::play() {
    pthread_create(&this->threadDecode, NULL, NativeVideo::decode, this);
}

int NativeVideo::display(void *custom) {
    NativeVideo *v = (NativeVideo *)custom;

    // Read frame from ring buffer
    if (PaUtil_GetRingBufferReadAvailable(v->rBuff) < 1) {
        printf("Nothing to read from rBuff\n");
        // Schedule next wakeup in 10ms
        return 10;
    }

    PaUtil_ReadRingBuffer(v->rBuff, (void *)v->tmpFrame, 1);

    // Wakup decode thread
    if (PaUtil_GetRingBufferReadAvailable(v->rBuff) > 6) {
        pthread_cond_signal(&v->buffNotEmpty);
    }

    // Call the frame callback
    if (v->cbk != NULL) {
        v->cbk(v->pCodecCtx->width, v->pCodecCtx->height, v->tmpFrame, v->cbkArg);
    }

    return 0;
}

void *NativeVideo::decode(void *args) {
    NativeVideo *v = static_cast<NativeVideo *>(args);
    int             gotFrame;
    AVPacket        packet;
    AVStream *stream;
    double pts;

    pts = 0;
    stream = v->pFormatCtx->streams[v->videoStream];

    while (av_read_frame(v->pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == v->videoStream) {

            NativeVideo::pktPts = packet.pts;

            avcodec_decode_video2(v->pCodecCtx, v->pFrame, &gotFrame, &packet);

            if (packet.dts == AV_NOPTS_VALUE && v->pFrame->opaque && *(uint64_t*)v->pFrame->opaque != AV_NOPTS_VALUE) {
                pts = *(uint64_t*)v->pFrame->opaque;
            } else if (packet.dts != AV_NOPTS_VALUE) {
                pts = packet.dts;
            } else {
                pts = 0;
            }

            pts *= av_q2d(stream->time_base);

            if (gotFrame) {
                // Convert the image from its native format to RGBA 
                sws_scale(v->swsCtx,
                          v->pFrame->data, v->pFrame->linesize,
                          0, v->pCodecCtx->height, v->pFrameRGB->data, v->pFrameRGB->linesize);

                // Check ringbuffer space and wait or continue
                if (PaUtil_GetRingBufferWriteAvailable(v->rBuff) < 1) {
                    if (v->shutdown) break;
                    pthread_cond_wait(&v->buffNotEmpty, &v->buffLock);
                    if (v->shutdown) break;
                }

                // Write frame to ringbuffer
                PaUtil_WriteRingBuffer(v->rBuff, v->pFrameRGB->data[0], 1);

                // Schedule next display
                double frameDelay;

                if (pts != 0) {
                    v->videoClock = pts;
                } else {
                    pts = v->videoClock;
                }

                frameDelay = av_q2d(stream->time_base);
                frameDelay += v->pFrame->repeat_pict * (frameDelay * 0.5);
                v->videoClock = frameDelay;

                double delay = pts - v->lastPts;
                double actualDelay;

                v->lastPts = pts;
                v->frameTimer += delay;
                actualDelay = v->frameTimer - (av_gettime() / 1000000.0);

                if (actualDelay < 0) {
                    actualDelay = 0;
                }

                ape_timer *ctimer;
                ctimer = add_timer(&v->net->timersng, ((int)(actualDelay * 1000 + 0.5)), NativeVideo::display, v);
                ctimer->flags &= ~APE_TIMER_IS_PROTECTED;
                v->timer = ctimer->identifier;
            }
        }
        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // Free the RGB image
    //av_free(buffer); //tmp hack comment
    av_free(v->pFrameRGB);

    // Free the YUV frame
    av_free(v->pFrame);

    // Close the codec
    avcodec_close(v->pCodecCtx);

    // Close the video file
    avformat_close_input(&(v->pFormatCtx));

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
    pthread_cond_signal(&this->buffNotEmpty);
    pthread_join(this->threadDecode, NULL);
    free(this->tmpFrame);
    delete this->rBuff;
}
