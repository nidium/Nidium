#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NativeAudio.h"

NativeAudio::NativeAudio(int bufferSize, int channels, int sampleRate)
    :tracks(NULL), tracksCount(0)
{
    pthread_cond_init(&this->bufferNotEmpty, NULL);
    pthread_mutex_init(&this->decodeLock, NULL);

    pthread_cond_init(&this->queueHaveData, NULL);
    pthread_mutex_init(&this->queueLock, NULL);

    // Save output parameters
    this->outputParameters = new NativeAudioParameters(bufferSize, channels, NativeAudio::FLOAT32, sampleRate);

    // Init libav
	av_register_all();

    // Init output buffer
    if (!(this->rBufferOutData = (float *)calloc(sizeof(float), (sizeof(float) * NATIVE_AVDECODE_BUFFER_SAMPLES * 2)))) {
        printf("Failed to init ouput ringbuffer\n");
        return;
    }

    if (0 < PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)&this->rBufferOut, 
            (NativeAudio::FLOAT32),
            NATIVE_AVDECODE_BUFFER_SAMPLES * 2,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        return;
    }

    printf("Iam init %p\n", this->tracks);
}

void NativeAudio::bufferData() {
    NativeAudioTracks *tracks = this->tracks;
    bool wrote = false;

    // XXX : Should I lock the track list while working on it?

    // Buffer data for each tracks
    while (tracks != NULL) 
    {
        if (tracks->curr != NULL && tracks->curr->opened) {
            if (tracks->curr->buffer() > 0) {
                wrote = true;
            }
        }
        tracks = tracks->next;
    }

    if (wrote == true) {
        // Send signal to decoding thread
        pthread_cond_signal(&this->bufferNotEmpty);
    }
}

bool NativeAudioNode::recurseGetData()
{
        SPAM(("in recurseGetData\n"));

        SPAM(("node %p, in count is %d\n", this, this->inCount));
        for (int i = 0; i < this->inCount; i++) {
            SPAM(("processing input #%d on %p\n", i, this));
            if (!this->nodeProcessed) {
                for (int j = 0; j < this->input[i]->count; j++) {
                    if (!this->input[i]->wire[j]->feedback) {
                        if (!this->input[i]->wire[j]->node->recurseGetData()) {
                            return false;
                        }
                    }
                }
            } else {
                SPAM(("  input already processed\n"));
            }
        }
        if (!this->nodeProcessed) {
            SPAM((" => processing data\n"));
            for (int i = 0; i < this->inCount; i++) {
                // Have multiple data on one input
                // add all input
                if (this->input[i]->count > 1) {
                    SPAM(("Merging input\n"));

                    // Reset buffer
                    if (!this->input[i]->haveFeedback) {
                        memset(this->frames[i], 0, this->params->bufferSize/this->params->channels);
                    }

                    // Merge all input
                    for (int j = 0; j < this->input[i]->count; j++) {
                        if (this->frames[i] != this->input[i]->wire[j]->frame) {
                            for (int k = 0; k < this->params->framesPerBuffer; k++) {
                                   this->frames[i][k] += this->input[i]->wire[j]->frame[k];
                            }
                        }
                    }
                } 
            }

            if (!this->process()) {
                SPAM(("  => FAILED!\n"));
                return false;
            }

            for (int i = 0; i < this->outCount; i++) {
                // Have multiple data on one ouput.
                // Copy output data to next bloc
                if (this->output[i]->count > 1) {
                    SPAM(("Copying output to next bloc\n"));
                    for (int j = 1; j < this->output[i]->count; j++) {
                        SPAM(("    ouput #%d from %p to %p\n", j, this->output[i]->wire[j]->frame, this->frames[i]));
                        if (this->output[i]->wire[j]->frame != this->frames[i]) {
                            for (int k = 0; k < this->params->framesPerBuffer; k++) {
                                this->output[i]->wire[j]->frame[k] = this->frames[i][k];
                            }
                        }
                    }
                } 
            }

            if (this->inCount >= 0) {
                SPAM(("    marked as  processed\n"));
                this->nodeProcessed = true;
            } else {
                SPAM(("    processed\n"));
                this->nodeProcessed = false;
            }
        } else {
            this->nodeProcessed = false;
            SPAM(("    already processed\n"));
        }

        return true;
}

