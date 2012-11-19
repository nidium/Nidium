#include "pa_dither.h"
#include "pa_ringbuffer.h"
#include "pa_converters.h"
#include "NativeAudioNode.h"
#include "zita-resampler/resampler.h"
#include <NativeSharedMessages.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

NativeAudioNode::NativeAudioNode(int inCount, int outCount, NativeAudio *audio)
    : nodeProcessed(0), totalProcess(0), inQueueCount(0), inCount(inCount), outCount(outCount), audio(audio)
{
    SPAM(("NativeAudioNode init located at %p\n", this));
    int max;

    // Init exports array
    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        this->args[i] = NULL;
    }

    memset(this->input, 0, sizeof(NodeLink *) * 32);
    memset(this->output, 0, sizeof(NodeLink *) * 32);

    // Init node IO queue
    for (int i = 0; i < inCount; i++) {
        this->input[i] = new NodeLink(INPUT, i, this);
    }

    for (int i = 0; i < outCount; i++) {
        this->output[i] = new NodeLink(OUTPUT, i, this);
    }

    // Malloc node I/O frames
    max = (inCount > outCount ? inCount : outCount);
    this->frames = (float **)calloc(sizeof(float), max);

    for (int i = 0; i < max; i++) {
        this->frames[i] = NULL;
    }

    // More output than input
    // malloc() the frames
    if (outCount > inCount || inCount == 0) {
        int s = inCount == 0 ? 0 : outCount - inCount;
        for (int i = 0; i < outCount; i++) {
            printf("alloc buffer #%d\n", i);
            if (i >= s) {
                printf("=>malloc\n");
                this->frames[i] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
            }
        }
    }
}

NativeAudioNode::Message::Message(NativeAudioNode *node, void *source, void *dest, unsigned long size)
: node(node)
{
    this->source = malloc(size);
    this->size = size;
    this->dest = dest;

    memcpy(this->source, source, size);
}

NativeAudioNode::Message::~Message() {
    free(this->source);
}

bool NativeAudioNode::set(const char *name, ArgType type, void *value, unsigned long size) 
{
    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        if (this->args[i] != NULL && strcmp(name, this->args[i]->name) == 0) {
            if (this->args[i]->type == type) {
                this->post(NATIVE_AUDIO_NODE_SET, value, this->args[i]->ptr, size);
                return true;
            } /* else {
                 return false;
            } */
        }
    }

    return false;
}

void NativeAudioNode::updateFeedback(NativeAudioNode *nOut) 
{
    SPAM(("updateFeedback called\n"));
    for (int i = 0; i < this->inCount; i++) {
        for (int j = 0; j < this->input[i]->count; j++) {
            SPAM(("  checking input #%d wire %d; node = %p/%p\n", i, j, this->input[i]->wire[j]->node, nOut));
            if (!this->input[i]->wire[j]->feedback && 
                 this->input[i]->wire[j]->node == nOut) {
                SPAM(("=================== Its a feedback\n"));
                // It's a feedback
                this->input[i]->wire[j]->feedback = true;
                this->input[i]->haveFeedback = true;
                return;
            } else if (!this->input[i]->wire[j]->feedback) {
                SPAM(("Go back\n"));
                // Go back a node, and check
                this->input[i]->wire[j]->node->updateFeedback(nOut);
            }
        }
    }

    return;
}

void NativeAudioNode::updateWiresFrame(int channel, float *frame) {
    this->frames[channel] = frame;

    if (!this->output[channel]) return;

    int count = this->output[channel]->count;
    for (int i = 0; i < count; i++) 
    {
        this->output[channel]->wire[i]->node->updateWiresFrame(channel, frame);
    }

    return;
}

