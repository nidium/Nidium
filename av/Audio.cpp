#include "Audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <portaudio.h>
#include "pa_ringbuffer.h"

extern "C" {
#include "libavformat/avformat.h"
}

#include "AudioNodeGain.h"
#include "AudioNodeDelay.h"

namespace Nidium {
namespace AV {

// {{{ Functions
// Next power of 2
// Taken from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static uint32_t upperPow2(uint32_t num)
{
    uint32_t n = num > 0 ? num - 1 : 0;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}
// }}}

// {{{ NativeAudio
NativeAudio::NativeAudio(ape_global *n, unsigned int bufferSize, unsigned int channels, unsigned int sampleRate)
    : m_Net(n), m_SourcesCount(0), m_PlaybackStartTime(0), m_PlaybackConsumedFrame(0),
      m_Output(NULL), m_InputStream(NULL), m_OutputStream(NULL),
      m_rBufferOutData(NULL), m_volume(1), m_SourceNeedWork(false), m_QueueFreeLock(false),
      m_SharedMsgFlush(false), m_ThreadShutdown(false), m_Sources(NULL), m_MainCtx(NULL)
{
    NATIVE_PTHREAD_VAR_INIT(&m_QueueHaveData);
    NATIVE_PTHREAD_VAR_INIT(&m_QueueHaveSpace);
    NATIVE_PTHREAD_VAR_INIT(&m_QueueNeedData);
    NATIVE_PTHREAD_VAR_INIT(&m_QueueMessagesFlushed);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_SourcesLock, &mta);
    pthread_mutex_init(&m_RecurseLock, &mta);

    av_register_all();

    Pa_Initialize();

    m_SharedMsg = new Nidium::Core::SharedMessages();

    int actualBufferSize = bufferSize;
    if (bufferSize == 0) {
        NativeAudioParameters tmp(0, 0, channels, NativeAudio::FLOAT32, sampleRate);
        actualBufferSize = NativeAudio::GetOutputBufferSize(&tmp);
        if (actualBufferSize == 0) {
            fprintf(stderr, "[NativeAudio] Failed to request optimal buffer size. Defaulting to 4096\n");
            actualBufferSize = 4096;
        } else {
            /*
             * Portaudio does not necessarily give us a power of 2 for the
             * buffer size, which is not appropriate for audio processing
             */
            actualBufferSize = upperPow2(actualBufferSize) * NativeAudio::FLOAT32;
        }
    }

    m_OutputParameters = new NativeAudioParameters(bufferSize, actualBufferSize, channels, NativeAudio::FLOAT32, sampleRate);

    /*
     * Portaudio ring buffer require a power of two
     * for the number of elements in the ring buffer
     */
    uint32_t count = upperPow2(actualBufferSize * NATIVE_AUDIO_BUFFER_MULTIPLIER * channels);

    m_rBufferOut = new PaUtilRingBuffer();
    m_rBufferOutData = (float *)calloc(count, NativeAudio::FLOAT32);

    PaUtil_InitializeRingBuffer(m_rBufferOut, NativeAudio::FLOAT32,
            count/NativeAudio::FLOAT32, m_rBufferOutData);