void *NativeAudio::queueThread(void *args) {
    NativeAudio *audio = (NativeAudio *)args;
    bool wrote;

    wrote = false;

    while (true) {
        pthread_cond_wait(&audio->queueHaveData, &audio->queueLock);

        if (audio->output != NULL) {
            for (;;) {
                if (PaUtil_GetRingBufferWriteAvailable(&audio->rBufferOut) >= audio->outputParameters->framesPerBuffer) {
                    if (!audio->output->recurseGetData()) {
                        SPAM(("break cause of false\n"));
                        break;
                    } else {
                        wrote = true;
                    }
                    audio->output->nodeProcessed = false;
                    SPAM(("----------------------\n"));

                    if (wrote) {
                        //SPAM(("output data is at %p and %p (node = %p)\n",audio->output->inQueue[0]->frame, audio->output->inQueue[1]->frame, audio->output->inQueue[0]->node));
                        /*
                        for (int i = 0; i < 256; i++) {
                            SPAM(("write data = %f/%f\n", audio->output->frames[0][i], audio->output->frames[1][i]));
                        }
                        */
                        PaUtil_WriteRingBuffer(&audio->rBufferOut, audio->output->frames[0], audio->outputParameters->framesPerBuffer);
                        PaUtil_WriteRingBuffer(&audio->rBufferOut, audio->output->frames[1], audio->outputParameters->framesPerBuffer);
                        pthread_cond_signal(&audio->bufferNotEmpty);
                    }
                } else {
                    SPAM(("no more space to write\n"));
                    break;
                }
            }
            SPAM(("Finished FX queue\n"));
        }
    }
}

void *NativeAudio::decodeThread(void *args) {
    NativeAudio *audio = (NativeAudio *)args;

    while (true) 
    {
        NativeAudioTracks *tracks = audio->tracks;
        NativeAudioTrack *track;
        int haveEnought;

        haveEnought = 0;

        // Go through all the tracks that need data to be decoded
        while (tracks != NULL) 
        {
            haveEnought = 0;

            if (tracks->curr != NULL && tracks->curr->opened) {

                track = tracks->curr;

                // Loop as long as there is data to read and write
                for (;;)
                {
                    if (!track->work()) {
                        // No data to process, exit. 
                        break;
                    } 
                }

                if (track->avail() >= audio->outputParameters->framesPerBuffer) {
                    haveEnought++;
                }
            }
            tracks = tracks->next;
        }

        if (audio->tracksCount > 0 && haveEnought == audio->tracksCount) {
            if (PaUtil_GetRingBufferWriteAvailable(&audio->rBufferOut) > 0) {
                pthread_cond_signal(&audio->queueHaveData);
            } 
        }

        // Wait for more work to do
        pthread_cond_wait(&audio->bufferNotEmpty, &audio->decodeLock);
    }
}

int NativeAudio::openOutput() {
    const PaDeviceInfo *infos;
    PaDeviceIndex device;
    PaError error;
    PaStreamParameters paOutputParameters;

    // Start portaudio
    Pa_Initialize();
    
    // TODO : Device should be defined by user
    device = Pa_GetDefaultOutputDevice();
    if (device == paNoDevice) {// An error was encoutered
        return 1;
    }
    infos = Pa_GetDeviceInfo(device);

    // Set output parameters for PortAudio
    paOutputParameters.device = device; 
    paOutputParameters.channelCount = this->outputParameters->channels; 
    paOutputParameters.suggestedLatency = infos->defaultHighOutputLatency; 
    paOutputParameters.hostApiSpecificStreamInfo = 0; /* no api specific data */
    paOutputParameters.sampleFormat = paFloat32;

    // Try to open the output
    error = Pa_OpenStream(&this->outputStream,  
            0,  // No input
            &paOutputParameters,
            this->outputParameters->sampleRate,  
            this->outputParameters->framesPerBuffer,
            paNoFlag,  
            &NativeAudio::paOutputCallback,  
            (void *)this); 

    if (error)
    {
        printf("[NativeAudio] Error opening output, error code = %i\n", error);
        Pa_Terminate();
        return error;
    }
    printf("[NativeAudio] output opened with latency to %f\n", infos->defaultHighOutputLatency);

    Pa_StartStream(this->outputStream);
    return 0;
}