// XXX : Use mutex?
void NativeAudioNode::queue(NodeLink *in, NodeLink *out) 
{
    SPAM(("connect in node %p; out node %p\n", in->node, out->node));

    // First connect blocks frames
    if (in->node->frames[in->channel] == NULL) {
        SPAM(("Malloc frame\n"));
        in->node->frames[in->channel] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
        //this->frames[out->channel] = in->node->frames[in->channel];
    }

    if (out->count == 0 && in->count == 0 && in->node != out->node) {
        // Frame was previously assigned, update next outputs 
        if (this->frames[out->channel] != NULL) {
            free(this->frames[out->channel]);
            SPAM(("Update wires\n"));
            this->updateWiresFrame(out->channel, in->node->frames[in->channel]);
        }
        this->frames[out->channel] = in->node->frames[in->channel];
    } else if (out->count == 1 || in->count == 1 || in->node == out->node) {
        // Multiple input or output on same channel
        // need to use an internal buffer
        this->frames[out->channel] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
        SPAM(("Using custom frames\n"));
    } 

    // Then connect wires 
    NodeLink *tmp = this->input[out->channel];
    tmp->wire[tmp->count] = new NodeIO(in->node, in->node->frames[in->channel]);
    SPAM(("frame %p\n", in->node->frames[in->channel]));
    SPAM(("Assigning input on %p wire %d on channel %d to %p\n", this, tmp->count , out->channel, in->node));

    // And update input node wires (this is informative)
    tmp = in->node->output[in->channel];
    tmp->wire[tmp->count] = new NodeIO(out->node, out->node->frames[out->channel]);

    in->count++;
    out->count++;

    if (in->count == 1) {
        in->node->totalProcess++;
    }

    // Check if wire created a feedback somewhere
    this->updateFeedback(out->node);
}

// TODO : Use mutex before dsconnecting node
void NativeAudioNode::unqueue(NodeLink *input, NodeLink *output) 
{
    NodeLink *wiresIn, *wiresOut;
    
    wiresIn = this->input[output->channel];
    wiresOut = input->node->output[input->channel];

    // Find connecting wires and delete them
    for (int i = 0; i < wiresIn->count; i++) 
    {
        printf("checking %p/%p\n", wiresIn->wire[i]->node, input->node);
        if (wiresIn->wire[i]->node == input->node) {
            printf("deleted in node\n");
            delete wiresIn->wire[i];
            wiresIn->wire[i] = NULL;
            wiresIn->count--;
            break;
        }
    }

    for (int i = 0; i < wiresOut->count; i++) 
    {
        if (wiresOut->wire[i]->node == output->node) {
            printf("deleted out node\n");
            delete wiresOut->wire[i];
            wiresOut->wire[i] = NULL;
            wiresOut->count--;
            break;
        }
    }

    // Updates frames 
    if (this->input[output->channel]->count == 0) {
        printf("count = 0\n");
        if (this->frames[output->channel] == input->node->frames[input->channel]) {
            printf("free\n");
            this->frames[output->channel] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
        } else {
            free(this->frames[output->channel]);
        }
        input->node->totalProcess--;
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
                            SPAM(("FAILED\n"));
                            //return false;
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
                        memset(this->frames[i], 0, this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
                    } 

                    // Merge all input
                    for (int j = 0; j < this->input[i]->count; j++) {
                        if (this->frames[i] != this->input[i]->wire[j]->frame) {
                            SPAM(("    input #%d from %p to %p\n", j, this->input[i]->wire[j]->frame, this->frames[i]));
                            for (int k = 0; k < this->audio->outputParameters->framesPerBuffer; k++) {
                                   this->frames[i][k] += this->input[i]->wire[j]->frame[k];
                            }
                        }
                    }
                }  else if (this->input[i]->count == 0) {
                    if (this->frames[i] == NULL) {
                        SPAM(("Setting up nullBuff %d\n", i));
                       // this->frames[i] = this->audio->nullBuffer;
                        this->frames[i] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
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
                    for (int j = 0; j < this->output[i]->count; j++) {
                        if (this->output[i]->wire[j]->frame != this->frames[i]) {
                            SPAM(("    ouput #%d from %p to %p\n", j, this->output[i]->wire[j]->frame, this->frames[i]));
                            for (int k = 0; k < this->audio->outputParameters->framesPerBuffer; k++) {
                                this->output[i]->wire[j]->frame[k] = this->frames[i][k];
                            }
                        }
                    }
                } 
            }

            if (this->inCount >= 0) {
                SPAM(("    marked as  processed\n"));
                if (this->totalProcess > 1) {
                    this->nodeProcessed = 1;
                }
            } else {
                SPAM(("    processed\n"));
                this->nodeProcessed = false;
            }
        } else {
            this->nodeProcessed++;
            SPAM(("    already processed %d/%d\n", this->nodeProcessed, this->totalProcess));
            if (this->nodeProcessed >= this->totalProcess) {
                this->nodeProcessed = 0;
                SPAM(("    RESET %p\n", this));
            }
        }

        return true;
}