    pthread_create(&m_ThreadDecode, NULL, NativeAudio::decodeThread, this);
    pthread_create(&m_ThreadQueue, NULL, NativeAudio::queueThread, this);
}

void *NativeAudio::queueThread(void *args)
{
    NativeAudio *audio = static_cast<NativeAudio *>(args);
    bool wrote;
    bool needSpace;

    while (true) {
        bool msgFlush = audio->m_SharedMsgFlush;

        audio->readMessages(msgFlush);

        if (msgFlush && audio->m_SharedMsgFlush) {
            audio->m_SharedMsgFlush = false;
            NATIVE_PTHREAD_SIGNAL(&audio->m_QueueMessagesFlushed);
        }

        // Using a trylock because we don't want to wait for another thread to
        // finish working with the recurseLock for being able to read shared messages.
        //
        // Example : For shutdown, we need to suspend the queue thread (using
        // NativeAudio::lockQueue()) but we also need the ability to read
        // shared messages for shuting down various nodes.
        int lockErr = pthread_mutex_trylock(&audio->m_RecurseLock);

        if (lockErr == 0 && audio->m_Output != NULL && msgFlush != true) {
            wrote = false;
            needSpace = false;

            if (audio->m_ThreadShutdown) {
                pthread_mutex_unlock(&audio->m_RecurseLock);
                break;
            }

            for (;;) {
                if (!audio->canWriteFrame()) {
                    needSpace = true;
                    break;
                } else if (audio->m_QueueFreeLock) {
                    usleep(500);
                    break;
                }

                audio->processQueue();

                if (!audio->m_Output->m_Processed) {
                    break;
                }

                wrote = true;
                audio->m_Output->m_Processed = false;

                // Copy output node frame data to output ring buffer
                // XXX : Find a more efficient way to copy data to output right buffer
                for (int i = 0; i < audio->m_OutputParameters->m_FramesPerBuffer; i++) {
                    for (int j = 0; j < audio->m_OutputParameters->m_Channels; j++) {
                        audio->m_Output->m_Frames[j][i] *= audio->m_volume;
                        PaUtil_WriteRingBuffer(audio->m_rBufferOut, &audio->m_Output->m_Frames[j][i], 1);
                    }
                }
            }

            if (wrote) {
                SPAM(("Sending queueNeedData\n"));
                NATIVE_PTHREAD_SIGNAL(&audio->m_QueueNeedData);
            }
        }

        if (lockErr == 0) {
            pthread_mutex_unlock(&audio->m_RecurseLock);
        }

        if (audio->m_ThreadShutdown) break;

        if (needSpace) {
            SPAM(("Waiting for more space\n"));
            NATIVE_PTHREAD_WAIT(&audio->m_QueueHaveSpace)
        } else {
            SPAM(("Waiting for more data\n"));
            NATIVE_PTHREAD_WAIT(&audio->m_QueueHaveData)
        }

        if (audio->m_ThreadShutdown) break;

        SPAM(("Queue thead is now working shutdown=%d\n", audio->m_ThreadShutdown));
    }

    audio->readMessages(true);

    SPAM(("Exiting queueThread\n"));

    return NULL;
}
// }}}

// {{{ Messages
void NativeAudio::readMessages()
{
    this->readMessages(false);
}

void NativeAudio::readMessages(bool flush)
{
#define MAX_MSG_IN_ROW 1024
    Nidium::Core::SharedMessages::Message *msg;
    int nread = 0;
    while (((!flush && ++nread < MAX_MSG_IN_ROW) || flush) && (msg = m_SharedMsg->readMessage())) {
        switch (msg->event()) {
            case NATIVE_AUDIO_NODE_CALLBACK : {
                NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg->dataPtr());
                cbkMsg->m_Cbk(cbkMsg->m_Node, cbkMsg->m_Custom);
                delete cbkMsg;
            }
            break;
            case NATIVE_AUDIO_NODE_SET : {
                NativeAudioNode::Message *nodeMsg =  static_cast<NativeAudioNode::Message *>(msg->dataPtr());
                if (nodeMsg->m_Arg->m_Ptr == NULL) {
                    nodeMsg->m_Arg->m_Cbk(nodeMsg->m_Node, nodeMsg->m_Arg->m_Id, nodeMsg->m_Val, nodeMsg->m_Size);
                } else {
                    memcpy(nodeMsg->m_Arg->m_Ptr, nodeMsg->m_Val, nodeMsg->m_Size);
                }

                delete nodeMsg;
            }
            break;
            case NATIVE_AUDIO_CALLBACK :
                NativeAudio::CallbackMessage *cbkMsg = static_cast<NativeAudio::CallbackMessage*>(msg->dataPtr());
                cbkMsg->m_Cbk(cbkMsg->m_Custom);
                delete cbkMsg;
            break;
        }
        delete msg;
    }
#undef MAX_MSG_IN_ROW
}
// }}}

void NativeAudio::processQueue()
{
    SPAM(("process queue\n"));
    NativeAudioSources *sources = m_Sources;

    while (sources!= NULL)
    {
        SPAM(("curr=%p connected=%d\n", m_Sources->curr, m_Sources->curr->m_IsConnected));
        if (sources->curr != NULL && sources->curr->m_IsConnected) {
            sources->curr->processQueue();
        }
        sources = sources->next;
    }
    SPAM(("-------------------------------- finished\n"));
}

