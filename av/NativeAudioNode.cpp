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

#define NODE_IO_FOR(i, io) int I = 0;\
while (I < io->count) { \
if (io->wire[i] != NULL) {\
I++;
#define NODE_IO_FOR_END(i) }i++;}

NativeAudioNode::NativeAudioNode(int inCount, int outCount, NativeAudio *audio)
    : nullFrames(true), processed(false), inCount(inCount), outCount(outCount), 
      audio(audio), doNotProcess(false)
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
            if (i >= s) {
                this->frames[i] = this->newFrame();
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

// XXX : This need to be checked again. 
// Since many breaking changes have been introduced
void NativeAudioNode::updateFeedback(NativeAudioNode *nOut) 
{
    //SPAM(("updateFeedback called\n"));
    for (int i = 0; i < this->inCount; i++) {
        for (int j = 0; j < this->input[i]->count; j++) {
            //SPAM(("  checking input #%d wire %d; node = %p/%p\n", i, j, this->input[i]->wire[j]->node, nOut));
            if (!this->input[i]->wire[j]->feedback && 
                 this->input[i]->wire[j]->node == nOut) {
                //SPAM(("=================== Its a feedback\n"));
                // It's a feedback
                this->input[i]->wire[j]->feedback = true;
                this->input[i]->haveFeedback = true;
                return;
            } else if (!this->input[i]->wire[j]->feedback) {
                //SPAM(("Go back\n"));
                // Go back a node, and check
                this->input[i]->wire[j]->node->updateFeedback(nOut);
            }
        }
    }

    return;
}

void NativeAudioNode::updateWiresFrame(int channel, float *frame) {
    if (this->frames[channel] != NULL && this->isFrameOwner(this->frames[channel])) {
        return;
    }

    this->frames[channel] = frame;

    if (!this->output[channel]) {
        return;
    }

    int count = this->output[channel]->count;
    for (int i = 0; i < count; i++) 
    {
        this->output[channel]->wire[i]->node->updateWiresFrame(channel, frame);
    }

    return;
}

bool NativeAudioNode::queue(NodeLink *in, NodeLink *out) 
{
    SPAM(("connect in node %p; out node %p\n", in->node, out->node));
    NodeIO **inLink;
    NodeIO **outLink;

    // First, make sure we have enought space to connect wire
    inLink = this->getWire(this->input[out->channel]);
    outLink = this->getWire(in->node->output[in->channel]);

    if (inLink == NULL || outLink == NULL) {
        return false;
    }

    pthread_mutex_lock(&this->audio->recurseLock);
    // Connect blocks frames
    if (in->node->frames[in->channel] == NULL) {
        SPAM(("Malloc frame\n"));
        in->node->frames[in->channel] = this->newFrame();
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

        this->frames[out->channel] = this->newFrame();

        SPAM(("Update wires\n"));
        this->updateWiresFrame(out->channel, this->frames[out->channel]);

        SPAM(("Using custom frames\n"));
    } 

    // Then connect wires 
    *inLink = new NodeIO(in->node, in->node->frames[in->channel]);

    // And update input node wires 
    *outLink = new NodeIO(out->node, out->node->frames[out->channel]);

    in->count++;
    out->count++;

    // Check if wire created a feedback somewhere
    this->updateFeedback(out->node);

    pthread_mutex_unlock(&this->audio->recurseLock);

    return true;
}

bool NativeAudioNode::unqueue(NodeLink *input, NodeLink *output) 
{
    pthread_mutex_lock(&this->audio->recurseLock);
    NodeLink *wiresIn, *wiresOut;
    int count;
    
    wiresIn = this->input[output->channel];
    wiresOut = input->node->output[input->channel];

    // Find connecting wires and delete them
    count = wiresIn->count;
    for (int i = 0; i < count; i++) 
    {
        if (wiresIn->wire[i] != NULL && wiresIn->wire[i]->node == input->node) {
            delete wiresIn->wire[i];
            wiresIn->wire[i] = NULL;
            wiresIn->count--; 
            break;
        }
    }

    count = wiresOut->count;
    for (int i = 0; i < count; i++) 
    {
        if (wiresOut->wire[i] != NULL && wiresOut->wire[i]->node == output->node) {
            delete wiresOut->wire[i];
            wiresOut->wire[i] = NULL;
            wiresOut->count--;
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

    if (empty) {
        // Output node input channel is orpheline, set his frame to null
        // If the frame is needed later, the fx queue will handle it
        if (this->frames[output->channel] != NULL && this->isFrameOwner(this->frames[output->channel])) {
            free(this->frames[output->channel]);
        }
        this->frames[output->channel] = NULL;
        // Forward update the queue
        this->updateWiresFrame(output->channel, this->frames[output->channel]);
        wiresIn->count = 0;
    }

    pthread_mutex_unlock(&this->audio->recurseLock);
    return true;
}

void NativeAudioNode::processQueue()
{
    SPAM(("process queue on %p\n", this));
    // Let's go for a new round.
    // First mark all output as unprocessed
    for (int i = 0; i < this->outCount ; i++) {
        int j = 0;
        NODE_IO_FOR(j, this->output[i]) 
            SPAM(("     Marking output at %p as unprocessed (%p)\n", this->output[i]->wire[j]->node, this));
            this->output[i]->wire[j]->node->processed = false;
        NODE_IO_FOR_END(j)
    }

    // Do we have all data we need to process this node?
    for (int i = 0; i < this->inCount; i++) {
        int j = 0;
        NODE_IO_FOR(j, this->input[i]) 
            if (!this->input[i]->wire[j]->node->processed) {
                SPAM(("     Input %p havn't been processed, return\n", this->input[i]->wire[j]->node));
                // Needed data havn't been processed yet. Return.
                return;
            } else {
                SPAM(("    Input at %p is already processed\n", this->input[i]->wire[j]->node));
            }
        NODE_IO_FOR_END(j)
    }

    // Some sanity check and merge input if needed
    for (int i = 0; i < this->inCount; i++) {
        // Something is wrong (ie : node is not connected)
        if (this->frames[i] == NULL) {
            SPAM(("     => Found a NULL frame. Fixing it\n"));
            this->frames[i] = this->newFrame();
            this->updateWiresFrame(i, this->frames[i]);
        }

        // Have multiple data on one input
        // add all input
        if (this->input[i]->count > 1) {
            // Reset buffer
            if (!this->input[i]->haveFeedback) {
                memset(this->frames[i], 0, this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels);
            } 

            // Merge all input
            int j = 0;
            NODE_IO_FOR(j, this->input[i]) 
                if (this->frames[i] != this->input[i]->wire[j]->frame) {
                    SPAM(("     Merging input #%d from %p to %p\n", this->input[i]->channel, this->input[i]->wire[j]->node, this));
                    SPAM(("     frames=%p from %p\n", this->frames[i], this->input[i]->wire[j]->frame));
                    for (int k = 0; k < this->audio->outputParameters->framesPerBuffer; k++) {
                        this->frames[i][k] += this->input[i]->wire[j]->frame[k];
                    }
                } 
                NODE_IO_FOR_END(j)
        }  
    }

    if (!this->process()) {
        SPAM(("Failed to process node at %p\n", this));
        this->processed = true;
        return; // XXX : This need to be double checked
    }

    for (int i = 0; i < this->outCount; i++) {
        // Have multiple data on one output.
        // Copy output data to next bloc
        if (this->output[i]->count > 1) {
            int j = 0;
            NODE_IO_FOR(j, this->output[i])
                if (this->output[i]->wire[j]->frame != this->frames[i]) {
                    for (int k = 0; k < this->audio->outputParameters->framesPerBuffer; k++) {
                        this->output[i]->wire[j]->frame[k] = this->frames[i][k];
                    }
                }
            NODE_IO_FOR_END(j)
        } 
    }

    SPAM(("Marking node %p as processed\n", this));
    this->processed = true;

    // Go process next outputs
    int count = 0;
    for (int i = 0; i < this->outCount; i++) {
        int j = 0;
        NODE_IO_FOR(j, this->output[i])
            if (!this->output[i]->wire[j]->node->processed) {
                this->output[i]->wire[j]->node->processQueue();
            }
            if (this->output[i]->wire[j]->node->processed) {
                count++;
            }
        NODE_IO_FOR_END(j)
    }

    SPAM(("----- processQueue on node %p finished\n", this));
}

#define FRAME_SIZE this->audio->outputParameters->bufferSize/this->audio->outputParameters->channels
float *NativeAudioNode::newFrame()
{
    float *ret = (float *)malloc(sizeof(float) * FRAME_SIZE + sizeof(void *));
    if (ret != NULL) {
        // Store at the end of the frame array
        // a pointer to the frame owner
        ptrdiff_t addr = reinterpret_cast<ptrdiff_t>(this);
        void *p = (void *)addr;
        float *tmp = &ret[FRAME_SIZE];
        memcpy(tmp, &p, sizeof(void *));
    }
    return ret;
}
#undef FRAME_SIZE

void NativeAudioNode::post(int msg, void *source, void *dest, unsigned long size) {
    this->audio->sharedMsg->postMessage((void *)new Message(this, source, dest, size), msg);
}

NativeAudioNode::~NativeAudioNode() {
    // Let's disconnect the node
    SPAM(("NativeAudioNode destructor %p\n", this));

    // Disconnect algorithm : 
    //  Follow each input and output wire to connected node
    //  From that node, delete all wire connected to this node
    //  Then delete the wire that drived us to that node

    // Disconnect all inputs
    SPAM(("--- Disconnect inputs\n"));
    for (int i = 0; i < this->inCount; i++) {
        int count = this->input[i]->count;
        SPAM(("    node have %d input\n", count));
        for (int j = 0; j < count; j++) {
            if (this->input[i]->wire[j] != NULL) { // Got a wire to a node
                NativeAudioNode *outNode = this->input[i]->wire[j]->node;
                SPAM(("    found a wire to node %p\n", outNode));
                SPAM(("    output node have %d output\n", outCount));
                for (int k = 0; k < outNode->outCount; k++) { // Go trought each output and wire
                    int wireCount = outNode->output[k]->count;
                    SPAM(("        #%d wire = %d\n", k, wireCount));
                    for (int l = 0; l < wireCount; l++) {
                        if (outNode->output[k]->wire[l] != NULL) {
                            SPAM(("        wire=%d node=%p\n", l, outNode->output[k]->wire[l]->node));
                            if (outNode->output[k]->wire[l]->node == this) { // Found a wire connected to this node
                                SPAM(("        DELETE\n"));
                                delete outNode->output[k]->wire[l];
                                outNode->output[k]->wire[l] = NULL;
                                outNode->output[k]->count--;
                            }
                        }
                    }
                }
                SPAM(("    Deleting input wire\n\n"));
                delete this->input[i]->wire[j];
                this->input[i]->wire[j] = NULL;
                this->input[i]->count--;
            }
        }
    }


    // Disconnect all outputs
    SPAM(("--- Disconnect ouputs\n"));
    for (int i = 0; i < this->outCount; i++) {
        int count = this->output[i]->count;
        SPAM(("    node have %d output\n", count));
        for (int j = 0; j < count; j++) {
            if (this->output[i]->wire[j] != NULL) {
                NativeAudioNode *inNode = this->output[i]->wire[j]->node;
                SPAM(("    found a wire to node %p\n", inNode));
                SPAM(("    input node have %d input\n", outCount));
                for (int k = 0; k < inNode->inCount; k++) {
                    int wireCount = inNode->input[k]->count;
                    SPAM(("        #%d wire = %d\n", k, wireCount));
                    for (int l = 0; l < wireCount; l++) {
                        if (inNode->input[k]->wire[l] != NULL) {
                            SPAM(("        wire=%d node=%p\n", l, inNode->input[k]->wire[l]->node));
                            if (inNode->input[k]->wire[l]->node == this) {
                                SPAM(("       DELETE\n"));
                                delete inNode->input[k]->wire[l];
                                inNode->input[k]->wire[l] = NULL;
                                inNode->input[k]->count--;
                            }
                        }
                    }
                }
                SPAM(("    Deleting input wire\n\n"));
                delete this->output[i]->wire[j];
                this->output[i]->wire[j] = NULL;
                this->output[i]->count--;
            }
        }
    }

    pthread_mutex_lock(&this->audio->recurseLock);

    // Free all frames
    int m = this->outCount > this->inCount ? this->outCount : this->inCount;
    for (int i = 0; i < m; i++) {
        if (this->frames[i] != NULL && this->isFrameOwner(this->frames[i])) {
            free(this->frames[i]);
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

    pthread_mutex_unlock(&this->audio->recurseLock);
}
NativeAudioNodeTarget::NativeAudioNodeTarget(int inCount, int outCount, NativeAudio *audio) 
    : NativeAudioNode(inCount, outCount, audio)
{ 
    if (audio->openOutput() != 0) {
        // TODO : Throw exception
    }
}

bool NativeAudioNodeTarget::process()
{
    return true;
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
        //SPAM(("Gain data %f/%f\n", this->frames[0][i], this->frames[1][i]));
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
      playing(false), stopped(false), loop(false), reader(NULL),
      codecCtx(NULL), tmpPacket(NULL), 
      frameConsumed(true), packetConsumed(true), samplesConsumed(0), audioStream(-1),
      swrCtx(NULL), fCvt(NULL), eof(false), buffering(false)
{
    // TODO : Throw exception instead of return
    this->doSeek = false;
    this->seeking = false;

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

    this->mainCoro = Coro_new();
    Coro_initializeMainCoro(this->mainCoro);

    /* XXX : Not sure if i should free main coro inside close(); 
     *  libcoroutine docs : 
     *       "Note that you can't free the currently 
     *        running coroutine as this would free 
     *        the current C stack."
     */

    this->reader = new NativeAVFileReader(src, &this->audio->bufferNotEmpty, this, this->audio->net);

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

    NativeAudioTrack *thiz = static_cast<NativeAudioTrack *>(arg);

    int ret;
    if ((ret = thiz->initStream()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    if ((ret = thiz->initInternal()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

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

    this->nbChannel = this->outCount;

	// Find the apropriate codec and open it
	this->codecCtx = this->container->streams[this->audioStream]->codec;
	codec = avcodec_find_decoder(this->codecCtx->codec_id);

	if (!avcodec_open2(this->codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Could not find or open the needed codec\n");
		return ERR_NO_CODEC;
	}
    
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
    this->processed = false;

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
        if (this->buffering || this->doSeek) {
            // Reader already trying to get data
            // or we need to seek, so don't buffer
            return false;
        }
        this->buffering = true;
        Coro_startCoro_(this->mainCoro, this->coro, this, NativeAudioTrack::bufferCoro);

        if (!this->reader->pending) {
            this->buffering = false;
            return true;
        }

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
                av_free_packet(this->tmpPacket);
                if (this->readError(ret) < 0) {
                    return false;
                }
            } else {
                this->packetConsumed = false;
            }

            return ret >= 0;
        } else {
            av_free_packet(this->tmpPacket);
        }
    }

    return true;
}

void NativeAudioTrack::bufferCoro(void *arg) {
    NativeAudioTrack *t = static_cast<NativeAudioTrack*>(arg);
    t->bufferInternal();

    Coro_switchTo_(t->coro, t->mainCoro);
}

void NativeAudioTrack::buffer(AVPacket *pkt) {
    this->tmpPacket = pkt;
    this->packetConsumed = false;
}

bool NativeAudioTrack::work() 
{
    if (!this->externallyManaged) {
        if (!this->reader) {
            return false;
        }

        if (this->reader->needWakup) {
            Coro_switchTo_(this->mainCoro, this->coro);
            if (this->reader->pending) {
                return false;
            } else {
                this->buffering = false;
            }
        }
    }

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

    if (this->doNotProcess) {
        return false;
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
    // TODO : Do not alloc a frame each time
    out = (float *)malloc(write * this->nbChannel * NativeAudio::FLOAT32);
    if (!out) {
        printf("malloc failed %d", write * this->nbChannel * NativeAudio::FLOAT32);
        exit(1);
    }

    write = this->resample(out, write);

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
            SPAM(("decode() no packet avail\n"));
            return false;
        } else {
            if (!this->buffer()) {
                SPAM(("decode() buffer call failed\n"));
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
            return true;
        }


        if (this->tmpPacket->pts != AV_NOPTS_VALUE) {
            this->clock = av_q2d(this->container->streams[this->audioStream]->time_base) * this->tmpPacket->pts;
        }

        // tmpFrame is too small to hold the new data
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

                if (this->eof) {
                    return 0;
                }
            }
        }
    }

    return 0;
}

double NativeAudioTrack::getClock() {
    ring_buffer_size_t queuedTrack = PaUtil_GetRingBufferReadAvailable(this->rBufferOut);

    double coef = this->audio->outputParameters->sampleRate * av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);

    double delay = ((double)queuedTrack * NativeAudio::FLOAT32 * this->outCount) / (coef * this->outCount);
    //double audioBuffer = ((double)queuedAudio * NativeAudio::FLOAT32 * this->outCount) / (coef * this->audio->outputParameters->channels);
    //printf("queue=%f audiobuff=%f\n", delay, audioBuffer);
    delay += this->audio->getLatency();

    // TODO : This can be more accurate by calculating how much data is in NativeAudio buffer
    // but i couldn't figure how to do it right (yet)
    return this->clock - delay + 0.28;
}

void NativeAudioTrack::drop(double ms) 
{
    ring_buffer_size_t del = (ms * (this->audio->outputParameters->sampleRate * NativeAudio::FLOAT32) * this->outCount / (NativeAudio::FLOAT32 * this->outCount));
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
    PaUtil_AdvanceRingBufferReadIndex(this->rBufferOut, del > avail ? avail : del);
}

bool NativeAudioNode::isConnected() 
{
    for (int i = 0; i < this->outCount; i++)
    {
        int count = this->output[i]->count;
        for (int j = 0; j < count; j++) 
        {
            if (this->output[i]->wire[j] != NULL) {
                return true;
            }
        }
    }
    return false;
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

    this->doSeekTime = time < 0 ? 0 : time;
    this->doSeek = true;

    pthread_cond_signal(&this->audio->bufferNotEmpty);
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
                this->doNotProcess = false;
            }
        } else {
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
        }
    }

    if (!this->packetConsumed) {
        av_free_packet(this->tmpPacket);
    }
    this->packetConsumed = true;
    this->frameConsumed = true;

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

    // Make sure enought data is available
    if (this->audio->outputParameters->framesPerBuffer >= PaUtil_GetRingBufferReadAvailable(this->rBufferOut)) {
        this->resetFrames();
        //SPAM(("Not enought to read\n"));
        // EOF reached, send message to NativeAudio
        if (this->error == AVERROR_EOF) {
            this->eof = true;
            SPAM(("     => EOF\n"));
            if (this->loop) {
                this->seek(0);
            } else {
                this->doNotProcess = true;
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

void NativeAudioTrack::close(bool reset) 
{
    pthread_mutex_lock(&this->audio->recurseLock);

    if (this->opened) {
        avcodec_close(this->codecCtx);
        swr_free(&this->swrCtx);

        PaUtil_FlushRingBuffer(this->rBufferOut);
        free(this->rBufferOutData);

        this->codecCtx = NULL;
        this->swrCtx = NULL;
        this->rBufferOutData = NULL;
    } else {
        if (!this->externallyManaged && this->reader != NULL) {
            av_free(this->container->pb);
            avformat_free_context(this->container);

            this->container = NULL;
        }
        if (!reset) {
            av_free(this->avioBuffer);
        }
    }

    if (this->reader != NULL) {
        delete this->reader;
        this->reader = NULL;
    }

    if (!this->packetConsumed) {
        av_free_packet(this->tmpPacket);
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
    } else {
        delete this->rBufferOut;
    }

    pthread_mutex_unlock(&this->audio->recurseLock);
}

void NativeAudioTrack::play() 
{
    if (!this->opened) {
        return;
    }

    this->playing = true;
    this->stopped = false;

    pthread_cond_signal(&this->audio->bufferNotEmpty);

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
