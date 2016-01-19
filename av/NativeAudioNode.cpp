#include "NativeAudioNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <pa_ringbuffer.h>

#include <zita-resampler/resampler.h>
#include <Coro.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include <NativeUtils.h>

#define MAX_FAILED_DECODING 50
#define NODE_IO_FOR(i, io) int I = 0;\
while (I < io->count) { \
if (io->wire[i] != NULL) {\
I++;
#define NODE_IO_FOR_END(i) }i++;}

NativeAudioNode::NativeAudioNode(int inCount, int outCount, NativeAudio *audio)
    : m_NullFrames(true), m_Processed(false), m_IsConnected(false), m_InCount(inCount), m_OutCount(outCount),
      m_Audio(audio), m_DoNotProcess(false)
{
    SPAM(("NativeAudioNode init located at %p\n", this));
    int max;

    // Init exports array
    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        m_Args[i] = NULL;
    }

    memset(m_Input, 0, sizeof(m_Input));
    memset(m_Output, 0, sizeof(m_Output));

    // Init node IO queue
    for (int i = 0; i < inCount; i++) {
        m_Input[i] = new NodeLink(INPUT, i, this);
    }

    for (int i = 0; i < outCount; i++) {
        m_Output[i] = new NodeLink(OUTPUT, i, this);
    }

    // Malloc node I/O frames
    max = (inCount > outCount ? inCount : outCount);
    m_Frames = (float **)calloc(max, sizeof(void *));

    for (int i = 0; i < max; i++) {
        m_Frames[i] = NULL;
    }

    // More output than input
    // malloc() the frames
    if (outCount > inCount || inCount == 0) {
        int s = inCount == 0 ? 0 : outCount - inCount;
        for (int i = 0; i < outCount; i++) {
            if (i >= s) {
                m_Frames[i] = this->newFrame();
            }
        }
    }
}

NativeAudioNode::Message::Message(NativeAudioNode *node, ExportsArgs *arg, void *val, unsigned long size)
: m_Node(node), m_Arg(arg), m_Size(size)
{
    m_Val = malloc(size);
    memcpy(m_Val, val, size);
}

NativeAudioNode::Message::~Message() {
    free(m_Val);
}

void NativeAudioNode::callback(NodeMessageCallback cbk, void *custom)
{
    this->callback(cbk, custom, false);
}
void NativeAudioNode::callback(NodeMessageCallback cbk, void *custom, bool block)
{
    m_Audio->m_SharedMsg->postMessage((void *)new CallbackMessage(cbk, this, custom), NATIVE_AUDIO_NODE_CALLBACK);
    if (block) {
        m_Audio->wakeup();
    }
}

bool NativeAudioNode::set(const char *name, ArgType type, void *value, unsigned long size)
{
    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        ExportsArgs *arg = m_Args[i];
        if (arg != NULL && strcmp(name, arg->m_Name) == 0) {
            void *val = value;

            // If posted type and expected type are different
            // try to typecast the value (only few are supported)
            int intVal = 0;
            double doubleVal = 0;
            if (arg->m_Type != type) {
                switch (type) {
                    case DOUBLE: {
                        if (arg->m_Type == INT) {
                            size = sizeof(int);
                            intVal = static_cast<int>(*((double*)value));
                            val = &intVal;
                        } else {
                            return false;
                        }
                    }
                    break;
                    case INT: {
                        if (arg->m_Type == DOUBLE) {
                            size = sizeof(double);
                            doubleVal = static_cast<double>(*((int*)value));
                            val = &doubleVal;
                        } else {
                            return false;
                        }
                    }
                    break;
                    default :
                        return false;
                    break;
                }
            }
            this->post(NATIVE_AUDIO_NODE_SET, arg, val, size);
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
    for (int i = 0; i < m_InCount; i++) {
        for (int j = 0; j < m_Input[i]->count; j++) {
            //SPAM(("  checking input #%d wire %d; node = %p/%p\n", i, j, m_Input[i]->wire[j]->node, nOut));
            if (!m_Input[i]->wire[j]->feedback &&
                 m_Input[i]->wire[j]->node == nOut) {
                //SPAM(("= = = = = = = = = = Its a feedback\n"));
                // It's a feedback
                m_Input[i]->wire[j]->feedback = true;
                m_Input[i]->haveFeedback = true;
                return;
            } else if (!m_Input[i]->wire[j]->feedback) {
                //SPAM(("Go back\n"));
                // Go back a node, and check
                m_Input[i]->wire[j]->node->updateFeedback(nOut);
            }
        }
    }

    return;
}

void NativeAudioNode::updateWiresFrame(int channel, float *frame) {
    this->updateWiresFrame(channel, frame, NULL);
}
void NativeAudioNode::updateWiresFrame(int channel, float *frame, float *discardFrame) {
    // Stop updating wire when a framed owned by the node is met
    // Note : If the frame owned is the one that need to be updated
    // (dicardFrame) do not stop and update it
    if (m_Frames[channel] != NULL &&
            m_Frames[channel] != discardFrame &&
            this->isFrameOwner(m_Frames[channel])) {
        return;
    }

    m_Frames[channel] = frame;

    if (!m_Output[channel]) {
        return;
    }

    int count = m_Output[channel]->count;
    for (int i = 0; i < count; i++)
    {
        m_Output[channel]->wire[i]->node->updateWiresFrame(channel, frame, discardFrame);
    }

    return;
}

bool NativeAudioNode::queue(NodeLink *in, NodeLink *out)
{
    SPAM(("connect in node %p; out node %p\n", in->node, out->node));
    NodeIO **inLink;
    NodeIO **outLink;

    // First, make sure we have enought space to connect wire
    inLink = this->getWire(m_Input[out->channel]);
    outLink = this->getWire(in->node->m_Output[in->channel]);

    if (inLink == NULL || outLink == NULL) {
        return false;
    }

    m_Audio->lockQueue();
    // Connect blocks frames
    if (in->node->m_Frames[in->channel] == NULL) {
        SPAM(("Malloc frame\n"));
        in->node->m_Frames[in->channel] = in->node->newFrame();
        //m_Frames[out->channel] = in->node->m_Frames[in->channel];
    }

    if (out->count == 0 && in->count == 0 && in->node != out->node) {
        SPAM(("frame previously assigned\n"));
        // Frame was previously assigned, update next outputs
        if (m_Frames[out->channel] != NULL) {
            float *discard = m_Frames[out->channel];
            m_Frames[out->channel] = NULL;

            SPAM(("Update wires\n"));
            this->updateWiresFrame(out->channel, in->node->m_Frames[in->channel], discard);

            SPAM(("Freeing frame @ %p\n", discard));
            free(discard);
        }
        m_Frames[out->channel] = in->node->m_Frames[in->channel];
    } else if (out->count == 1 || in->count == 1 || in->node == out->node) {
        // Multiple input or output on same channel
        // need to use an internal buffer

        m_Frames[out->channel] = this->newFrame();

        SPAM(("Update wires\n"));
        this->updateWiresFrame(out->channel, m_Frames[out->channel]);

        SPAM(("Using custom frames\n"));
    }

    // Then connect wires
    *inLink = new NodeIO(in->node, in->node->m_Frames[in->channel]);

    // And update input node wires
    *outLink = new NodeIO(out->node, out->node->m_Frames[out->channel]);

    in->count++;
    out->count++;

    // Check if wire created a feedback somewhere
    this->updateFeedback(out->node);

    // Go trought all the node connected to this one,
    // and update "isConnected" for each one.
    in->node->updateIsConnected();

    m_Audio->unlockQueue();

    return true;
}

bool NativeAudioNode::unqueue(NodeLink *input, NodeLink *output)
{
    m_Audio->lockQueue();
    NodeLink *wiresIn, *wiresOut;
    int count;

    wiresIn = m_Input[output->channel];
    wiresOut = input->node->m_Output[input->channel];

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
        if (m_Frames[output->channel] != NULL && this->isFrameOwner(m_Frames[output->channel])) {
            free(m_Frames[output->channel]);
        }
        m_Frames[output->channel] = NULL;
        // Forward update the queue
        this->updateWiresFrame(output->channel, m_Frames[output->channel]);
        wiresIn->count = 0;
    }

    m_Audio->unlockQueue();
    return true;
}

void NativeAudioNode::processQueue()
{
    SPAM(("process queue on %p\n", this));

    if (!m_IsConnected) {
        SPAM(("    Node is not connected %p\n", this));
        return;
    }

    // Let's go for a new round.
    // First mark all output as unprocessed
    for (int i = 0; i < m_OutCount ; i++) {
        int j = 0;
        NODE_IO_FOR(j, m_Output[i])
            SPAM(("     Marking output at %p as unprocessed (%p)\n", m_Output[i]->wire[j]->node, this));
            m_Output[i]->wire[j]->node->m_Processed = false;
        NODE_IO_FOR_END(j)
    }

    // Do we have all data we need to process this node?
    for (int i = 0; i < m_InCount; i++) {
        int j = 0;
        NODE_IO_FOR(j, m_Input[i])
            if (!m_Input[i]->wire[j]->node->m_Processed && m_Input[i]->wire[j]->node->m_IsConnected) {
                SPAM(("     Input %p havn't been processed, return\n", m_Input[i]->wire[j]->node));
                // Needed data havn't been processed yet. Return.
                return;
            } else {
                if (!m_Input[i]->wire[j]->node->m_IsConnected) {
                    SPAM(("     Input %p isn't connected. No need to process\n", m_Input[i]->wire[j]->node));
                } else {
                    SPAM(("    Input at %p is already processed\n", m_Input[i]->wire[j]->node));
                }
            }
        NODE_IO_FOR_END(j)
    }

    // Some sanity check and merge input if needed
    for (int i = 0; i < m_InCount; i++) {
        // Something is wrong (ie : node is not connected)
        if (m_Frames[i] == NULL) {
            SPAM(("     => Found a NULL frame. Fixing it\n"));
            m_Frames[i] = this->newFrame();
            this->updateWiresFrame(i, m_Frames[i]);
        }

        // Have multiple data on one input
        // add all input
        if (m_Input[i]->count > 1) {
            // Reset buffer
            if (!m_Input[i]->haveFeedback) {
                memset(m_Frames[i], 0, m_Audio->m_OutputParameters->m_BufferSize/m_Audio->m_OutputParameters->m_Channels);
            }

            // Merge all input
            int j = 0;
            NODE_IO_FOR(j, m_Input[i])
                if (m_Frames[i] != m_Input[i]->wire[j]->frame) {
                    SPAM(("     Merging input #%d from %p to %p\n", m_Input[i]->channel, m_Input[i]->wire[j]->node, this));
                    SPAM(("     frames=%p from %p\n", m_Frames[i], m_Input[i]->wire[j]->frame));
                    for (int k = 0; k < m_Audio->m_OutputParameters->m_FramesPerBuffer; k++) {
                        m_Frames[i][k] += m_Input[i]->wire[j]->frame[k];
                    }
                }
                NODE_IO_FOR_END(j)
        }
    }

    if (!m_DoNotProcess) {
        if (!this->process()) {
            SPAM(("Failed to process node at %p\n", this));
            m_Processed = true;
            return; // XXX : This need to be double checked
        }
    } else {
        SPAM(("Node marked as doNotProcess\n"));
    }

    for (int i = 0; i < m_OutCount; i++) {
        // Have multiple data on one output.
        // Copy output data to next bloc
        if (m_Output[i]->count > 1) {
            int j = 0;
            NODE_IO_FOR(j, m_Output[i])
                if (m_Output[i]->wire[j]->frame != m_Frames[i]) {
                    for (int k = 0; k < m_Audio->m_OutputParameters->m_FramesPerBuffer; k++) {
                        m_Output[i]->wire[j]->frame[k] = m_Frames[i][k];
                    }
                }
            NODE_IO_FOR_END(j)
        }
    }

    SPAM(("Marking node %p as processed\n", this));
    m_Processed = true;

    // Go process next outputs
    int count = 0;
    for (int i = 0; i < m_OutCount; i++) {
        int j = 0;
        NODE_IO_FOR(j, m_Output[i])
            if (!m_Output[i]->wire[j]->node->m_Processed) {
                m_Output[i]->wire[j]->node->processQueue();
            }
            if (m_Output[i]->wire[j]->node->m_Processed) {
                count++;
            }
        NODE_IO_FOR_END(j)
    }

    SPAM(("----- processQueue on node %p finished\n", this));
}

float *NativeAudioNode::newFrame()
{
#define FRAME_SIZE m_Audio->m_OutputParameters->m_BufferSize/m_Audio->m_OutputParameters->m_Channels
    float *ret = (float *)calloc(FRAME_SIZE + sizeof(void *), NativeAudio::FLOAT32);
    if (ret != NULL) {
        // Store at the end of the frame array
        // a pointer to the frame owner
        ptrdiff_t addr = reinterpret_cast<ptrdiff_t>(this);
        void *p = (void *)addr;
        float *tmp = &ret[FRAME_SIZE];
        memcpy(tmp, &p, sizeof(void *));
    }
    return ret;
#undef FRAME_SIZE
}

void NativeAudioNode::post(int msg, ExportsArgs *arg, void *val, unsigned long size) {
    m_Audio->m_SharedMsg->postMessage((void *)new Message(this, arg, val, size), msg);
}

NativeAudioNode::~NativeAudioNode() {
    // NOTE : The caller is responsible for calling NativeAudio::lockQueue();
    // before deleting a node

    // Disconnect algorithm :
    //  - Start from the node and free internal frames (frame id > input count)
    //  - Go trought all input wire to find connected node
    //      - Itterate over nodes output and delete the wire
    //  - Delete destructing node input wire
    //
    //  - Go trought all output
    //      - Free frame owned by the node, set it to NULL then forward update
    //      the frames for output nodes (the frames will be set to NULL, if
    //      needed the FX queue will alloc a new frame on next run)
    //  - Delete the wires, using the same technique than inputs

    SPAM(("--- Disconnect inputs\n"));
    for (int i = 0; i < m_InCount; i++) {
        int count = m_Input[i]->count;
        SPAM(("    node have %d input\n", count));
        // Free internal frames
        if (i > m_OutCount) {
            if (m_Frames[i] != NULL && this->isFrameOwner(m_Frames[i])) {
                free(m_Frames[i]);
                m_Frames[i] = NULL;
            }
        }
        // Find all connected inputs
        for (int j = 0; j < count; j++) {
            if (m_Input[i]->wire[j] != NULL) { // Got a wire to a node
                NativeAudioNode *outNode = m_Input[i]->wire[j]->node;
                SPAM(("    found a wire to node %p\n", outNode));
                SPAM(("    output node have %d output\n", m_OutCount));
                for (int k = 0; k < outNode->m_OutCount; k++) { // Go trought each output and wire
                    int wireCount = outNode->m_Output[k]->count;
                    SPAM(("        #%d wire = %d\n", k, wireCount));
                    for (int l = 0; l < wireCount; l++) {
                        if (outNode->m_Output[k]->wire[l] != NULL) {
                            SPAM(("        wire=%d node=%p\n", l, outNode->m_Output[k]->wire[l]->node));
                            // Found a wire connected to this node, delete it
                            if (outNode->m_Output[k]->wire[l]->node == this) {
                                SPAM(("        DELETE\n"));
                                delete outNode->m_Output[k]->wire[l];
                                outNode->m_Output[k]->wire[l] = NULL;
                                outNode->m_Output[k]->count--;
                            }
                        }
                    }
                }
                SPAM(("    Deleting input wire\n\n"));
                // Deleting input wire
                delete m_Input[i]->wire[j];
                m_Input[i]->wire[j] = NULL;
                m_Input[i]->count--;
            }
        }
    }


    SPAM(("--- Disconnect ouputs\n"));
    for (int i = 0; i < m_OutCount; i++) {
        int count = m_Output[i]->count;
        SPAM(("    node have %d output on channel %d/%d\n", count, m_Output[i]->channel, i));
        // Free remaining frames owned by the node
        // And forward update it
        if (m_Frames[i] != NULL && this->isFrameOwner(m_Frames[i])) {
            float *frame = m_Frames[i];
            m_Frames[i] = NULL;
            this->updateWiresFrame(i, m_Frames[i]);
            free(frame);
        }
        for (int j = 0; j < count; j++) {
            if (m_Output[i]->wire[j] != NULL) {
                NativeAudioNode *inNode = m_Output[i]->wire[j]->node;
                SPAM(("    found a wire to node %p\n", inNode));
                SPAM(("    input node have %d input\n", m_OutCount));
                for (int k = 0; k < inNode->m_InCount; k++) {
                    int wireCount = inNode->m_Input[k]->count;
                    SPAM(("        #%d wire = %d\n", k, wireCount));
                    for (int l = 0; l < wireCount; l++) {
                        if (inNode->m_Input[k]->wire[l] != NULL) {
                            SPAM(("        wire=%d node=%p\n", l, inNode->m_Input[k]->wire[l]->node));
                            if (inNode->m_Input[k]->wire[l]->node == this) {
                                SPAM(("       DELETE\n"));
                                delete inNode->m_Input[k]->wire[l];
                                inNode->m_Input[k]->wire[l] = NULL;
                                inNode->m_Input[k]->count--;
                            }
                        }
                    }
                }
                SPAM(("    Deleting input wire\n\n"));
                delete m_Output[i]->wire[j];
                m_Output[i]->wire[j] = NULL;
                m_Output[i]->count--;
            }
        }
    }

    free(m_Frames);

    for (int i = 0; i < m_InCount; i++) {
        delete m_Input[i];
    }

    for (int i = 0; i < m_OutCount; i++) {
        delete m_Output[i];
    }

    for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
        if (m_Args[i] != NULL) {
            delete m_Args[i];
        }
    }

    if (this == m_Audio->m_Output) {
        m_Audio->m_Output = NULL;
    }
}