void *NativeAudio::decodeThread(void *args)
{
    NativeAudio *audio = static_cast<NativeAudio *>(args);

    NativeAudioSources *sources;
    NativeAudioSource *source;
    int haveEnought, sourcesCount;

    for(; ;) {
        // Go through all the sources that need data to be decoded
        pthread_mutex_lock(&audio->m_SourcesLock);

        sources = audio->m_Sources;
        haveEnought = 0;
        sourcesCount = audio->m_SourcesCount;

        while (sources != NULL)
        {
            haveEnought = 0;

            if (sources->curr != NULL && !sources->externallyManaged) {
                source = static_cast<NativeAudioSource*>(sources->curr);

                // Loop as long as there is data to read and write
                while (source->work()) {}

                if (source->avail() >= audio->m_OutputParameters->m_FramesPerBuffer * audio->m_OutputParameters->m_Channels) {
                    haveEnought++;
                    //audio->notEmpty = false;
                }

                if (!source->m_Opened || !source->m_Playing) {
                    sourcesCount--;
                }
            }
            sources = sources->next;
        }
        pthread_mutex_unlock(&audio->m_SourcesLock);
        SPAM(("haveEnought %d / sourcesCount %d\n", haveEnought, audio->m_SourcesCount));

        // FIXME : find out why when playing multiple song,
        // the commented expression bellow fail to work
        if (audio->m_SourcesCount > 0 /*&& haveEnought == sourcesCount*/) {
            if (audio->canWriteFrame()) {
                SPAM(("Have data %lu\n", PaUtil_GetRingBufferWriteAvailable(audio->m_rBufferOut)));
                NATIVE_PTHREAD_SIGNAL(&audio->m_QueueHaveData);
                //audio->haveData = true;
            }  else {
                SPAM(("dont Have data %lu\n", PaUtil_GetRingBufferWriteAvailable(audio->m_rBufferOut)));
            }
        }

        NATIVE_AUDIO_CHECK_EXIT_THREAD

        // Wait for work to do unless some source need to wakeup
        if (!audio->m_SourceNeedWork) {
            SPAM(("Waitting for queueNeedData m_SourceNeedWork=%d\n", audio->m_SourceNeedWork));
            NATIVE_PTHREAD_WAIT(&audio->m_QueueNeedData);
            SPAM(("QueueNeedData received\n"));
        } else {
            SPAM(("decodeThread not sleeping cause it need wakup\n"));
        }

        audio->m_SourceNeedWork = false;

        NATIVE_AUDIO_CHECK_EXIT_THREAD
    }

    SPAM(("Exiting\n"));
    return NULL;
}

int NativeAudio::InitPortAudioOutput(NativeAudioParameters *params, \
        PaStream **outputStream, PaStreamCallback *callback, void *userData) {
    const PaDeviceInfo *infos;
    PaDeviceIndex device;
    PaError error;
    PaStreamParameters paOutputParameters;

    // TODO : Device should be defined by user
    device = Pa_GetDefaultOutputDevice();
    if (device == paNoDevice) {
        return 1;
    }

    infos = Pa_GetDeviceInfo(device);

    // Set output parameters for PortAudio
    paOutputParameters.device = device;
    paOutputParameters.channelCount = params->m_Channels;
    paOutputParameters.suggestedLatency = infos->defaultLowInputLatency;
    paOutputParameters.hostApiSpecificStreamInfo = 0; /* no api specific data */
    paOutputParameters.sampleFormat = paFloat32;

    // Try to open the output
    error = Pa_OpenStream(outputStream,
            0,
            &paOutputParameters,
            params->m_SampleRate,
            0,
            paPrimeOutputBuffersUsingStreamCallback,
            callback,
            userData);

    if (error) {
        fprintf(stderr, "[NativeAudio] Error opening output. Code = %i\n", error);
        Pa_Terminate();
        return error;
    }

    error = Pa_StartStream(*outputStream);
    if (error) {
        fprintf(stderr, "[NativeAudio] Failed to start PortAudio stream. Code=%i\n", error);
        return 2;
    }

    return 0;
}

/*
 * Return the number of *samples* per buffer. This method should only be used when no portaudio
 * stream is running. It's intended  to get an estimation of the audio bufer size selected by
 * portaudio before allocating the buffers for NativeAudio
 */
int NativeAudio::GetOutputBufferSize(NativeAudioParameters *params) {
    PaStream *outputStream = NULL;

    NativeAudio::InitPortAudioOutput(params, &outputStream, nullptr, nullptr);
    if (outputStream == NULL) {
        return 0;
    }

    const PaStreamInfo *streamInfo = Pa_GetStreamInfo(outputStream);

    double latency = streamInfo == nullptr ? 0 : streamInfo->outputLatency;

    Pa_StopStream(outputStream);
    Pa_CloseStream(outputStream);

    if (latency > 0) {
        return latency * params->m_SampleRate;
    } else {
        return 0;
    }
}

int NativeAudio::openOutput() {
    return NativeAudio::InitPortAudioOutput(m_OutputParameters, &m_OutputStream, &NativeAudio::paOutputCallback, (void *)this);
}

