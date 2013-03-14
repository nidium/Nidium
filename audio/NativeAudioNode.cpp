#include "pa_dither.h"
#include "pa_ringbuffer.h"
#include "pa_converters.h"
#include "NativeAudio.h"
#include "NativeAudioNode.h"
#include "zita-resampler/resampler.h"
#include "NativeSharedMessages.h"
#include "Coro.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

NativeAudioNode::NativeAudioNode(int inCount, int outCount, NativeAudio *audio)
    : nullFrames(true), nodeProcessed(0), totalProcess(0), inQueueCount(0), inCount(inCount), outCount(outCount), audio(audio)
{
    SPAM(("NativeAudioNode init located at %p / %p\n", this, audio));
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
            if (i >= s) {
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

void NativeAudioNode::callback(NodeMessageCallback cbk, void *custom) 
{
    this->audio->sharedMsg->postMessage((void *)new CallbackMessage(cbk, this, custom), NATIVE_AUDIO_NODE_CALLBACK);
}

bool NativeAudioNode::set(const char *name, ArgType type, void *value, unsigned long size) 
{
    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        if (this->args[i] != NULL && strcmp(name, this->args[i]->name) == 0) {
            void *val;

            // If posted type and expected type are different
            // try to typecast the value (only few are supported)
            if (this->args[i]->type != type) {

                double doubleVal;

                switch (type) {
                    case DOUBLE:
                        return false;
                    break;
                    case INT : {
                        int *tmp = static_cast<int *>(value);
                        doubleVal = static_cast<double>(*tmp);
                        val = &doubleVal;
                        size = sizeof(double);
                    }
                    break;
                    default :
                        return false;
                    break;
                }
            } else {
                val = value;
            }

            this->post(NATIVE_AUDIO_NODE_SET, val, this->args[i]->ptr, size);
            return true;
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

void NativeAudioNode::queue(NodeLink *in, NodeLink *out) 
{
    SPAM(("connect in node %p; out node %p\n", in->node, out->node));

    pthread_mutex_lock(&this->audio->recurseLock);
    // First connect blocks frames
    if (in->node->frames[in->channel] == NULL) {
        SPAM(("Malloc frame\n"));
        in->node->frames[in->channel] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
        //this->frames[out->channel] = in->node->frames[in->channel];
    }

    if (out->count == 0 && in->count == 0 && in->node != out->node) {
        SPAM(("frame previously assigned\n"));
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

        SPAM(("Update wires\n"));
        this->updateWiresFrame(out->channel, this->frames[out->channel]);

        SPAM(("Using custom frames\n"));
    } 

    // Then connect wires 
    NodeLink *tmp = this->input[out->channel];
    // TODO FIXME : Do not rely on tmp->count. First try to find if a wire is NULL
    tmp->wire[tmp->count] = new NodeIO(in->node, in->node->frames[in->channel]);
    SPAM(("frame %p\n", in->node->frames[in->channel]));
    SPAM(("Assigning input on %p wire %d on channel %d to %p\n", this, tmp->count , out->channel, in->node));

    // And update input node wires 
    tmp = in->node->output[in->channel];
    tmp->wire[tmp->count] = new NodeIO(out->node, out->node->frames[out->channel]);

    in->count++;
    out->count++;

    if (in->count == 1) {
        in->node->totalProcess++;
    }

    // Check if wire created a feedback somewhere
    this->updateFeedback(out->node);

    pthread_mutex_unlock(&this->audio->recurseLock);
}

void NativeAudioNode::unqueue(NodeLink *input, NodeLink *output) 
{
    pthread_mutex_lock(&this->audio->recurseLock);
    NodeLink *wiresIn, *wiresOut;
    
    wiresIn = this->input[output->channel];
    wiresOut = input->node->output[input->channel];

    // Find connecting wires and delete them
    for (int i = 0; i < wiresIn->count; i++) 
    {
        if (wiresIn->wire[i] != NULL && wiresIn->wire[i]->node == input->node) {
            delete wiresIn->wire[i];
            wiresIn->wire[i] = NULL;
            // NOTE : This is commented because when a new wire will be connected, 
            // it will override the last wire. Need to fix queue().
            //wiresIn->count--; 
            break;
        }
    }

    for (int i = 0; i < wiresOut->count; i++) 
    {
        if (wiresOut->wire[i] != NULL && wiresOut->wire[i]->node == output->node) {
            delete wiresOut->wire[i];
            wiresOut->wire[i] = NULL;
            //wiresOut->count--;
            break;
        }
    }

    // Check if no remaning wire exist on the node channel
    bool empty = true;
    for (int i = 0; i < wiresIn->count; i++) {
        if (wiresIn->wire[i] != NULL) {
            empty = false;
        }
    }

    // Updates frames 
    if (empty) {
        SPAM(("count = 0\n"));
        if (this->frames[output->channel] == input->node->frames[input->channel]) {
            // Output node input channel is orpheline, 
            // create a new frame for the node
            this->frames[output->channel] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
            this->updateWiresFrame(output->channel, this->frames[output->channel]);
            if (this->frames[output->channel] != NULL) {
                this->resetFrame(output->channel);
            } else {
                printf("FRAME IS NULL, calloc failed\n");
                // Yes, calloc failed and I don't do anything
                // Because this will be automatically fixed when 
                // the node will be connected again
            }
            wiresIn->count = 0;
        } 
        input->node->totalProcess--;
    }

    pthread_mutex_unlock(&this->audio->recurseLock);
}

bool NativeAudioNode::recurseGetData()
{
        SPAM(("in recurseGetData\n"));

        SPAM(("node %p, in count is %d\n", this, this->inCount));
        for (int i = 0; i < this->inCount; i++) {
            SPAM(("processing input #%d on %p\n", i, this));
            if (!this->nodeProcessed) {
                for (int j = 0; j < this->input[i]->count; j++) {
                    if (this->input[i]->wire[j] != NULL && !this->input[i]->wire[j]->feedback) {
                        if (!this->input[i]->wire[j]->node->recurseGetData()) {
                            SPAM(("FAILED\n"));
                            // TODO : Check side effect of returning false (two source?)
                            // => After checking this block song for playing if one fail
                            // now recurseGetData return always true (side effect?)
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
                }  /*else if (this->input[i]->count == 0) {
                    // This point should not be reached 
                    if (this->frames[i] == NULL) {
                        SPAM(("Setting up nullBuff %d\n", i));
                        // this->frames[i] = this->audio->nullBuffer;
                        this->frames[i] = (float *)calloc(sizeof(float), this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
                    }
                }*/

                // Something is wrong :'(
                if (this->frames[i] == NULL) {
                    printf("Frame is null. don't process the node\n");
                    // Returning here will prevent NativeAudio from crashing
                    // But the audio queue might not continue to fully work
                    return true;
                }
            }

            if (!this->process()) {
                SPAM(("  => FAILED!\n"));
                // XXX : Returning true here, because we don't want
                // to stop the FX queue because one node cannot be processed
                // Note : 
                // - Only source return false
                // - Need to check for side effect when a source doesn't have enought data to read. I think the last frame is going to play as long as no new data is available
                return true;
            }

            for (int i = 0; i < this->outCount; i++) {
                // Have multiple data on one output.
                // Copy output data to next bloc
                if (this->output[i]->count > 1) {
                    SPAM(("Copying output to next bloc\n"));
                    for (int j = 0; j < this->output[i]->count; j++) {
                        if (this->output[i]->wire[j]->frame != this->frames[i]) {
                            SPAM(("    output #%d from %p to %p\n", j, this->output[i]->wire[j]->frame, this->frames[i]));
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
    //printf("Destroying node %p\n", this);
    // Let's disconnect the node
    for (int i = 0; i < this->inCount; i++) {
        int count = this->input[i]->count;
        for (int j = 0; j < count; j++) {
            if (this->input[i]->wire[j] != NULL && !this->input[i]->wire[j]->feedback) {
                int outCount = this->input[i]->wire[j]->node->outCount;
                //printf("outCount is %d\n", outCount);
                for (int k = 0; k < outCount; k++) {
                    //if (this->input[i]->wire[j] != NULL) {
                    //    printf("channel is %d\n", this->input[i]->wire[j]->node->output[k]->channel);
                    //} else {
                    //    printf("input %d wire %d is null\n", i, j);
                    //}
                    if (this->input[i]->wire[j] != NULL && i == this->input[i]->wire[j]->node->output[k]->channel) {
                        //printf("Disconnect output %d to input %d on wire %d\n", i, k, j);
                        this->audio->disconnect(this->input[i]->wire[j]->node->output[k], this->input[i]);
                    }
                }
            }
        }
    }

    for (int i = 0; i < this->outCount; i++) {
        int count = this->output[i]->count;
        for (int j = 0; j < count; j++) {
            if (this->output[i]->wire[j] != NULL && !this->output[i]->wire[j]->feedback) {
                int inCount = this->output[i]->wire[j]->node->inCount;
                //printf("inCount is %d\n", inCount);
                for (int k = 0; k < inCount; k++) {
                    if (this->output[i]->wire[j] != NULL && i == this->output[i]->wire[j]->node->input[k]->channel) {
                        //printf("Disconnect output %d to input %d on wire %d\n", i, k, j);
                        this->audio->disconnect(this->output[i], this->output[i]->wire[j]->node->input[k]);
                    }
                }
            }
        }
    }

    // Free all frames
    // TODO : This need to be checked
    if (this->outCount > this->inCount || this->inCount == 0) {
        int s = this->inCount == 0 ? 0 : this->outCount - this->inCount;
        for (int i = 0; i < outCount; i++) {
            if (i >= s && this->frames[i] != NULL) {
                free(this->frames[i]);
            }
        }
    } else {
        for (int i = 0; i < this->inCount; i++) {
            if (this->input[i]->count == 0 && this->frames[i] != NULL) {
                free(this->frames[i]);
            }
        }
    }
    free(this->frames);

    // And free all NodeLink
    for (int i = 0; i < this->inCount; i++) {
        delete this->input[i];
    }

    for (int i = 0; i < this->outCount; i++) {
        delete this->output[i];
    }

    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        if (this->args[i] != NULL) {
            delete this->args[i];
        } 
    }

    if (this == this->audio->output) {
        this->audio->output = NULL;
    }
}

NativeAudioNodeGain::NativeAudioNodeGain(int inCount, int outCount, NativeAudio *audio) 
    : NativeAudioNode(inCount, outCount, audio), gain(1)
{
    this->args[0] = new ExportsArgs("gain", DOUBLE, &this->gain);
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

NativeAudioNodeCustom::NativeAudioNodeCustom(int inCount, int outCount, NativeAudio *audio) 
    : NativeAudioNode(inCount, outCount, audio), cbk(NULL)
{
}

void NativeAudioNodeCustom::setCallback(NodeCallback cbk, void *custom)
{
    this->cbk = cbk;
    this->custom = custom;
}

bool NativeAudioNodeCustom::process()
{
    if (this->cbk != NULL) {
        NodeEvent ev;

        ev.data = this->frames;
        ev.size = this->audio->outputParameters->framesPerBuffer;
        ev.custom = this->custom;

        this->cbk(&ev);
    }

    return true;
}

NativeAudioTrack::NativeAudioTrack(int out, NativeAudio *audio, bool external) 
    : NativeAudioNode(0, out, audio), externallyManaged(external), 
      playing(false), stopped(false), loop(false),
      codecCtx(NULL), tmpPacket(NULL), 
      frameConsumed(true), packetConsumed(true), audioStream(-1),
      swrCtx(NULL), fCvt(NULL), eof(false)
{
    // TODO : Throw exception instead of return
    if (!external) {
        // Alloc buffers memory
        if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {// XXX : Pick better buffer size?
            return;
        }

        this->tmpPacket = new AVPacket();
        av_init_packet(this->tmpPacket);
    }

    this->rBufferOut = new PaUtilRingBuffer();
  
    this->tmpFrame.size = 0;
    this->tmpFrame.data = NULL;
    this->tmpFrame.nbSamples = 0;
}

int NativeAudioTrack::open(const char *src) 
{
    // If a previous file has been opened, close it
    if (this->container != NULL) {
        this->close(true);
    }

    this->reader = new NativeAVFileReader(src, this, this->audio->net);

    this->container = avformat_alloc_context();
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->reader, NativeAVFileReader::read, NULL, NativeAVFileReader::seek);

    return 0;
}

int NativeAudioTrack::openInit() 
{
    this->coro = Coro_new();
    Coro_startCoro_(this->mainCoro, this->coro, this, NativeAudioTrack::openInitCoro);
    return 0;
}

void NativeAudioTrack::openInitCoro(void *arg) 
{
#define RETURN_WITH_ERROR(err) \
thiz->sendEvent(SOURCE_EVENT_ERROR, err, false);\
thiz->close(true); \
Coro_switchTo_(thiz->coro, thiz->mainCoro);

    bool sendEvent = true;
    NativeAudioTrack *thiz = static_cast<NativeAudioTrack *>(arg);

    int ret;
    if ((ret = thiz->initStream()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    if ((ret = thiz->initInternal()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    pthread_cond_signal(&thiz->audio->bufferNotEmpty);

    Coro_switchTo_(thiz->coro, thiz->mainCoro);
#undef RETURN_WITH_ERROR
}

int NativeAudioTrack::open(void *buffer, int size) 
{
#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
this->close(true); \
return err;

    this->sendEvent(SOURCE_EVENT_BUFFERING, 100, false);

    // If a previous file has been opened, close it
    if (this->container != NULL) {
        this->close(true);
    }

    this->reader = new NativeAVBufferReader((uint8_t *)buffer, size);
    this->container = avformat_alloc_context();
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AVIO_BUFFER_SIZE, 0, this->reader, NativeAVBufferReader::read, NULL, NativeAVBufferReader::seek);

    int ret;
    if ((ret = this->initStream()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    if ((ret = this->initInternal()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    pthread_cond_signal(&this->audio->bufferNotEmpty);

    return 0;

#undef RETURN_WITH_ERROR
}

int NativeAudioTrack::initStream()
{
	// Open input 
	int ret = avformat_open_input(&this->container, "dummyFile", NULL, NULL);

	if (ret != 0) {
		char error[1024];
		av_strerror(ret, error, 1024);
		fprintf(stderr, "Couldn't open file : %s\n", error);
        return ERR_INTERNAL;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(this->container, NULL) < 0) {
		fprintf(stderr, "Couldn't find stream information\n");
        return ERR_NO_INFORMATION;
	}

	// Dump information about file onto standard error
	av_dump_format(this->container, 0, "Memory input", 0);

	// Find first audio stream
	for (unsigned int i = 0; i < this->container->nb_streams; i++) {
		if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			this->audioStream = i;
            break;
		}
	}

	if (this->audioStream == -1) {
		fprintf(stderr, "Couldn't find audio stream\n");
		return ERR_NO_AUDIO;
	}
    
    return 0;
}

int NativeAudioTrack::initInternal() 
{
    AVCodec *codec;

	// Find the apropriate codec and open it
	this->codecCtx = this->container->streams[this->audioStream]->codec;
	codec = avcodec_find_decoder(this->codecCtx->codec_id);

	if (!avcodec_open2(this->codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Could not find or open the needed codec\n");
		return ERR_NO_CODEC;
	}

    this->nbChannel = this->outCount;

    // Frequency resampling
    if (this->codecCtx->sample_rate != this->audio->outputParameters->sampleRate) {
        this->fCvt = new Resampler();
        this->fCvt->setup(this->codecCtx->sample_rate, this->audio->outputParameters->sampleRate, this->outCount, 32);

        if (!(this->fBufferOutData = (float *)malloc(NATIVE_RESAMPLER_BUFFER_SAMPLES * this->outCount * NativeAudio::FLOAT32))) {
            fprintf(stderr, "Failed to init frequency resampler buffers");
            return ERR_OOM;
        }

        this->fCvt->inp_count = this->fCvt->inpsize() / 2 -1;
        this->fCvt->inp_data = 0;

        this->fCvt->out_count = NATIVE_RESAMPLER_BUFFER_SAMPLES;
        this->fCvt->out_data = this->fBufferOutData;
    }

    // Init output buffer
    if (!(this->rBufferOutData = calloc(sizeof(float), (NativeAudio::FLOAT32 * NATIVE_AVDECODE_BUFFER_SAMPLES * this->outCount)))) {
        return ERR_OOM;
    }

    if (0 > PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)this->rBufferOut, 
            (NativeAudio::FLOAT32 * this->outCount),
            NATIVE_AVDECODE_BUFFER_SAMPLES,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        return ERR_OOM;
    }

    if (this->codecCtx->sample_fmt != AV_SAMPLE_FMT_FLT ||
        this->codecCtx->channel_layout != AV_CH_LAYOUT_STEREO) {
        int64_t channelLayout;

        // Channel layout is not set, use default one
        if (this->codecCtx->channel_layout == 0) {
            channelLayout = av_get_default_channel_layout(this->codecCtx->channels);
        } else {
            channelLayout = this->codecCtx->channel_layout;
        }

        this->swrCtx = swr_alloc_set_opts(NULL,
                AV_CH_LAYOUT_STEREO_DOWNMIX, AV_SAMPLE_FMT_FLT, this->codecCtx->sample_rate, 
                channelLayout, this->codecCtx->sample_fmt, this->codecCtx->sample_rate,
                0, NULL
            );
        if (!this->swrCtx || swr_init(this->swrCtx) < 0) {
            fprintf(stderr, "Failed to init sample resampling converter\n");
            return ERR_NO_RESAMPLING_CONVERTER;
        }
    } 

    this->opened = true;

    this->sendEvent(SOURCE_EVENT_READY, 0, false);

    return 0;
}

int NativeAudioTrack::avail() 
{
    return (int) PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
}

bool NativeAudioTrack::buffer() 
{
    if (this->reader->async) {
        // TODO : Do not rely on reader->pending
        // instead use NativeAudioTrack bufferPending
        if (this->reader->pending) {
            // Reader already trying to get data
            return false;
        }
        Coro_startCoro_(this->mainCoro, this->coro, this, NativeAudioTrack::bufferCoro);
        return false;
    } else {
        return this->bufferInternal();
    }
}

bool NativeAudioTrack::bufferInternal() 
{
    for (;;) {
        int ret = av_read_frame(this->container, this->tmpPacket);
        if (this->tmpPacket->stream_index == this->audioStream) {
            if (ret < 0) {
                if (ret == AVERROR_EOF || (this->container->pb && this->container->pb->eof_reached)) {
                    this->eof = true;
                } else if (ret != AVERROR(EAGAIN)) {
                    this->eof = true;
                    this->sendEvent(SOURCE_EVENT_ERROR, ERR_FAILED_READING, true);
                }
            } else {
                this->packetConsumed = false;
            }

            if (this->reader->async) {
                Coro_switchTo_(this->coro, this->mainCoro);
            } else {
                return ret >= 0;
            }
        } else {
            av_free_packet(this->tmpPacket);
        }
    }
}

void NativeAudioTrack::bufferCoro(void *arg) {
   (static_cast<NativeAudioTrack*>(arg))->bufferInternal();
}

void NativeAudioTrack::buffer(AVPacket *pkt) {
    this->tmpPacket = pkt;
    this->packetConsumed = false;
}

bool NativeAudioTrack::work() 
{
    if (this->doSeek) {
        if (this->reader->pending) {
            return false;
        }

        if (!this->reader->async) {
            this->seekInternal(this->doSeekTime);
        } else {
            Coro_startCoro_(this->mainCoro, this->coro, this, NativeAudioTrack::seekCoro);
            return false;
        } 
    }

    ring_buffer_size_t avail = PaUtil_GetRingBufferWriteAvailable(this->rBufferOut);

    if (avail < 256) {
        SPAM(("Work failed because not enought space is available to write decoded packet\n"));
        return false;
    }

    if (this->stopped || !this->decode()) {
        SPAM(("Work failed because track is stoped or decoding failed %d\n", this->stopped));
        return false;
    }

    // XXX : Refactor this inside resample()
    int write;
    float *out;

    write = avail > this->audio->outputParameters->framesPerBuffer ? this->audio->outputParameters->framesPerBuffer : avail;
    out = (float *)malloc(write * this->nbChannel * NativeAudio::FLOAT32);

    write = this->resample((float *)out, write);

    PaUtil_WriteRingBuffer(this->rBufferOut, out, write);

    free(out);

    return true;
}
bool NativeAudioTrack::decode() 
{
#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, true);\
return false;

    // No last packet, get a new one
    if (this->packetConsumed) {
        if (this->externallyManaged) {
            return false;
        } else {
            if (!this->buffer()) {
                // XXX : Return with error for eof?
                return false;
            }
        }
    }

    // No last frame, get a new one
    if (this->frameConsumed) {
        int gotFrame, len;
        AVFrame *tmpFrame;

        if (!(tmpFrame = avcodec_alloc_frame())) {
            SPAM(("Failed to alloc frame\n"));
            RETURN_WITH_ERROR(ERR_OOM);
        }

        // Decode packet 
        len = avcodec_decode_audio4(this->codecCtx, tmpFrame, &gotFrame, this->tmpPacket);

        //printf("sample_rate %d\n", tmpFrame->sample_rate);
        /*
        uint16_t *c = (uint16_t *)tmpFrame->data[0];
        for (int i = 0; i < tmpFrame->nb_samples; i++) {
            printf("read data %d/%d\n", *c++, *c++);
        }
        printf("------------------\n");
        */

        if (len < 0) {
            RETURN_WITH_ERROR(ERR_DECODING);
            return false;
        } else if (len < this->tmpPacket->size) {
            //printf("Read len = %u/%d\n", len, this->tmpPacket->size);
            this->tmpPacket->data += len;
            this->tmpPacket->size -= len;
        } else {
            this->packetConsumed = true;
            av_free_packet(this->tmpPacket);
        }

        /*
        int dataSize = av_samples_get_buffer_size(NULL, this->codecCtx->channels, tmpFrame->nb_samples, this->codecCtx->sample_fmt, 1);
        this->clock += (double)dataSize / (double( 2 * this->codecCtx->channels * this->codecCtx->sample_rate));
        */

        // Didn't got a frame let's try next time
        if (gotFrame == 0) {
            printf("============================== I DIDN'T GOT A FRAME ======================\n");
            return false;
        }

        if (this->tmpPacket->pts != AV_NOPTS_VALUE) {
            this->clock = av_q2d(this->container->streams[this->audioStream]->time_base) * this->tmpPacket->pts;
        }

        if (this->tmpFrame.nbSamples < tmpFrame->nb_samples) {
            if (this->tmpFrame.size != 0) {
                free(this->tmpFrame.data);
            }
            this->tmpFrame.size = tmpFrame->linesize[0];
            this->tmpFrame.data = (float *)malloc(tmpFrame->nb_samples * NativeAudio::FLOAT32 * 2);// Right now, source output is always stereo
            if (this->tmpFrame.data == NULL) {
                RETURN_WITH_ERROR(ERR_OOM);
            }
        }

        if (this->swrCtx) {
            uint8_t *out[1];
            out[0] = (uint8_t *)this->tmpFrame.data;

            const uint8_t **in = (const uint8_t **)tmpFrame->data;

            int len = swr_convert(this->swrCtx, out, tmpFrame->nb_samples, in, tmpFrame->nb_samples);
            this->tmpFrame.nbSamples = len;
        } else {
            memcpy(this->tmpFrame.data, tmpFrame->data[0], tmpFrame->linesize[0]);
            this->tmpFrame.nbSamples = tmpFrame->nb_samples;
        }


        /*
        float *c = (float *)this->tmpFrame.data;
        for (int i = 0; i < this->tmpFrame.nbSamples; i++) {
            printf("f32 data %f/%f\n", *c++, *c++);
        }
        */

        // Reset frequency converter input
        if (this->fCvt) {
            this->fCvt->inp_count = this->tmpFrame.nbSamples;
            this->fCvt->inp_data = this->tmpFrame.data;
        }

        av_free(tmpFrame);

        this->samplesConsumed = 0;
        this->frameConsumed = false;
    } 

    return true;
#undef RETURN_WITH_ERROR
}
int NativeAudioTrack::resample(float *dest, int destSamples) {
    int channels = this->nbChannel;

    if (this->fCvt) {
        int copied = 0;
        for (;;) {
            int sampleSize;

            sampleSize = channels * NativeAudio::FLOAT32;

            // Output is empty
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
                return copied;
                /* 
                 * No, do not try to decode more, because
                 * we might decode more than destSamples
                 *
                if (!this->decode()) {
                    return copied;
                }
                */
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
                if (!this->decode()) {
                    return copied;
                }
            }
        }
    }

    return 0;
}

double NativeAudioTrack::getClock() {
    ring_buffer_size_t queuedTrack = PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
    ring_buffer_size_t queuedAudio = PaUtil_GetRingBufferReadAvailable(this->audio->rBufferOut);

    double coef = this->audio->outputParameters->sampleRate * av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);

    double delay = ((double)queuedTrack * NativeAudio::FLOAT32 * this->outCount) / (coef * this->outCount);
    //double audioBuffer = ((double)queuedAudio * NativeAudio::FLOAT32 * this->outCount) / (coef * this->audio->outputParameters->channels);
    //printf("queue=%f audiobuff=%f\n", delay, audioBuffer);
    delay += this->audio->getLatency();

    // TODO : This can be more accurate by calculating how much data is in NativeAudio buffer
    // but i couldn't figure how to do it right (yet)
    return this->clock - delay + 0.28;
}

void NativeAudioTrack::sync(double ts) 
{
    double diff = ts - this->getClock();
    printf("dropping %f seconds ", diff);

    ring_buffer_size_t del = (diff * (this->audio->outputParameters->sampleRate * NativeAudio::FLOAT32) * this->outCount / (NativeAudio::FLOAT32 * this->outCount));
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
    printf(" avail = %lu dropped=%lu\n", avail, del > avail ? avail : del);
    PaUtil_AdvanceRingBufferReadIndex(this->rBufferOut, del > avail ? avail : del);
}

void NativeAudioNode::resetFrames() {
    if (!this->nullFrames) {
        for (int i = 0; i < this->outCount; i++) {
            this->resetFrame(i);
        }
        this->nullFrames = true;
    }
}

void NativeAudioNode::resetFrame(int channel) 
{
    memset(this->frames[channel], 0, this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
}

void NativeAudioTrack::seek(double time) 
{
    if (!this->opened || this->doSeek) {
        return;
    }

    this->doSeekTime = time;
    this->doSeek = true;
}

void NativeAudioTrack::seekCoro(void *arg) 
{
    NativeAudioTrack *source = static_cast<NativeAudioTrack *>(arg);
    source->seekInternal(source->doSeekTime);
}
void NativeAudioTrack::seekInternal(double time) 
{
    if (this->externallyManaged) {
        avcodec_flush_buffers(this->codecCtx);
        PaUtil_FlushRingBuffer(this->rBufferOut);
        this->resetFrames();
        this->eof = false;
    } else {
        int64_t target = 0;
        int flags = 0;
        double clock = this->getClock();

        flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;

        target = time * AV_TIME_BASE;

        target = av_rescale_q(target, AV_TIME_BASE_Q, this->container->streams[this->audioStream]->time_base);

        if (av_seek_frame(this->container, this->audioStream, target, flags) >= 0) {
            avcodec_flush_buffers(this->codecCtx);
            PaUtil_FlushRingBuffer(this->rBufferOut);
            this->resetFrames();
            if (this->eof && flags == AVSEEK_FLAG_BACKWARD) {
                this->eof = false;
            }
        } else {
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
        }
    }

    this->doSeek = false;

    if (this->reader->async) {
        Coro_switchTo_(this->coro, this->mainCoro);
    } 
}

bool NativeAudioTrack::process() {
    if (!this->opened) {
        SPAM(("Not opened\n"));
        return false;
    }

    if (!this->playing) {
        SPAM(("Not playing\n"));
        this->resetFrames();
        return false;
    }

    // Frame already processed, return;
    if (this->nodeProcessed) {
        return true;
    }

    // Make sure enought data is available
    if (this->audio->outputParameters->framesPerBuffer > PaUtil_GetRingBufferReadAvailable(this->rBufferOut)) {
        this->resetFrames();
        SPAM(("Not enought to read\n"));
        // EOF reached, send message to NativeAudio
        if (this->eof) {
            SPAM(("     => EOF\n"));
            if (this->loop) {
                this->seek(0);
            } 
            this->sendEvent(SOURCE_EVENT_EOF, 0, true);
        } 
        return false;
    }

    // Get the frame
    if (this->nbChannel > 1) { // More than 1 channel, need to split
        float *tmp;
        int j;

        j = 0;
        // TODO : malloc each time could be avoided?
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

    this->nullFrames = false;

    return true;
}

void NativeAudioTrack::close(bool reset) {
    if (this->opened) {
        free(this->rBufferOutData);

        if (!this->packetConsumed) {
            av_free_packet(this->tmpPacket);
        }

        if (!this->externallyManaged) {
            av_free(this->container->pb);
            avformat_free_context(this->container);
            delete this->reader;
        }
        swr_free(&this->swrCtx);
        PaUtil_FlushRingBuffer(this->rBufferOut);
        avcodec_close(this->codecCtx);
    }

    if (this->tmpFrame.data != NULL) {
        free(this->tmpFrame.data);
        this->tmpFrame.data = NULL;
    }
    this->tmpFrame.size = 0;
    this->tmpFrame.nbSamples = 0;

    if (this->fCvt != NULL) {
        delete this->fCvt;
        free(this->fBufferOutData);
        this->fCvt = NULL;
    }

    if (reset) {
        this->playing = false;
        this->frameConsumed = true;
        this->packetConsumed = true;
        this->opened = false;
        this->audioStream = -1;

        //memset(this->avioBuffer, 0, NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
    } else {
        delete this->rBufferOut;
        //av_free(this->avioBuffer);
    }
}

void NativeAudioTrack::play() 
{
    if (!this->opened) {
        return;
    }

    this->playing = true;
    this->stopped = false;

    this->sendEvent(SOURCE_EVENT_PLAY, 0, false);
}

void NativeAudioTrack::pause() 
{
    if (!this->opened) {
        return;
    }

    this->playing = false;
    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void NativeAudioTrack::stop() 
{
    if (!this->opened) {
        return;
    }

    this->playing = false;
    this->stopped = true;
    SPAM(("Stoped\n"));

    if (!this->packetConsumed) {
        av_free_packet(this->tmpPacket);
    }

    if (!this->externallyManaged) {
        this->seek(0);
    }

    PaUtil_FlushRingBuffer(this->rBufferOut);

    this->samplesConsumed = 0;
    this->frameConsumed = true;
    this->packetConsumed = true;

    this->resetFrames();

    avcodec_flush_buffers(this->codecCtx);

    this->sendEvent(SOURCE_EVENT_STOP, 0, false);
}


NativeAudioTrack::~NativeAudioTrack() {
    this->close(false);
}