NativeAudioNodeTarget::NativeAudioNodeTarget(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNode(inCount, outCount, audio)
{
    if (audio->openOutput() != 0) {
        throw new NativeAudioNodeException("Failed to open audio output");
    }
}

bool NativeAudioNodeTarget::process()
{
    return true;
}

NativeAudioNodeReverb::NativeAudioNodeReverb(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNode(inCount, outCount, audio), m_Delay(500)
{
    m_Args[0] = new ExportsArgs("delay", DOUBLE, &m_Delay);
}

bool NativeAudioNodeReverb::process()
{
#if 0
    int delayMilliseconds = 3; // half a second
    int delaySamples = (int)((float)delayMilliseconds * 44.1f); // assumes 44100 Hz sample rate
    float decay = 0.5f;
    int length = m_Audio->m_OutputParameters->m_FramesPerBuffer;

    for (int j = 0; j < m_InCount; j++) {
        for (int i = 0; i < length - delaySamples; i++)
        {
            // WARNING: overflow potential
            m_Frames[j][i + delaySamples] += (short)((float)m_Frames[j][i] * decay);
        }
    }
#endif
    return true;
}

NativeAudioNodeStereoEnhancer::NativeAudioNodeStereoEnhancer(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNode(inCount, outCount, audio), m_Width(0)
{
    m_Args[0] = new ExportsArgs("width", DOUBLE, &m_Width);
    if (inCount != 2 || outCount != 2) {
        throw new NativeAudioNodeException("Stereo enhancer must have 2 input and 2 output");
    }
}

bool NativeAudioNodeStereoEnhancer::process()
{
    double scale = m_Width * 0.5;

    for (int i = 0; i < m_Audio->m_OutputParameters->m_FramesPerBuffer; i++) {
        float l = m_Frames[0][i];
        float r = m_Frames[1][i];

        double m = (l + r) * 0.5;
        double s = (r - l) * scale;

        m_Frames[0][i] = m-s;
        m_Frames[1][i] = m+s;
    }

    return true;
}

NativeAudioNodeCustom::NativeAudioNodeCustom(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNode(inCount, outCount, audio), m_Cbk(NULL), m_Custom(NULL)
{
}

void NativeAudioNodeCustom::setCallback(NodeCallback cbk, void *custom)
{
    m_Cbk = cbk;
    m_Custom = custom;
}

bool NativeAudioNodeCustom::process()
{
    if (m_Cbk != NULL) {
        NodeEvent ev;

        ev.data = m_Frames;
        ev.size = m_Audio->m_OutputParameters->m_FramesPerBuffer;
        ev.custom = m_Custom;

        m_Cbk(&ev);
    }

    return true;
}

NativeAudioSource::NativeAudioSource(int out, NativeAudio *audio, bool external) :
    NativeAudioNode(0, out, audio), m_OutputParameters(NULL),
    m_BufferNotEmpty(NULL), m_rBufferOut(NULL), m_Reader(NULL),
    m_ExternallyManaged(external), m_Playing(false), m_PlayWhenReady(false), 
    m_Stopped(false), m_Loop(false), m_NbChannel(0), m_CodecCtx(NULL), 
    m_TmpPacket(NULL), m_Clock(0), m_FrameConsumed(true), m_PacketConsumed(true),
    m_SamplesConsumed(0), m_AudioStream(-1), m_FailedDecoding(0),
    m_SwrCtx(NULL), m_sCvt(NULL), m_fCvt(NULL), m_AvioBuffer(NULL),
    m_fBufferInData(NULL), m_fBufferOutData(NULL),
    m_rBufferOutData(NULL), m_Buffering(false)
{
    m_DoSemek = false;
    m_Seeking = false;

    m_TmpFrame.size = 0;
    m_TmpFrame.data = NULL;
    m_TmpFrame.nbSamples = 0;
}

int NativeAudioSource::open(const char *src)
{
#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
this->closeInternal(true); \
return err;
    // If a previous file has been opened, close it
    if (m_Container != NULL) {
        this->closeInternal(true);
    }

    m_Coro = Coro_new();
    m_MainCoro = Coro_new();
    Coro_initializeMainCoro(m_MainCoro);

    m_Reader = new NativeAVStreamReader(src, NativeAudio::sourceNeedWork, m_Audio, this, m_Audio->m_Net);

    m_AvioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!m_AvioBuffer) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container = avformat_alloc_context();
    if (!m_Container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container->pb = avio_alloc_context(m_AvioBuffer, NATIVE_AVIO_BUFFER_SIZE,
         0, m_Reader, NativeAVStreamReader::read, NULL, NativeAVStreamReader::seek);
    if (!m_Container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    return 0;
#undef RETURN_WITH_ERROR
}

int NativeAudioSource::openInit()
{
    m_SourceDoOpen = true;
    NATIVE_PTHREAD_SIGNAL(&m_Audio->m_QueueNeedData);
    return 0;
}

void NativeAudioSource::openInitCoro(void *arg)
{
#define RETURN_WITH_ERROR(err) \
thiz->sendEvent(SOURCE_EVENT_ERROR, err, false);\
thiz->postMessage(thiz, NativeAVSource::MSG_CLOSE);\
Coro_switchTo_(thiz->m_Coro, thiz->m_MainCoro);
    NativeAudioSource *thiz = static_cast<NativeAudioSource *>(arg);
    thiz->m_SourceDoOpen = false;

    int ret;
    if ((ret = thiz->initStream()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    if ((ret = thiz->initInternal()) != 0) {
        RETURN_WITH_ERROR(ret);
    }

    Coro_switchTo_(thiz->m_Coro, thiz->m_MainCoro);
#undef RETURN_WITH_ERROR
}

int NativeAudioSource::open(void *buffer, int size)
{
#define RETURN_WITH_ERROR(err) \
this->sendEvent(SOURCE_EVENT_ERROR, err, false);\
this->closeInternal(true); \
return err;

    // If a previous file has been opened, close it
    if (m_Container != NULL) {
        this->closeInternal(true);
    }

    m_Reader = new NativeAVBufferReader((uint8_t *)buffer, size);

    m_AvioBuffer = (unsigned char *)av_malloc(NATIVE_AVIO_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!m_AvioBuffer) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container = avformat_alloc_context();
    if (!m_Container) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

    m_Container->pb = avio_alloc_context(m_AvioBuffer, NATIVE_AVIO_BUFFER_SIZE,
        0, m_Reader, NativeAVBufferReader::read, NULL, NativeAVBufferReader::seek);
    if (!m_Container->pb) {
        RETURN_WITH_ERROR(ERR_OOM);
    }

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

int NativeAudioSource::initStream()
{
    // Open input
    int ret = avformat_open_input(&m_Container, "dummyFile", NULL, NULL);

    if (ret != 0) {
        char error[1024];
        av_strerror(ret, error, 1024);
        fprintf(stderr, "Couldn't open file : %s\n", error);
        return ERR_INTERNAL;
    }

    NativePthreadAutoLock lock(&NativeAVSource::m_FfmpegLock);
    // Retrieve stream information
    if (avformat_find_stream_info(m_Container, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information\n");
        return ERR_NO_INFORMATION;
    }

    // Dump information about file onto standard error
    av_dump_format(m_Container, 0, "Memory input", 0);

    // Find first audio stream
    for (unsigned int i = 0; i < m_Container->nb_streams; i++) {
        if (m_Container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_AudioStream = i;
            break;
        }
    }

    if (m_AudioStream == -1) {
        return ERR_NO_AUDIO;
    }

    return 0;
}

int NativeAudioSource::initInternal()
{
    AVCodec *codec;

    m_NbChannel = m_OutCount;

    m_TmpPacket = new AVPacket();
    av_init_packet(m_TmpPacket);

    // Find the apropriate codec and open it
    m_CodecCtx = m_Container->streams[m_AudioStream]->codec;
    codec = avcodec_find_decoder(m_CodecCtx->codec_id);
    if (!codec) {
        return ERR_NO_CODEC;
    }

    NativePthreadAutoLock lock(&NativeAVSource::m_FfmpegLock);
    if (avcodec_open2(m_CodecCtx, codec, NULL) < 0) {
        fprintf(stderr, "Could not find or open the needed codec\n");
        return ERR_NO_CODEC;
    }

    // Frequency resampling
    if (m_CodecCtx->sample_rate != m_Audio->m_OutputParameters->m_SampleRate) {
        m_fCvt = new Resampler();
        m_fCvt->setup(m_CodecCtx->sample_rate, m_Audio->m_OutputParameters->m_SampleRate, m_OutCount, 32);

        if (!(m_fBufferOutData = (float *)malloc(NATIVE_RESAMPLER_BUFFER_SAMPLES * m_OutCount * NativeAudio::FLOAT32))) {
            fprintf(stderr, "Failed to init frequency resampler buffers");
            return ERR_OOM;
        }

        m_fCvt->inp_count = m_fCvt->inpsize() / 2 -1;
        m_fCvt->inp_data = 0;

        m_fCvt->out_count = NATIVE_RESAMPLER_BUFFER_SAMPLES;
        m_fCvt->out_data = m_fBufferOutData;
    }

    // Init output buffer
    int bufferSize = m_Audio->m_OutputParameters->m_BufferSize;
    m_rBufferOut = new PaUtilRingBuffer();
    if (!(m_rBufferOutData = calloc(bufferSize * m_OutCount, NativeAudio::FLOAT32))) {
        return ERR_OOM;
    }

    if (0 > PaUtil_InitializeRingBuffer(static_cast<PaUtilRingBuffer*>(m_rBufferOut),
            (NativeAudio::FLOAT32 * m_OutCount),
            bufferSize,
            m_rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        return ERR_OOM;
    }

    if (m_CodecCtx->sample_fmt != AV_SAMPLE_FMT_FLT ||
        m_CodecCtx->channel_layout != AV_CH_LAYOUT_STEREO) {
        int64_t channelLayout;

        // Channel layout is not set, use default one
        if (m_CodecCtx->channel_layout == 0) {
            channelLayout = av_get_default_channel_layout(m_CodecCtx->channels);
        } else {
            channelLayout = m_CodecCtx->channel_layout;
        }

        m_SwrCtx = swr_alloc_set_opts(NULL,
                AV_CH_LAYOUT_STEREO_DOWNMIX, AV_SAMPLE_FMT_FLT, m_CodecCtx->sample_rate,
                channelLayout, m_CodecCtx->sample_fmt, m_CodecCtx->sample_rate,
                0, NULL);
        if (!m_SwrCtx || swr_init(m_SwrCtx) < 0) {
            fprintf(stderr, "Failed to init sample resampling converter\n");
            return ERR_NO_RESAMPLING_CONVERTER;
        }
    }

    m_Opened = true;
    m_Processed = false;

    if (m_PlayWhenReady) {
        this->play();
    }

    this->sendEvent(SOURCE_EVENT_READY, 0, false);

    return 0;
}

int NativeAudioSource::avail()
{
    return m_Opened ? (int) PaUtil_GetRingBufferReadAvailable(m_rBufferOut) : 0;
}

bool NativeAudioSource::buffer()
{
    if (m_Reader->m_Async) {
        if (m_Buffering || m_DoSemek) {
            // Reader already trying to get data
            // or we need to seek, so don't buffer
            return false;
        }

        m_Buffering = true;
        Coro_startCoro_(m_MainCoro, m_Coro, this, NativeAudioSource::bufferCoro);

        if (!m_Reader->m_Pending) {
            m_Buffering = false;
            return true;
        }

        return false;
    } else {
        return this->bufferInternal();
    }
}

bool NativeAudioSource::bufferInternal()
{
    for (; ;) {
        int ret = av_read_frame(m_Container, m_TmpPacket);
        if (m_TmpPacket->stream_index == m_AudioStream) {
            if (ret < 0) {
                av_free_packet(m_TmpPacket);
                if (this->readError(ret) < 0) {
                    return false;
                }
            } else {
                m_PacketConsumed = false;
            }

            return ret >= 0;
        } else {
            av_free_packet(m_TmpPacket);
        }
    }

    return true;
}

void NativeAudioSource::bufferCoro(void *arg) {
    NativeAudioSource *t = static_cast<NativeAudioSource*>(arg);
    t->bufferInternal();

    Coro_switchTo_(t->m_Coro, t->m_MainCoro);
}

// This method is used when the source is externally managed
// (To fill a packet to the source)
void NativeAudioSource::buffer(AVPacket *pkt) {
    m_TmpPacket = pkt;
    m_PacketConsumed = false;
}

bool NativeAudioSource::work()
{
    if (!m_ExternallyManaged) {
        if (!m_Reader) {
            return false;
        }

        if (m_SourceDoOpen) {
            Coro_startCoro_(m_MainCoro, m_Coro, this, NativeAudioSource::openInitCoro);
            return true;
        }

        if (m_Reader->m_NeedWakup) {
            Coro_switchTo_(m_MainCoro, m_Coro);
            if (m_Reader->m_Pending) {
                return false;
            } else {
                m_Buffering = false;
            }
        }
    }

    if (m_DoSemek) {
        if (m_Reader->m_Pending) {
            return false;
        }

        if (!m_Reader->m_Async) {
            this->seekInternal(m_DoSeekTime);
        } else {
            Coro_startCoro_(m_MainCoro, m_Coro, this, NativeAudioSource::seekCoro);
        }
    }

    if (m_DoNotProcess || !m_Opened || m_Stopped) {
        SPAM(("Source will not be decoded. doNotProcess = %d opened = %d stopped = %d",
            m_DoNotProcess, m_Opened, m_Stopped));
        return false;
    }

    ring_buffer_size_t avail = PaUtil_GetRingBufferWriteAvailable(m_rBufferOut);

    if (avail < m_Audio->m_OutputParameters->m_FramesPerBuffer) {
        SPAM(("Work failed because not enought space is available to write decoded packet %lu\n", avail));
        return false;
    }

    if (!this->decode()) {
        SPAM(("Work failed because source is stoped or decoding failed %d\n", m_Stopped));
        return false;
    }

    // Decode might return true but no frame have been decoded
    // So we return true here, to get a new frame right away
    if (m_FrameConsumed) {
        return true;
    }

    avail = PaUtil_GetRingBufferWriteAvailable(m_rBufferOut);
    int write = avail > m_Audio->m_OutputParameters->m_FramesPerBuffer ? m_Audio->m_OutputParameters->m_FramesPerBuffer : avail;

    this->resample(write);

    return true;
}
bool NativeAudioSource::decode()
{
#define RETURN_WITH_ERROR(err) \
av_free(tmpFrame); \
this->sendEvent(SOURCE_EVENT_ERROR, err, true);\
return false;
    if (m_Error) {
        SPAM(("decode() return false cause of error %d\n", m_Error));
        return false;
    }
    // No last packet, get a new one
    if (m_PacketConsumed) {
        if (!this->buffer()) {
            SPAM(("decode() buffer call failed\n"));
            return false;
        }
    }

    // No last frame, get a new one
    if (m_FrameConsumed) {
        int gotFrame, len;
        AVFrame *tmpFrame;

        if (!(tmpFrame = av_frame_alloc())) {
            SPAM(("Failed to alloc frame\n"));
            RETURN_WITH_ERROR(ERR_OOM);
        }

        // Decode packet
        len = avcodec_decode_audio4(m_CodecCtx, tmpFrame, &gotFrame, m_TmpPacket);

        if (len < 0) {
            if (m_FailedDecoding > MAX_FAILED_DECODING) {
                this->stop();
                RETURN_WITH_ERROR(ERR_DECODING);
            } else {
                // Got an error, skip the frame
                m_TmpPacket->size = 0;
            }

            m_FailedDecoding++;

            return true;
        } else if (len < m_TmpPacket->size) {
            m_TmpPacket->data += len;
            m_TmpPacket->size -= len;
        } else {
            m_PacketConsumed = true;
            av_free_packet(m_TmpPacket);
        }

        m_FailedDecoding = 0;

        // Didn't got a frame let's try next time
        if (gotFrame == 0) {
            return true;
        }

        if (m_TmpPacket->pts != AV_NOPTS_VALUE) {
            m_Clock = av_q2d(m_Container->streams[m_AudioStream]->time_base) * m_TmpPacket->pts;
        }

        // this->tmpFrame is too small to hold the new data
        if (m_TmpFrame.nbSamples < tmpFrame->nb_samples) {
            if (m_TmpFrame.size != 0) {
                free(m_TmpFrame.data);
            }
            m_TmpFrame.size = tmpFrame->linesize[0];
            m_TmpFrame.data = (float *)malloc(tmpFrame->nb_samples * NativeAudio::FLOAT32 * 2);
            // XXX : Right now, source output is always stereo
            if (m_TmpFrame.data == NULL) {
                RETURN_WITH_ERROR(ERR_OOM);
            }
        }

        if (m_SwrCtx) {
            uint8_t *out[1];
            out[0] = (uint8_t *)m_TmpFrame.data;

            const uint8_t **in = (const uint8_t **)tmpFrame->data;

            int len = swr_convert(m_SwrCtx, out, tmpFrame->nb_samples, in, tmpFrame->nb_samples);
            m_TmpFrame.nbSamples = len;
        } else {
            memcpy(m_TmpFrame.data, tmpFrame->data[0], tmpFrame->linesize[0]);
            m_TmpFrame.nbSamples = tmpFrame->nb_samples;
        }

        // Reset frequency converter input
        if (m_fCvt) {
            m_fCvt->inp_count = m_TmpFrame.nbSamples;
            m_fCvt->inp_data = m_TmpFrame.data;
        }

        av_free(tmpFrame);

        m_SamplesConsumed = 0;
        m_FrameConsumed = false;
    }

    return true;
#undef RETURN_WITH_ERROR
}
int NativeAudioSource::resample(int destSamples) {
    int channels = m_NbChannel;

    if (m_fCvt) {
        int copied = 0;
        int passCopied = 0;
        for (; ;) {
            int sampleSize;

            sampleSize = channels * NativeAudio::FLOAT32;

            // Output is empty
            if (m_fCvt->out_count == NATIVE_RESAMPLER_BUFFER_SAMPLES) {
                m_fCvt->out_data = m_fBufferOutData;
                m_SamplesConsumed = 0;
                passCopied= 0;

                // Resample as much data as possible
                while (m_fCvt->out_count > 0 && m_fCvt->inp_count > 0) {
                    if (0 != m_fCvt->process()) {
                        fprintf(stderr, "Failed to resample audio data\n");
                        return -1;
                    }
                }
            }

            if (m_fCvt->out_count < NATIVE_RESAMPLER_BUFFER_SAMPLES) {
                int write, avail;

                avail = NATIVE_RESAMPLER_BUFFER_SAMPLES - m_fCvt->out_count;
                write = destSamples > avail ? avail : destSamples;
                write -= passCopied;

                PaUtil_WriteRingBuffer(m_rBufferOut, m_fBufferOutData + m_SamplesConsumed * channels, write);

                m_SamplesConsumed += write;
                m_fCvt->out_count += write;
                copied += write;
                passCopied += write;

                if (copied == destSamples) {
                    return copied;
                }
            }

            if (m_fCvt->inp_count == 0) {
                m_FrameConsumed = true;
                return copied;
            }
        }
    } else {
        int sampleSize, copied;

        sampleSize = m_NbChannel * NativeAudio::FLOAT32;
        copied = 0;

        for (; ;) {
            int write, avail;

            avail = m_TmpFrame.nbSamples - m_SamplesConsumed;
            write = destSamples > avail ? avail : destSamples;
            write -= copied;

            PaUtil_WriteRingBuffer(m_rBufferOut, m_TmpFrame.data + m_SamplesConsumed * channels, write);

            copied += write;
            m_SamplesConsumed += write;

            if (m_SamplesConsumed == m_TmpFrame.nbSamples) {
                m_FrameConsumed = true;
                return copied;
            }

            if (copied == destSamples) {
                return copied;
            }
        }
    }

    return 0;
}

double NativeAudioSource::getClock() {
    if (!m_Opened) return 0;

    ring_buffer_size_t queuedSource = PaUtil_GetRingBufferReadAvailable(m_rBufferOut);

    double delay = ((double)(queuedSource - m_TmpFrame.nbSamples)* ((double)1/m_Audio->m_OutputParameters->m_SampleRate));

    // Remove the duration of the last consumed frame
    // (since the clock is for the start of the sample)
    delay -= m_TmpFrame.nbSamples / m_Audio->m_OutputParameters->m_SampleRate;

    return m_Clock - delay;
}

double NativeAudioSource::drop(double sec)
{
    ring_buffer_size_t drop = ceil(sec /(double)(1/m_Audio->m_OutputParameters->m_SampleRate));
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(m_rBufferOut);
    ring_buffer_size_t actualDrop = drop > avail ? drop : avail;

    PaUtil_AdvanceRingBufferReadIndex(m_rBufferOut, actualDrop);

    return actualDrop * 1/m_Audio->m_OutputParameters->m_SampleRate;
}

void NativeAudioCustomSource::play()
{
    m_Playing = true;

    NATIVE_PTHREAD_SIGNAL(&m_Audio->m_QueueHaveData);
}


void NativeAudioCustomSource::pause()
{
    m_Playing = false;
}

void NativeAudioCustomSource::stop()
{
    m_Playing = false;
    this->seek(0);
}

void NativeAudioCustomSource::setSeek(SeekCallback cbk, void *custom)
{
    m_SeekCallback = cbk;
    m_Custom = custom;
}

void NativeAudioCustomSource::seek(double ms)
{
    m_SeekTime = ms;
    if (m_SeekCallback != NULL) {
        this->callback(NativeAudioCustomSource::seekMethod, NULL);
    }
}

// Called from Audio thread
void NativeAudioCustomSource::seekMethod(NativeAudioNode *node, void *custom)
{
    NativeAudioCustomSource *thiz = static_cast<NativeAudioCustomSource*>(node);
    thiz->m_SeekCallback(static_cast<NativeAudioCustomSource*>(node), thiz->m_SeekTime, thiz->m_Custom);
}

bool NativeAudioCustomSource::process()
{
    if (!m_Playing) return false;

    NativeAudioNodeCustom::process();

    NATIVE_PTHREAD_SIGNAL(&m_Audio->m_QueueHaveData);

    return true;
}

bool NativeAudioCustomSource::isActive()
{
    return m_Playing;
}

bool NativeAudioNode::updateIsConnectedInput()
{
    SPAM(("    updateIsConnectedInput @ %p\n", this));
    if (m_InCount == 0) return true;

    for (int i = 0; i < m_InCount; i++)
    {
        SPAM(("    input %d\n", i));
        int count = m_Input[i]->count;
        for (int j = 0; j < count; j++)
        {
            if (m_Input[i]->wire[j] != NULL) {
                SPAM(("    Wire %d to %p\n", i, m_Input[i]->wire[j]->node));
                return m_Input[i]->wire[j]->node->updateIsConnected(false, true);
            }
        }
    }

    return false;
}

bool NativeAudioNode::updateIsConnectedOutput()
{
    SPAM(("    updateIsConnectedOutput @ %p\n", this));
    if (m_OutCount == 0) return true;

    for (int i = 0; i < m_OutCount; i++)
    {
        int count = m_Output[i]->count;
        SPAM(("    output %d count=%d\n", i, count));
        for (int j = 0; j < count; j++)
        {
            if (m_Output[i]->wire[j] != NULL) {
                SPAM(("    Wire %d to %p\n", i, m_Output[i]->wire[j]->node));
                return m_Output[i]->wire[j]->node->updateIsConnected(true, false);
            }
        }
    }

    return false;
}

// This method will check that the node
// is connected to a source and a target
bool NativeAudioNode::updateIsConnected() {
    return this->updateIsConnected(false, false);
}

bool NativeAudioNode::updateIsConnected(bool input, bool output)
{
    SPAM(("updateIsConnected @ %p input=%d output=%d\n", this, m_Input, m_Output));
    m_IsConnected =
        (input || this->updateIsConnectedInput()) &&
        (output || this->updateIsConnectedOutput());
    SPAM(("updateIsConnected finished @ %p / isConnected=%d\n", this, m_IsConnected));
    return m_IsConnected;
}

void NativeAudioNode::resetFrames() {
    if (!m_NullFrames) {
        for (int i = 0; i < m_OutCount; i++) {
            this->resetFrame(i);
        }
        m_NullFrames = true;
    }
}

void NativeAudioNode::resetFrame(int channel)
{
    memset(m_Frames[channel], 0, m_Audio->m_OutputParameters->m_BufferSize/m_Audio->m_OutputParameters->m_Channels);
}

void NativeAudioSource::seek(double time)
{
    if (!m_Opened || m_DoSemek) {
        return;
    }

    m_DoSeekTime = time < 0 ? 0 : time;
    m_DoSemek = true;

    NATIVE_PTHREAD_SIGNAL(&m_Audio->m_QueueNeedData);
}

void NativeAudioSource::seekCoro(void *arg)
{
    NativeAudioSource *source = static_cast<NativeAudioSource *>(arg);
    source->seekInternal(source->m_DoSeekTime);
}
void NativeAudioSource::seekInternal(double time)
{
    if (m_ExternallyManaged) {
        avcodec_flush_buffers(m_CodecCtx);
        PaUtil_FlushRingBuffer(m_rBufferOut);
        this->resetFrames();
        m_Eof = false;
        m_Error = 0;
        m_DoNotProcess = false;
    } else {
        int64_t target = 0;
        int flags = 0;
        double clock = this->getClock();

        SPAM(("Seeking source to=%f / position=%f\n", time, m_Clock));

        flags = time > clock ? 0 : AVSEEK_FLAG_BACKWARD;

        target = time * AV_TIME_BASE;

        target = av_rescale_q(target, AV_TIME_BASE_Q, m_Container->streams[m_AudioStream]->time_base);
        int ret = av_seek_frame(m_Container, m_AudioStream, target, flags);
        if (ret >= 0) {
            SPAM(("Seeking success\n"));
            avcodec_flush_buffers(m_CodecCtx);
            PaUtil_FlushRingBuffer(m_rBufferOut);
            this->resetFrames();
            m_Error = 0;
            if (m_Eof && flags == AVSEEK_FLAG_BACKWARD) {
                m_Eof = false;
                m_DoNotProcess = false;
                if (m_Loop) {
                    this->play();
                }
            }
        } else {
            char errorStr[2048];
            av_strerror(ret, errorStr, 2048);
            SPAM(("Seeking error %d : %s\n", ret, errorStr));
            this->sendEvent(SOURCE_EVENT_ERROR, ERR_SEEKING, true);
        }
    }

    if (!m_PacketConsumed) {
        av_free_packet(m_TmpPacket);
    }

    if (m_ExternallyManaged) {
        delete m_TmpPacket;
        m_TmpPacket = NULL;
    }

    m_PacketConsumed = true;
    m_FrameConsumed = true;

    m_DoSemek = false;

    if (m_Reader->m_Async) {
        Coro_switchTo_(m_Coro, m_MainCoro);
    }
}

bool NativeAudioSource::process() {
    if (!m_Opened) {
        SPAM(("Not opened\n"));
        return false;
    }

    if (!m_Playing) {
        SPAM(("Not playing\n"));
        this->resetFrames();
        return false;
    }

    // Make sure enought data is available
    if (m_Audio->m_OutputParameters->m_FramesPerBuffer >= PaUtil_GetRingBufferReadAvailable(m_rBufferOut)) {
        this->resetFrames();
        //SPAM(("Not enought to read\n"));
        // EOF reached, send message to NativeAudio
        if (m_Error == AVERROR_EOF && !m_Eof) {
            SPAM(("     => EOF loop=%d\n", m_Loop));

            m_Eof = true;
            m_DoNotProcess = true;
            this->stop();

            this->sendEvent(SOURCE_EVENT_EOF, 0, true);
        }
        SPAM(("Not enought data to read. return false %ld\n", PaUtil_GetRingBufferReadAvailable(m_rBufferOut)));
        return false;
    }

    // Get the frame
    if (m_OutCount > 1) { // More than 1 channel, need to split
        float *tmp;
        int j;

        j = 0;
        // TODO : malloc each time could be avoided?
        tmp = (float *)malloc(m_Audio->m_OutputParameters->m_BufferSize * m_OutCount);

        PaUtil_ReadRingBuffer(m_rBufferOut, tmp, m_Audio->m_OutputParameters->m_FramesPerBuffer);

        for (int i = 0; i < m_Audio->m_OutputParameters->m_FramesPerBuffer; i++) {
            for (int c = 0; c < m_OutCount; c++) {
                m_Frames[c][i] = tmp[j];
                j++;
            }
        }

        free(tmp);
    } else {
        PaUtil_ReadRingBuffer(m_rBufferOut, m_Frames[0], m_Audio->m_OutputParameters->m_FramesPerBuffer);
    }

    m_NullFrames = false;

    return true;
}

void NativeAudioSource::closeInternal(bool reset)
{
    m_Audio->lockQueue();
    m_Audio->lockSources();

    if (m_Opened) {
        NativePthreadAutoLock lock(&NativeAVSource::m_FfmpegLock);
        avcodec_close(m_CodecCtx);

        if (!m_ExternallyManaged) {
            av_free(m_Container->pb);
            avformat_close_input(&m_Container);
        }

        swr_free(&m_SwrCtx);

        PaUtil_FlushRingBuffer(m_rBufferOut);
        free(m_rBufferOutData);

        m_CodecCtx = NULL;
        m_SwrCtx = NULL;
        m_rBufferOutData = NULL;
    } else {
        if (!m_ExternallyManaged && m_Reader != NULL) {
            av_free(m_Container->pb);
            avformat_free_context(m_Container);

            m_Container = NULL;
        }
    }

    delete m_Reader;
    m_Reader = NULL;

    if (m_Coro != NULL) {
        Coro_free(m_Coro);
        Coro_free(m_MainCoro);
        m_Coro = NULL;
        m_MainCoro = NULL;
    }

    if (!m_PacketConsumed) {
        av_free_packet(m_TmpPacket);
    }
    if (!reset && !m_ExternallyManaged) {
        delete m_TmpPacket;
    }

    if (m_TmpFrame.data != NULL) {
        free(m_TmpFrame.data);
        m_TmpFrame.data = NULL;
    }
    m_TmpFrame.size = 0;
    m_TmpFrame.nbSamples = 0;

    if (m_fCvt != NULL) {
        delete m_fCvt;
        free(m_fBufferOutData);
        m_fCvt = NULL;
    }

    delete m_rBufferOut;
    m_rBufferOut = NULL;

    m_Playing = false;
    m_PlayWhenReady = false;
    m_FrameConsumed = true;
    m_PacketConsumed = true;
    m_Opened = false;
    m_AudioStream = -1;
    m_Container = NULL;
    m_AvioBuffer = NULL;

    m_Audio->unlockQueue();
    m_Audio->unlockSources();
}

void NativeAudioSource::play()
{
    if (!m_Opened) {
        m_PlayWhenReady = true;
        return;
    }

    m_Playing = true;
    m_Stopped = false;
    m_PlayWhenReady = false;

    SPAM(("Play source @ %p\n", this));

    NATIVE_PTHREAD_SIGNAL(&m_Audio->m_QueueNeedData);

    this->sendEvent(SOURCE_EVENT_PLAY, 0, false);
}

void NativeAudioSource::pause()
{
    if (!m_Opened) {
        return;
    }

    m_Playing = false;
    this->sendEvent(SOURCE_EVENT_PAUSE, 0, false);
}

void NativeAudioSource::stop()
{
    if (!m_ExternallyManaged) {
        this->seek(0);
    }

    m_Stopped = true;
    m_Playing = false;
    m_PlayWhenReady = false;

    this->resetFrames();

    this->sendEvent(SOURCE_EVENT_STOP, 0, false);
}

void NativeAudioSource::close()
{
    this->closeInternal(false);
#if 0
    if (!m_Opened) {
        return;
    }

    m_Playing = false;
    m_Stopped = true;
    m_Opened = false;
    SPAM(("Stoped\n"));

    if (!m_PacketConsumed) {
        av_free_packet(m_TmpPacket);
    }

    PaUtil_FlushRingBuffer(m_rBufferOut);

    m_SamplesConsumed = 0;
    m_FrameConsumed = true;
    m_PacketConsumed = true;

    this->resetFrames();

    avcodec_flush_buffers(m_CodecCtx);
#endif
}

bool NativeAudioSource::isActive()
{
    return m_Playing && m_Error != AVERROR_EOF;
}


NativeAudioSource::~NativeAudioSource() {
    m_Audio->removeSource(this);
    this->closeInternal(false);
}

NativeAudioProcessor::~NativeAudioProcessor() {};