void NativeAudioNode::post(int msg, void *source, void *dest, unsigned long size) {
    this->audio->sharedMsg->postMessage((void *)new Message(this, source, dest, size), msg);
}

NativeAudioNode::~NativeAudioNode() {
    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        if (this->args[i] != NULL) {
            delete this->args[i];
        } 
    }

    // TODO : Call disconnect() to update tree 
    //        and free local _tmp_ buffer

    for (int i = 0; i < this->inCount; i++) 
    {
        for (int j = 0; j < this->input[i]->count) 
        {
            this->unqueue(this->input[i]->wire[j]->node, this->input[i]->wire[j]->node->output[);
        }
    }

    for (int i = 0; i < this->outCount; i++)
    {
        this->unqueue(this->output[i], this->output[i]->);
    }

    /*
    if (this->outCount > this->inCount || this->inCount == 0) {
        int s = this->inCount == 0 ? 0 : this->outCount - this->inCount;
        for (int i = 0; i < outCount; i++) {
            if (i >= s) {
                free(this->frames[i]);
            }
        }
    } else {
        for (int i = 0; i < this->inCount; i++) {
            if (this->input[i]->count == 0) {
                free(this->frames[i]);
            }
        }
    }
    */
}

NativeAudioNodeGain::NativeAudioNodeGain(int inCount, int outCount, NativeAudio *audio) 
    : NativeAudioNode(inCount, outCount, audio), gain(1)
{
    printf("gain init\n");
    this->args[0] = new ExportsArgs("gain", DOUBLE, &this->gain);
    this->args[1] = new ExportsArgs("gain", INT, &this->gain);
}

bool NativeAudioNodeGain::process() 
{
    SPAM(("|process called on gain\n"));
    for (int i = 0; i < this->audio->outputParameters->framesPerBuffer; i++) {
        for (int j = 0; j < this->inCount; j++) {
            this->frames[this->input[j]->channel][i] *= this->gain;
        }

        SPAM(("Gain data %f/%f\n", this->frames[0][i], this->frames[1][i]));
    }
    return true;
}

NativeAudioTrack::NativeAudioTrack(int out, NativeAudio *audio) 
    : NativeAudioNode(0, out, audio), opened(false), playing(false), stopped(false), 
      container(NULL), avctx(NULL), frameConsumed(true), packetConsumed(true), audioStream(-1),
      sCvt(NULL), fCvt(NULL), eof(false)
{
    // TODO : Throw exception instead of return

    // Alloc buffers memory
    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {// XXX : Pick better buffer size?
        return;
    }

    if (!(this->rBufferInData = malloc((sizeof(AVPacket) * this->audio->outputParameters->framesPerBuffer) * 8))) {
        return;
    }

    this->tmpPacket = new AVPacket();
    this->rBufferIn = new PaUtilRingBuffer();
    this->rBufferOut = new PaUtilRingBuffer();
    
    av_init_packet(this->tmpPacket);
    this->tmpFrame.size = 0;
    this->tmpFrame.data = NULL;

    // I/O packet list ring buffer
    // XXX : Is it better to use a linked list + mutex instead?
    if (0 < PaUtil_InitializeRingBuffer(this->rBufferIn, 
            sizeof(AVPacket), 
            this->audio->outputParameters->framesPerBuffer * 8,
            this->rBufferInData)) {
        printf("[NativeAudio] Failed to init input ringbuffer");
        return;
    }

    printf("track init\n");
}