// The instance callback, where we have access to every method/variable of NativeAudio
int NativeAudio::paOutputCallbackMethod(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags)
{
    float *out = (float *)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    if (PaUtil_GetRingBufferReadAvailable(&this->rBufferOut) >= framesPerBuffer*2) {
        float tmp1[512] = {0};
        float tmp2[512] = {0};
        SPAM(("------------------------------------data avail\n"));
        PaUtil_ReadRingBuffer(&this->rBufferOut, &tmp1, framesPerBuffer);
        PaUtil_ReadRingBuffer(&this->rBufferOut, &tmp2, framesPerBuffer);
        for (i = 0; i < framesPerBuffer; i++)
        {
            //SPAM(("play data = %f/%f\n", tmp1[i], tmp2[i]));
            // Send sound to output
            *out++ = (float)tmp1[i];
            *out++ = (float)tmp2[i];
        }
    } else {
        SPAM(("-----------------------------------NO DATA\n"));
        for (i = 0; i < framesPerBuffer; i++)
        {
            // Send sound to output
            *out++ = 0;
            *out++ = 0;
        }
    }


    // Debuging purpose code
#if 0
    if (this->tracks != NULL && this->tracks->curr != NULL && this->tracks->curr->opened && this->tracks->curr->playing) {
        ring_buffer_size_t foo = PaUtil_GetRingBufferReadAvailable(&this->tracks->curr->rBufferOut);
        if ((unsigned long) foo < framesPerBuffer) {
            printf("WRONG can read %lu/%lu/%lu\n", foo, framesPerBuffer, PaUtil_GetRingBufferWriteAvailable(&this->tracks->curr->rBufferOut));
        } else {
            //printf("consuming %lu\n", framesPerBuffer);
        }
    }

    for (i = 0; i < framesPerBuffer; i++)
    {
        NativeAudioTracks *tracks = this->tracks;
        double left = 0, right = 0;

        // Loop all track and mix them 
        while (tracks != NULL) 
        {
            if (tracks->curr != NULL && tracks->curr->opened && tracks->curr->playing) {
                float tmp[2] = {0, 0};
                ring_buffer_size_t read;

                read = PaUtil_ReadRingBuffer(&tracks->curr->rBufferOut, &tmp, 1);
                dataRead = read > 0 ? true : false;
                totalRead += read;

                // TODO : Clip sound

                left += tmp[0];
                if (tracks->curr->nbChannel == 1) {
                    right += tmp[0];
                } else {
                    right += tmp[1];
                }
            }

            tracks = tracks->next;
        }

        /*
        if (this->tracksCount > 0) {
            left /= this->tracksCount;
            right /= this->tracksCount;
        }
        */

        // Send sound to output
        *out++ = (float)left;
        *out++ = (float)right;
    }
#endif

    // Data was read from ring buffer
    // need to decode more data
    /*
    if (dataRead) {
        if (totalRead < framesPerBuffer) {
            printf("FAILED TO READ ENOUGHT DATA %d\n", totalRead);
        }
    }
    pthread_cond_signal(&this->bufferNotEmpty);
    */
    pthread_cond_signal(&this->queueHaveData);

    return paContinue;

}

int NativeAudio::paOutputCallback( const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    return ((NativeAudio*)userData)->paOutputCallbackMethod(inputBuffer, outputBuffer,
        framesPerBuffer,
        timeInfo,
        statusFlags);
}