int NativeAudio::paOutputCallbackMethod(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags)
{
    (void) timeInfo;
    (void) statusFlags;
    (void) inputBuffer;

    float *out = (float*)outputBuffer;
    int channels = m_OutputParameters->m_Channels;
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(m_rBufferOut) / channels;
    avail = framesPerBuffer > avail ? avail : framesPerBuffer;
    int left = framesPerBuffer - avail;

    if (m_PlaybackStartTime == 0 || (statusFlags & paOutputUnderflow)) {
        /*
         * Reset in case of underflow, since it will
         * break the calculation  of the buffer size
         */
        m_PlaybackStartTime = av_gettime();
        m_PlaybackConsumedFrame = 0;
    }

    if (m_PlaybackStartTime > 0) {
        m_PlaybackConsumedFrame += framesPerBuffer;
    }

    PaUtil_ReadRingBuffer(m_rBufferOut, out, avail * channels);

#if 0
    //SPAM(("===========frames per buffer = %d avail = %ld processing = %ld left = %d\n",
        //framesPerBuffer, PaUtil_GetRingBufferReadAvailable(m_rBufferOut), avail, left));
    if (left > 0) {
        SPAM(("WARNING DROPING %d\n", left));
    }
#endif

    for (unsigned int i = 0; i < left * channels; i++)
    {
        *out++ = 0.0f;
    }

    NATIVE_PTHREAD_SIGNAL(&m_QueueHaveSpace);

    return paContinue;
}

int NativeAudio::paOutputCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    return (static_cast<NativeAudio*>(userData))->paOutputCallbackMethod(inputBuffer, outputBuffer,
        framesPerBuffer,
        timeInfo,
        statusFlags);
}

bool NativeAudio::haveSourceActive(bool excludeExternal)
{
    pthread_mutex_lock(&m_SourcesLock);
    NativeAudioSources *sources = m_Sources;
    while (sources != NULL)
    {
        NativeAudioNode *node = sources->curr;
        if (node != NULL && (!excludeExternal || (excludeExternal && !sources->externallyManaged))) {
            if (node->isActive()) {
                pthread_mutex_unlock(&m_SourcesLock);
                return true;
            }
        }
        sources = sources->next;
    }
    pthread_mutex_unlock(&m_SourcesLock);
    return false;
}

double NativeAudio::getLatency() {
    ring_buffer_size_t queuedAudio = PaUtil_GetRingBufferReadAvailable(m_rBufferOut);
    double nativeAudioLatency = (((queuedAudio) * (1.0/m_OutputParameters->m_SampleRate)) / m_OutputParameters->m_Channels);
    double paLatency = 0;

    if (m_PlaybackStartTime != 0) {
        int64_t now = av_gettime();
        double playbackDuration = ((1.0/m_OutputParameters->m_SampleRate)
                    * (double)m_PlaybackConsumedFrame);

        paLatency = playbackDuration - (double)(now - m_PlaybackStartTime)/1000000;
    }

    SPAM(("latency=%f native=%f pa=%f\n", paLatency + nativeAudioLatency, nativeAudioLatency, paLatency));

    return paLatency + nativeAudioLatency;
}

NativeAudioNode *NativeAudio::addSource(NativeAudioNode *source, bool externallyManaged)
{
    NativeAudioSources *sources = new NativeAudioSources();

    this->lockSources();
    this->lockQueue();

    sources->curr = source;
    sources->externallyManaged = externallyManaged;
    sources->prev = NULL;
    sources->next = m_Sources;

    if (m_Sources != NULL) {
        m_Sources->prev = sources;
    }

    m_Sources = sources;
    m_SourcesCount++;

    this->unlockSources();
    this->unlockQueue();

    return sources->curr;
}

void NativeAudio::removeSource(NativeAudioSource *source)
{
    this->lockSources();
    this->lockQueue();

    NativeAudioSources *sources = m_Sources;

    while (sources != NULL)
    {
        if (sources->curr != NULL && sources->curr == source) {

            if (sources->prev != NULL) {
                sources->prev->next = sources->next;
            } else {
                m_Sources = sources->next;
            }
            if (sources->next != NULL) {
                sources->next->prev = sources->prev;
            }

            delete sources;

            this->unlockSources();
            this->unlockQueue();

            return;
        }
        sources = sources->next;
    }

    this->unlockSources();
    this->unlockQueue();
}