NativeAudioTrack::BufferReader::BufferReader(uint8_t *buffer, unsigned long bufferSize) 
    : buffer(buffer), bufferSize(bufferSize), pos(0) {}

int NativeAudioTrack::BufferReader::read(void *opaque, uint8_t *buffer, int size) 
{
    BufferReader *reader = (BufferReader *)opaque;

    if (reader->pos + size > reader->bufferSize) {
        size = reader->bufferSize - reader->pos;
    }

    if (size > 0) {
        memcpy(buffer, reader->buffer + reader->pos, size);
        reader->pos += size;
    }

    return size;
}

int64_t NativeAudioTrack::BufferReader::seek(void *opaque, int64_t offset, int whence) 
{
    BufferReader *reader = (BufferReader *)opaque;
    int64_t pos = 0;

    switch(whence)
    {
        case AVSEEK_SIZE:
            return reader->bufferSize;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = reader->pos + offset;
        case SEEK_END:
            pos = reader->bufferSize - offset;
        default:
            return -1;
    }

    if( pos < 0 || pos > reader->bufferSize) {
        return -1;
    }

    reader->pos = pos;

    return pos;
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
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->br, this->br->read, NULL, this->br->seek);

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
    if (avctx->sample_rate != this->audio->outputParameters->sampleRate) {
        this->fCvt = new Resampler();
        this->fCvt->setup(avctx->sample_rate, this->audio->outputParameters->sampleRate, avctx->channels, 32);

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

    if (0 < PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)this->rBufferOut, 
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

    // As Loading from memory, immediately buffer data
    if (this->buffer() == 0) {
        SPAM(("WTF\n"));
        exit(1);
    } else {
        SPAM(("Sending buffer not empty\n"));
        pthread_cond_signal(&this->audio->bufferNotEmpty);
    }
    
    printf("ALL DONE\n");

    return 0;
}

int NativeAudioTrack::avail() {
    return (int) PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
}
int NativeAudioTrack::buffer() {
    int n = PaUtil_GetRingBufferWriteAvailable(this->rBufferIn);
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
                if (0 == PaUtil_WriteRingBuffer(this->rBufferIn, &pkt, 1)) {
                    return i;
                }
            }
        }
    }

    return i;
}