// TODO : replace sizeof by static size setuped on init
int NativeAudio::getSampleSize(int sampleFormat) {
    switch (sampleFormat) {
        case AV_SAMPLE_FMT_U8 :
            return sizeof(uint8_t);
        case AV_SAMPLE_FMT_NONE:
        case AV_SAMPLE_FMT_S16 :
            return sizeof(int16_t);
        case AV_SAMPLE_FMT_S32 :
            return sizeof(int32_t);
        case AV_SAMPLE_FMT_FLT :
            return sizeof(float);
        case AV_SAMPLE_FMT_DBL :
            return sizeof(double);
        default :
            return sizeof(int16_t); // This will mostly fail
    }
}

NativeAudioTrack *NativeAudio::addTrack() {
    NativeAudioTracks *tracks = (NativeAudioTracks *)malloc(sizeof(NativeAudioTracks));

    tracks->curr = new NativeAudioTrack(this->outputParameters);

    tracks->prev = NULL;
    tracks->next = this->tracks;

    if (this->tracks != NULL) {
        this->tracks->prev = tracks;
    }

    this->tracks = tracks;

    this->tracksCount++;

    return tracks->curr;
}

NativeAudioNode *NativeAudio::createNode(const char *name, int input, int output) {
    if (strcmp("source", name) == 0) {
        printf("added track\n");
        return this->addTrack();
    } else if (strcmp("wirdo", name) == 0) {
        return new NativeAudioNodeWirdo(input, output, this->outputParameters);
    } else if (strcmp("gain", name) == 0) {
        return new NativeAudioNodeGain(input, output, this->outputParameters);
    } else if (strcmp("mixer", name) == 0) {
        return new NativeAudioNodeMixer(input, output, this->outputParameters);
    } else if (strcmp("target", name) == 0) {
        if (this->openOutput() == 0) {
            this->output = new NativeAudioNodeTarget(input, output, this->outputParameters);
            return this->output;
        } else {
            printf("return null\n");
            return NULL;
        }
    }
    return NULL;
}

void NativeAudio::connect(NativeAudioNode::NodeLink *input, NativeAudioNode::NodeLink *output) {
    output->node->queue(input, output);
}

NativeAudio::~NativeAudio() {
    NativeAudioTracks *tracks = this->tracks;

    if (this->outputStream != NULL) {
        Pa_StopStream(this->outputStream); 
    }

    if (this->inputStream != NULL) {
        Pa_StopStream(this->inputStream);
    }

    Pa_Terminate(); 

    while (tracks != NULL) 
    {
        if (tracks->curr != NULL) {
            delete tracks->curr;

            tracks = tracks->next;
        }
    }

}

NativeAudioTrack::NativeAudioTrack(NativeAudioParameters *outputParameters) 
    : NativeAudioNode(0, 2, outputParameters), outputParameters(outputParameters), opened(false), playing(false), paused(false), 
      container(NULL), avctx(NULL), frameConsumed(true), packetConsumed(true), audioStream(-1),
      sCvt(NULL), fCvt(NULL), eof(false)
{
    // TODO : Throw exception instead of return

    // Alloc buffers memory
    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {// XXX : Pick better buffer size?
        return;
    }

    if (!(this->rBufferInData = malloc((sizeof(AVPacket) * this->outputParameters->framesPerBuffer) * 8))) {
        return;
    }

    av_init_packet(&this->tmpPacket);
    this->tmpFrame.size = 0;
    this->tmpFrame.data = NULL;

    // I/O packet list ring buffer
    // XXX : Is it better to use a linked list + mutex instead?
    if (0 < PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)&this->rBufferIn, 
            sizeof(AVPacket), 
            this->outputParameters->framesPerBuffer * 8,
            this->rBufferInData)) {
        printf("[NativeAudio] Failed to init input ringbuffer");
        return;
    }

    printf("track init\n");
}