NativeAudioNode *NativeAudio::createNode(NativeAudio::Node node, int input, int output)
{
    try {
        switch (node) {
            case SOURCE:
                {
                    NativeAudioNode *source = new NativeAudioSource(output, this, false);
                    return this->addSource(source, false);
                }
                break;
            case CUSTOM_SOURCE:
                {
                    NativeAudioNode *source = new NativeAudioCustomSource(output, this);
                    return this->addSource(source, true);
                }
                break;
            case GAIN:
                    return new NativeAudioNodeGain(input, output, this);
                break;
            case CUSTOM:
                    return new NativeAudioNodeCustom(input, output, this);
                break;
            case REVERB:
                    return new NativeAudioNodeReverb(input, output, this);
                break;
            case DELAY:
                    return new NativeAudioNodeDelay(input, output, this);
                break;
            case STEREO_ENHANCER:
                    return new NativeAudioNodeStereoEnhancer(input, output, this);
                break;
            case TARGET:
                    if (m_Output == NULL) {
                        m_Output = new NativeAudioNodeTarget(input, output, this);
                        return m_Output;
                    }
                    return NULL;
                break;
            default :
                break;
        }
    } catch (NativeAudioNodeException *e) {
        throw;
    }

    return NULL;
}

bool NativeAudio::connect(NodeLink *input, NodeLink *output)
{
    return output->node->queue(input, output);
}

bool NativeAudio::disconnect(NodeLink *input, NodeLink *output)
{
    return output->node->unqueue(input, output);
}

void NativeAudio::setVolume(float volume)
{
    m_volume = volume;
}

float NativeAudio::getVolume()
{
    return m_volume;
}

void NativeAudio::wakeup()
{
    m_SharedMsgFlush = true;
    m_QueueFreeLock = true;

    NATIVE_PTHREAD_SIGNAL(&m_QueueHaveData);
    NATIVE_PTHREAD_SIGNAL(&m_QueueHaveSpace);

    NATIVE_PTHREAD_WAIT(&m_QueueMessagesFlushed);

    m_QueueFreeLock = false;
}

void NativeAudio::shutdown()
{
    m_ThreadShutdown = true;

    NATIVE_PTHREAD_SIGNAL(&m_QueueHaveData)
    NATIVE_PTHREAD_SIGNAL(&m_QueueHaveSpace)
    NATIVE_PTHREAD_SIGNAL(&m_QueueNeedData)

    pthread_join(m_ThreadQueue, NULL);
    pthread_join(m_ThreadDecode, NULL);
}

void NativeAudio::lockQueue()
{
    m_QueueFreeLock = true;
    pthread_mutex_lock(&m_RecurseLock);
    m_QueueFreeLock = false;
}

void NativeAudio::unlockQueue()
{
    pthread_mutex_unlock(&m_RecurseLock);
}

void NativeAudio::sourceNeedWork(void *ptr)
{
    NativeAudio *thiz = static_cast<NativeAudio*>(ptr);
    thiz->m_SourceNeedWork = true;
    NATIVE_PTHREAD_SIGNAL(&thiz->m_QueueNeedData);
}

void NativeAudio::lockSources()
{
    pthread_mutex_lock(&m_SourcesLock);
}

void NativeAudio::unlockSources()
{
    pthread_mutex_unlock(&m_SourcesLock);
}

void NativeAudio::postMessage(AudioMessageCallback cbk, void *custom, bool block)
{
    m_SharedMsg->postMessage((void *)new CallbackMessage(cbk, custom),
            NATIVE_AUDIO_CALLBACK);

    if (block) {
        this->wakeup();
    }
}
void NativeAudio::postMessage(AudioMessageCallback cbk, void *custom)
{
    this->postMessage(cbk, custom, false);
}

bool NativeAudio::canWriteFrame()
{
    return PaUtil_GetRingBufferWriteAvailable(m_rBufferOut) >=
        m_OutputParameters->m_FramesPerBuffer * m_OutputParameters->m_Channels;
}

NativeAudio::~NativeAudio() {
    if (m_ThreadShutdown == false) {
        this->shutdown();
    }

    if (m_OutputStream != NULL) {
        Pa_StopStream(m_OutputStream);
        Pa_CloseStream(m_OutputStream);
        m_OutputStream = nullptr;
    }

    if (m_InputStream != NULL) {
        Pa_StopStream(m_InputStream);
        Pa_CloseStream(m_InputStream);
        m_InputStream = nullptr;
    }

    Pa_Terminate();

    free(m_rBufferOutData);

    delete m_OutputParameters;
    delete m_rBufferOut;
    delete m_SharedMsg;
}

} // namespace AV
} // namespace Nidium