bool NativeAudioTrack::work() 
{
    if (this->stopped || !this->decode()) {
        return false;
    }

    // XXX : Refactor this inside resample()
    if (ring_buffer_size_t avail = PaUtil_GetRingBufferWriteAvailable(this->rBufferOut) > 256) {
        int write;
        float *out;

        write = avail > this->audio->outputParameters->framesPerBuffer ? this->audio->outputParameters->framesPerBuffer : avail;
        out = (float *)malloc(write * this->nbChannel * NativeAudio::FLOAT32);

        write = this->resample((float *)out, write);

        PaUtil_WriteRingBuffer(this->rBufferOut, out, write);

        free(out);

        return true;
    }

    return false;
}
bool NativeAudioTrack::decode() 
{
    // No data to read
    if (PaUtil_GetRingBufferReadAvailable(this->rBufferIn) < 1 && this->frameConsumed) {
        // Loading data from memory, try to get more
        if (this->br != NULL && !this->buffer()) {
            return false;
        }
    }

    // Not last packet, get a new one
    if (this->packetConsumed) {
        PaUtil_ReadRingBuffer(this->rBufferIn, (void *)this->tmpPacket, 1);
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
        len = avcodec_decode_audio4(avctx, tmpFrame, &gotFrame, this->tmpPacket);

        if (len < 0) {
            fprintf(stderr, "Error while decoding\n");
            // TODO : Better error handling (events?)
            return false;
        } else if (len < this->tmpPacket->size) {
            //printf("Read len = %lu/%d\n", len, pkt.size);
            this->tmpPacket->data += len;
            this->tmpPacket->size -= len;
        } else {
            this->packetConsumed = true;
            av_free_packet(this->tmpPacket);
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

            /*
        float *c = (float *)this->tmpFrame.data;
        for (int i = 0; i < this->tmpFrame.size/2; i++) {
            SPAM(("read data %f/%f\n", *c++, *c++));
        }
        */
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
        SPAM(("Not opened\n"));
        return false;
    }

    if (!this->playing) {
        SPAM(("Not playing\n"));
        memset(this->frames[0], 0, this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
        memset(this->frames[1], 0, this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
        return false;
    }

    // Frame already processed, return;
    if (this->nodeProcessed) {
        return true;
    }

    // Make sure enought data is available
    if (this->audio->outputParameters->framesPerBuffer > PaUtil_GetRingBufferReadAvailable(this->rBufferOut)) {
        return false;
    }

    // Get the frame
    if (this->nbChannel > 1) { // More than 1 channel, need to split
        float *tmp;
        int j;

        j = 0;
        // XXX : malloc each time could be avoided?
        tmp = (float *)malloc(this->audio->outputParameters->bufferSize);

        PaUtil_ReadRingBuffer(this->rBufferOut, tmp, this->audio->outputParameters->framesPerBuffer);

        for (int i = 0; i < this->audio->outputParameters->framesPerBuffer; i++) {
            for (int c = 0; c < this->outCount; c++) {
                this->frames[c][i] = tmp[j];
                j++;
            }
        }

        free(tmp);
    } else {
        PaUtil_ReadRingBuffer(this->rBufferOut, this->frames[0], this->audio->outputParameters->framesPerBuffer);
    }

    return true;
}

void NativeAudioTrack::close(bool reset) {
    if (reset) {
        this->playing = false;
        this->frameConsumed = true;
        this->packetConsumed = true;
        this->opened = false;
        this->audioStream = -1;

        PaUtil_FlushRingBuffer(this->rBufferIn);
        PaUtil_FlushRingBuffer(this->rBufferOut);

        //memset(this->avioBuffer, 0, NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
    } else {
        free(this->rBufferInData);
        free(this->rBufferOutData);

        //av_free(this->avioBuffer);
    }

    av_free(this->container->pb);

    if (!this->packetConsumed) {
        av_free_packet(this->tmpPacket);
    }

    avformat_free_context(this->container);

    delete this->br;
}

void NativeAudioTrack::play() 
{
    this->playing = true;
    if (this->stopped) {

        PaUtil_FlushRingBuffer(this->rBufferIn);
        PaUtil_FlushRingBuffer(this->rBufferOut);

        if (this->buffer() == 0) {
            SPAM(("WTF\n"));
            exit(1);
        } else {
            SPAM(("Sending buffer not empty\n"));
            pthread_cond_signal(&this->audio->bufferNotEmpty);
            pthread_cond_signal(&this->audio->queueHaveSpace);
        }
        
        this->stopped = false;
    }
}

void NativeAudioTrack::pause() 
{
    this->playing = false;
}

void NativeAudioTrack::stop() 
{
    this->playing = false;
    this->stopped = true;

    if (!this->packetConsumed) {
        av_free_packet(this->tmpPacket);
    }

    avformat_seek_file(this->container, this->audioStream, 0, 0, 0, 0);

    PaUtil_FlushRingBuffer(this->rBufferIn);
    PaUtil_FlushRingBuffer(this->rBufferOut);

    this->samplesConsumed = 0;
    this->frameConsumed = true;
    this->packetConsumed = true;
}


NativeAudioTrack::~NativeAudioTrack() {
    this->close(false);
}