int NativeAudioTrack::open(void *buffer, int size) 
{
    unsigned int i;
    AVCodec *codec;
    PaSampleFormat inputFmt;

    // If a previous file has been opened, close it
    if (this->container != NULL) {
        this->close(true);
    }

    // Setup libav context
    this->br = new BufferReader((uint8_t *)buffer, size);
    this->container = avformat_alloc_context();
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->br, this->br->read, NULL, NULL);

	// Open input 
	int ret = avformat_open_input(&this->container, "dummyFile", NULL, NULL);

	if (ret != 0) {
		char error[1024];
		av_strerror(ret, error, 1024);
		fprintf(stderr, "Couldn't open file : %s\n", error);
        return -2;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(this->container, NULL) < 0) {
		fprintf(stderr, "Couldn't find stream information\n");
        return -3;
	}

	// Dump information about file onto standard error
	av_dump_format(container, 0, "Memory input", 0);

	// Find first audio stream
	for (i = 0; i < this->container->nb_streams; i++) {
		if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && this->audioStream < 0) {
			this->audioStream = i;
		}
	}

	if(this->audioStream == -1) {
		fprintf(stderr, "Couldn't find audio stream\n");
		return -4;
	}

	// Find the apropriate codec and open it
	avctx = container->streams[audioStream]->codec;
	codec = avcodec_find_decoder(avctx->codec_id);

	if (!avcodec_open2(avctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not find or open the needed codec\n");
		return -5;
	}

    this->nbChannel = avctx->channels;

    // Frequency resampling
    if (avctx->sample_rate != this->outputParameters->sampleRate) {
        this->fCvt = new Resampler();
        this->fCvt->setup(avctx->sample_rate, this->outputParameters->sampleRate, avctx->channels, 32);

        if (!(this->fBufferOutData = (float *)malloc(NATIVE_RESAMPLER_BUFFER_SAMPLES * avctx->channels * NativeAudio::FLOAT32))) {
            fprintf(stderr, "Failed to init frequency resampler buffers");
            return -8;
        }

        this->fCvt->inp_count = this->fCvt->inpsize() / 2 -1;
        this->fCvt->inp_data = 0;

        this->fCvt->out_count = NATIVE_RESAMPLER_BUFFER_SAMPLES;
        this->fCvt->out_data = this->fBufferOutData;
    }

    // Init output buffer
    if (!(this->rBufferOutData = calloc(sizeof(float), (sizeof(float) * NATIVE_AVDECODE_BUFFER_SAMPLES * avctx->channels)))) {
        return -8;
    }

    if (0 < PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)&this->rBufferOut, 
            (NativeAudio::FLOAT32 * avctx->channels),
            NATIVE_AVDECODE_BUFFER_SAMPLES,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        return -8;
    }

    // Init sample converter
    switch (avctx->sample_fmt) {
        case AV_SAMPLE_FMT_U8 :
            inputFmt = paUInt8;
            break;
        case AV_SAMPLE_FMT_S16 :
            inputFmt = paInt16;
            break;
        case AV_SAMPLE_FMT_S32 :
            inputFmt = paInt32;
            break;
        case AV_SAMPLE_FMT_FLT :
            inputFmt = paFloat32;
            break;
        case AV_SAMPLE_FMT_NONE : /* Thoses cases will most likely fail */
        case AV_SAMPLE_FMT_DBL :
            inputFmt = paInt16;
            break;
        default :
            inputFmt = paInt16;
    }

    if (inputFmt != paFloat32) {
        if (!(this->sCvt = PaUtil_SelectConverter(inputFmt, paFloat32, paNoFlag))) {
                fprintf(stderr, "Failed to init sample resampling converter\n");
                return -9;
        }
    } 

    this->opened = true;

    // Everything is ok and setup
    // TODO : buffer more data and start decoding
    
    printf("ALL DONE\n");

    return 0;
}

int NativeAudioTrack::avail() {
    return (int) PaUtil_GetRingBufferReadAvailable(&this->rBufferOut);
}
int NativeAudioTrack::buffer() {
    int n = PaUtil_GetRingBufferWriteAvailable(&this->rBufferIn);
    //SPAM(("buffer() called %lu\n", n));
    if (n > 0) {
        return this->buffer(n);
    } else {
        return 0;
    }
}

int NativeAudioTrack::buffer(int n) {
    int i;

    i = 0;

    if (n > 0) {
        AVPacket pkt;

        av_init_packet(&pkt);

        for (i = 0; i < n; i++) 
        {
            int ret = av_read_frame(this->container, &pkt);
            if (ret < 0) {
                if (ret == AVERROR(EOF) || (this->container->pb && this->container->pb->eof_reached)) {
                    this->eof = true;
                } else if (ret != AVERROR(EAGAIN)) {
                    break;
                }
            } else {
                av_dup_packet(&pkt);
                if (0 == PaUtil_WriteRingBuffer(&this->rBufferIn, &pkt, 1)) {
                    return i;
                }
            }
        }
    }

    return i;
}

bool NativeAudioTrack::work() 
{
    if (!this->decode()) {
        return false;
    }

    // XXX : Refactor this inside resample()
    if (ring_buffer_size_t avail = PaUtil_GetRingBufferWriteAvailable(&this->rBufferOut) > 256) {
        int write;
        float *out;

        write = avail > this->outputParameters->framesPerBuffer ? this->outputParameters->framesPerBuffer : avail;
        out = (float *)malloc(write * this->nbChannel * NativeAudio::FLOAT32);

        write = this->resample((float *)out, write);

        PaUtil_WriteRingBuffer(&this->rBufferOut, out, write);

        free(out);

        return true;
    }

    return false;
}
bool NativeAudioTrack::decode() 
{
    // No data to read, return;
    if (PaUtil_GetRingBufferReadAvailable(&this->rBufferIn) < 1 && this->frameConsumed) {
        return false;
    }

    // Not last packet, get a new one
    if (this->packetConsumed) {
        PaUtil_ReadRingBuffer(&this->rBufferIn, (void *)&this->tmpPacket, 1);
        this->packetConsumed = false;
    }

    // No last frame, get a new one
    if (this->frameConsumed) {
        int gotFrame, len;
        AVFrame *tmpFrame;

        if (!(tmpFrame = avcodec_alloc_frame())) {
            printf("Failed to alloc frame\n");
            return false;
        }

        // Decode packet 
        len = avcodec_decode_audio4(avctx, tmpFrame, &gotFrame, &this->tmpPacket);

        if (len < 0) {
            fprintf(stderr, "Error while decoding\n");
            // TODO : Better error handling (events?)
            return false;
        } else if (len < this->tmpPacket.size) {
            //printf("Read len = %lu/%d\n", len, pkt.size);
            this->tmpPacket.data += len;
            this->tmpPacket.size -= len;
        } else {
            this->packetConsumed = true;
            av_free_packet(&this->tmpPacket);
        }

        if (this->tmpFrame.size < tmpFrame->linesize[0]) {
            if (this->tmpFrame.size != 0) {
                free(this->tmpFrame.data);
            }
            this->tmpFrame.size = tmpFrame->linesize[0];
            this->tmpFrame.data = (float *)malloc(tmpFrame->nb_samples * NativeAudio::FLOAT32 * this->nbChannel);
        }

        this->tmpFrame.nbSamples = tmpFrame->nb_samples;

        // Format resampling
        if (this->sCvt) {
            (*this->sCvt)(this->tmpFrame.data, 1, tmpFrame->data[0], 1, tmpFrame->nb_samples * this->nbChannel, NULL);
        } else {
            memcpy(this->tmpFrame.data, tmpFrame->data[0], tmpFrame->linesize[0]);
        }

        // Reset frequency converter input
        if (this->fCvt) {
            this->fCvt->inp_count = this->tmpFrame.nbSamples;
            this->fCvt->inp_data = this->tmpFrame.data;
        }

        av_free(tmpFrame);

        this->samplesConsumed = 0;
        this->frameConsumed = false;

        return true;
    } else {
        return true;
    }

    return true;
}
int NativeAudioTrack::resample(float *dest, int destSamples) {
    int channels = this->nbChannel;

    if (this->fCvt) {
        for (;;) {
            int sampleSize, copied;

            copied = 0;
            sampleSize = channels * NativeAudio::FLOAT32;

            if (this->fCvt->out_count == NATIVE_RESAMPLER_BUFFER_SAMPLES) {
                this->samplesConsumed = 0;
                this->fCvt->out_data = this->fBufferOutData;

                // Resample as much data as possible
                while (this->fCvt->out_count > 0 && this->fCvt->inp_count > 0) {
                    if (0 != this->fCvt->process()) {
                        printf("Failed to resample audio data\n");
                        return -1;
                    }
                }
            } 

            if (this->fCvt->out_count <= NATIVE_RESAMPLER_BUFFER_SAMPLES) {
                int write, avail;
                
                avail = NATIVE_RESAMPLER_BUFFER_SAMPLES - this->fCvt->out_count;
                write = destSamples > avail ? avail : destSamples;
                write -= copied;

                memcpy(
                        dest + copied * channels, 
                        this->fBufferOutData + this->samplesConsumed * channels, 
                        write * sampleSize
                    );

                this->samplesConsumed += write;
                this->fCvt->out_count += write;
                copied += write;

                if (copied == destSamples) {
                    return copied;
                }
            } 

            if (this->fCvt->inp_count == 0) {
                this->frameConsumed = true;
                if (!this->decode()) {
                    return copied;
                }
            } 
        }
    } else {
        int sampleSize, copied;

        sampleSize = this->nbChannel * NativeAudio::FLOAT32;
        copied = 0;

        for (;;) {
            int write, avail;

            avail = this->tmpFrame.nbSamples - this->samplesConsumed;
            write = destSamples > avail ? avail : destSamples;
            write -= copied;

            memcpy(
                    dest + copied * channels, 
                    this->tmpFrame.data + this->samplesConsumed * channels, 
                    write * sampleSize
                );

            copied += write;
            this->samplesConsumed += write;

            if (this->samplesConsumed == this->tmpFrame.nbSamples) {
                this->frameConsumed = true;
            }

            if (copied == destSamples) {
                return copied;
            } else if (this->frameConsumed) {
                this->decode();
            }
        }
    }

    return 0;
}

bool NativeAudioTrack::process() {
    if (!this->opened) {
        return false;
    }

    // Frame already processed, return;
    if (this->nodeProcessed) {
        return true;
    }

    // Make sure enought data is available
    if (this->outputParameters->framesPerBuffer > PaUtil_GetRingBufferReadAvailable(&this->rBufferOut)) {
        return false;
    }

    // Get the frame
    if (this->nbChannel > 1) { // More than 1 channel, need to split
        float *tmp;
        int j;

        j = 0;
        tmp = (float *)malloc(this->outputParameters->bufferSize);

        PaUtil_ReadRingBuffer(&this->rBufferOut, tmp, this->outputParameters->framesPerBuffer);

        for (int i = 0; i < this->outputParameters->framesPerBuffer; i++) {
            for (int c = 0; c < this->outCount; c++) {
                this->frames[c][i] = tmp[j];
                j++;
            }
        }

        SPAM(("track data is at %p and %p\n", this->frames[0], this->frames[1]));

        /*
        for (int i = 0; i < this->outputParameters->framesPerBuffer; i++) {
            SPAM(("frame data %f/%f\n", this->frames[0][i], this->frames[1][i]));
        }
        */

        free(tmp);
    } else {
        PaUtil_ReadRingBuffer(&this->rBufferOut, this->frames[0], this->outputParameters->framesPerBuffer);
    }

    return true;
}

void NativeAudioTrack::close(bool reset) {
    if (reset) {
        this->playing = false;
        this->paused = false;
        this->frameConsumed = true;
        this->packetConsumed = true;
        this->opened = false;
        this->audioStream = -1;

        PaUtil_FlushRingBuffer(&this->rBufferIn);
        PaUtil_FlushRingBuffer(&this->rBufferOut);

        memset(this->avioBuffer, 0, NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
    } else {
        free(this->rBufferInData);
        free(this->rBufferOutData);

        av_free(this->avioBuffer);
    }

    av_free(this->container->pb);

    if (!this->packetConsumed) {
        av_free_packet(&this->tmpPacket);
    }

    avformat_free_context(this->container);

    delete this->br;
}

void NativeAudioTrack::play() 
{
    this->playing = true;
}

NativeAudioTrack::~NativeAudioTrack() {
    this->close(false);
}
