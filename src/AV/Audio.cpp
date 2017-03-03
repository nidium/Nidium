/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
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

using Nidium::Core::SharedMessages;

namespace Nidium {
namespace AV {

// {{{ Functions
// Next power of 2
// Taken from
// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
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

// {{{ Audio
Audio::Audio(ape_global *n,
             unsigned int bufferSize,
             unsigned int channels,
             unsigned int sampleRate)
    : m_Net(n), m_SourcesCount(0), m_PlaybackStartTime(0),
      m_PlaybackConsumedFrame(0), m_Output(NULL), m_InputStream(NULL),
      m_OutputStream(NULL), m_rBufferOutData(NULL), m_volume(1),
      m_SourceNeedWork(false), m_QueueFreeLock(false), m_SharedMsgFlush(false),
      m_ThreadShutdown(false), m_Sources(NULL)
{
    NIDIUM_PTHREAD_VAR_INIT(&m_QueueHaveData);
    NIDIUM_PTHREAD_VAR_INIT(&m_QueueHaveSpace);
    NIDIUM_PTHREAD_VAR_INIT(&m_QueueNeedData);
    NIDIUM_PTHREAD_VAR_INIT(&m_QueueMessagesFlushed);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_SourcesLock, &mta);
    pthread_mutex_init(&m_RecurseLock, &mta);

    av_register_all();

    Pa_Initialize();

    m_SharedMsg = new SharedMessages();

    if (channels == 0) channels = 2;
    if (sampleRate == 0) sampleRate = 44100;

    int actualBufferSize = bufferSize;
    if (bufferSize == 0) {
        AudioParameters tmp(0, 0, channels, Audio::FLOAT32, sampleRate);
        actualBufferSize = Audio::GetOutputBufferSize(&tmp);
        if (actualBufferSize == 0) {
            ndm_log(NDM_LOG_ERROR, "Audio",
                    "Failed to request optimal buffer size. Defaulting "
                    "to 4096");
            actualBufferSize = 4096;
        } else {
            /*
             * Portaudio does not necessarily give us a power of 2 for the
             * buffer size, which is not appropriate for audio processing
             */
            actualBufferSize = upperPow2(actualBufferSize) * Audio::FLOAT32;
        }
    }

    m_OutputParameters = new AudioParameters(
        bufferSize, actualBufferSize, channels, Audio::FLOAT32, sampleRate);

    /*
     * Portaudio ring buffer require a power of two
     * for the number of elements in the ring buffer
     */
    uint32_t count = upperPow2(actualBufferSize * NIDIUM_AUDIO_BUFFER_MULTIPLIER
                               * channels);

    m_rBufferOut     = new PaUtilRingBuffer();
    m_rBufferOutData = static_cast<float *>(calloc(count, Audio::FLOAT32));

    PaUtil_InitializeRingBuffer(m_rBufferOut, Audio::FLOAT32,
                                count / Audio::FLOAT32, m_rBufferOutData);

    pthread_create(&m_ThreadDecode, NULL, Audio::decodeThread, this);
    pthread_create(&m_ThreadQueue, NULL, Audio::queueThread, this);
}

void *Audio::queueThread(void *args)
{
    Audio *audio = static_cast<Audio *>(args);
    bool wrote;
    bool needSpace;

    while (true) {
        bool msgFlush = audio->m_SharedMsgFlush;

        audio->readMessages(msgFlush);

        if (msgFlush && audio->m_SharedMsgFlush) {
            audio->m_SharedMsgFlush = false;
            NIDIUM_PTHREAD_SIGNAL(&audio->m_QueueMessagesFlushed);
        }

        // Using a trylock because we don't want to wait for another thread to
        // finish working with the recurseLock for being able to read shared
        // messages.
        //
        // Example : For shutdown, we need to suspend the queue thread (using
        // Audio::lockQueue()) but we also need the ability to read
        // shared messages for shuting down various nodes.
        int lockErr = pthread_mutex_trylock(&audio->m_RecurseLock);

        if (lockErr == 0 && audio->m_Output != NULL && msgFlush != true) {
            wrote     = false;
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

                wrote                        = true;
                audio->m_Output->m_Processed = false;

                // Copy output node frame data to output ring buffer
                // XXX : Find a more efficient way to copy data to output right
                // buffer
                for (int i = 0;
                     i < audio->m_OutputParameters->m_FramesPerBuffer; i++) {
                    for (int j = 0; j < audio->m_OutputParameters->m_Channels;
                         j++) {
                        audio->m_Output->m_Frames[j][i] *= audio->m_volume;
                        PaUtil_WriteRingBuffer(audio->m_rBufferOut,
                                               &audio->m_Output->m_Frames[j][i],
                                               1);
                    }
                }
            }

            if (wrote) {
                SPAM(("[Audio] Sending queueNeedData\n"));
                NIDIUM_PTHREAD_SIGNAL(&audio->m_QueueNeedData);
            }
        }

        if (lockErr == 0) {
            pthread_mutex_unlock(&audio->m_RecurseLock);
        }

        if (audio->m_ThreadShutdown) break;

        if (needSpace) {
            SPAM(("[Audio] Waiting for more space\n"));
            NIDIUM_PTHREAD_WAIT(&audio->m_QueueHaveSpace)
        } else {
            SPAM(("[Audio] Waiting for more data\n"));
            NIDIUM_PTHREAD_WAIT(&audio->m_QueueHaveData)
        }

        if (audio->m_ThreadShutdown) break;

        SPAM(("[Audio] Queue thead is now working shutdown=%d\n",
              audio->m_ThreadShutdown));
    }

    audio->readMessages(true);

    SPAM(("[Audio] Exiting queueThread\n"));

    return NULL;
}
// }}}

// {{{ Messages
void Audio::readMessages()
{
    this->readMessages(false);
}

void Audio::readMessages(bool flush)
{
#define MAX_MSG_IN_ROW 1024
    SharedMessages::Message *msg;
    int nread = 0;
    while (((!flush && ++nread < MAX_MSG_IN_ROW) || flush)
           && (msg = m_SharedMsg->readMessage())) {
        switch (msg->event()) {
            case NIDIUM_AUDIO_NODE_CALLBACK: {
                AudioNode::CallbackMessage *cbkMsg
                    = static_cast<AudioNode::CallbackMessage *>(msg->dataPtr());
                cbkMsg->m_Cbk(cbkMsg->m_Node, cbkMsg->m_Custom);
                delete cbkMsg;
            } break;
            case NIDIUM_AUDIO_NODE_SET: {
                AudioNode::Message *nodeMsg
                    = static_cast<AudioNode::Message *>(msg->dataPtr());
                if (nodeMsg->m_Arg->m_Ptr == NULL) {
                    nodeMsg->m_Arg->m_Cbk(nodeMsg->m_Node, nodeMsg->m_Arg->m_Id,
                                          nodeMsg->m_Val, nodeMsg->m_Size);
                } else {
                    memcpy(nodeMsg->m_Arg->m_Ptr, nodeMsg->m_Val,
                           nodeMsg->m_Size);
                }

                delete nodeMsg;
            } break;
            case NIDIUM_AUDIO_CALLBACK:
                Audio::CallbackMessage *cbkMsg
                    = static_cast<Audio::CallbackMessage *>(msg->dataPtr());
                cbkMsg->m_Cbk(cbkMsg->m_Custom);
                delete cbkMsg;
                break;
        }
        delete msg;
    }
#undef MAX_MSG_IN_ROW
}
// }}}

void Audio::processQueue()
{
    SPAM(("[Audio] Process queue\n"));
    AudioSources *sources = m_Sources;

    while (sources != NULL) {
        SPAM(("[Audio] curr=%p connected=%d\n", m_Sources->curr,
              m_Sources->curr->m_IsConnected));
        if (sources->curr != NULL && sources->curr->m_IsConnected) {
            sources->curr->processQueue();
        }
        sources = sources->next;
    }
    SPAM(("[Audio]-------------------------- finished\n"));
}

void *Audio::decodeThread(void *args)
{
    Audio *audio = static_cast<Audio *>(args);

    AudioSources *sources;
    AudioSource *source;

    for (;;) {
        int haveEnough, sourcesCount;
        // Go through all the sources that need data to be decoded
        pthread_mutex_lock(&audio->m_SourcesLock);

        sources      = audio->m_Sources;
        haveEnough   = 0;
        sourcesCount = audio->m_SourcesCount;

        while (sources != NULL) {
            haveEnough = 0;

            if (sources->curr != NULL && !sources->externallyManaged) {
                source = static_cast<AudioSource *>(sources->curr);

                // Loop as long as there is data to read and write
                while (source->work()) {
                }

                if (source->avail()
                    >= audio->m_OutputParameters->m_FramesPerBuffer
                           * audio->m_OutputParameters->m_Channels) {
                    haveEnough++;
                    // audio->notEmpty = false;
                }

                if (!source->m_Opened || !source->m_Playing) {
                    sourcesCount--;
                }
            }
            sources = sources->next;
        }
        pthread_mutex_unlock(&audio->m_SourcesLock);
        SPAM(("[Audio] haveEnough %d / sourcesCount %d\n", haveEnough,
              audio->m_SourcesCount));

        // FIXME : find out why when playing multiple song,
        // the commented expression bellow fail to work
        if (audio->m_SourcesCount > 0 /*&& haveEnough == sourcesCount*/) {
            if (audio->canWriteFrame()) {
                SPAM(("[Audio] Have data %lu\n",
                      PaUtil_GetRingBufferWriteAvailable(audio->m_rBufferOut)));
                NIDIUM_PTHREAD_SIGNAL(&audio->m_QueueHaveData);
                // audio->haveData = true;
            } else {
                SPAM(("[Audio] Does not have data %lu\n",
                      PaUtil_GetRingBufferWriteAvailable(audio->m_rBufferOut)));
            }
        }

        NIDIUM_AUDIO_CHECK_EXIT_THREAD

        // Wait for work to do unless some source need to wakeup
        if (!audio->m_SourceNeedWork) {
            SPAM(("[Audio] Waitting for queueNeedData m_SourceNeedWork=%d\n",
                  audio->m_SourceNeedWork));
            NIDIUM_PTHREAD_WAIT(&audio->m_QueueNeedData);
            SPAM(("[Audio] QueueNeedData received\n"));
        } else {
            SPAM(("[Audio] decodeThread not sleeping cause it need wakup\n"));
        }

        audio->m_SourceNeedWork = false;

        NIDIUM_AUDIO_CHECK_EXIT_THREAD
    }

    SPAM(("[Audio] Exiting\n"));
    return NULL;
}

int Audio::InitPortAudioOutput(AudioParameters *params,
                               PaStream **outputStream,
                               PaStreamCallback *callback,
                               void *userData)
{
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
    paOutputParameters.device                    = device;
    paOutputParameters.channelCount              = params->m_Channels;
    paOutputParameters.suggestedLatency          = infos->defaultLowInputLatency;
    paOutputParameters.hostApiSpecificStreamInfo = 0; /* no api specific data */
    paOutputParameters.sampleFormat              = paFloat32;

    // Try to open the output
    error = Pa_OpenStream(
        outputStream, 0, &paOutputParameters, params->m_SampleRate, 0,
        paPrimeOutputBuffersUsingStreamCallback, callback, userData);

    if (error) {
        ndm_logf(NDM_LOG_ERROR, "Audio", "Error opening output. Code = %i", error);
        Pa_Terminate();
        return error;
    }

    error = Pa_StartStream(*outputStream);
    if (error) {
        ndm_logf(NDM_LOG_ERROR, "Audio", "Failed to start PortAudio stream. Code=%i",
                error);
        return 2;
    }

    return 0;
}

/*
 * Return the number of *samples* per buffer. This method should only be used
 * when no portaudio
 * stream is running. It's intended  to get an estimation of the audio bufer
 * size selected by
 * portaudio before allocating the buffers for Audio
 */
int Audio::GetOutputBufferSize(AudioParameters *params)
{
    PaStream *outputStream = NULL;

    Audio::InitPortAudioOutput(params, &outputStream, nullptr, nullptr);
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

int Audio::openOutput()
{
    return Audio::InitPortAudioOutput(m_OutputParameters, &m_OutputStream,
                                      &Audio::paOutputCallback, (void *)this);
}

int Audio::paOutputCallbackMethod(const void *inputBuffer,
                                  void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *timeInfo,
                                  PaStreamCallbackFlags statusFlags)
{
    (void)timeInfo;
    (void)statusFlags;
    (void)inputBuffer;

    float *out   = static_cast<float *>(outputBuffer);
    int channels = m_OutputParameters->m_Channels;
    ring_buffer_size_t avail
        = PaUtil_GetRingBufferReadAvailable(m_rBufferOut) / channels;
    avail    = framesPerBuffer > avail ? avail : framesPerBuffer;
    int left = framesPerBuffer - avail;

    if (m_PlaybackStartTime == 0 || (statusFlags & paOutputUnderflow)) {
        /*
         * Reset in case of underflow, since it will
         * break the calculation  of the buffer size
         */
        m_PlaybackStartTime     = av_gettime();
        m_PlaybackConsumedFrame = 0;
    }

    if (m_PlaybackStartTime > 0) {
        m_PlaybackConsumedFrame += framesPerBuffer;
    }

    PaUtil_ReadRingBuffer(m_rBufferOut, out, avail * channels);

#if 0
    //SPAM(("[Audio] ===========frames per buffer = %d avail = %ld processing = %ld left = %d\n",
        //framesPerBuffer, PaUtil_GetRingBufferReadAvailable(m_rBufferOut), avail, left));
    if (left > 0) {
        SPAM(("[Audio] WARNING DROPING %d\n", left));
    }
#endif

    for (unsigned int i = 0; i < left * channels; i++) {
        *out++ = 0.0f;
    }

    NIDIUM_PTHREAD_SIGNAL(&m_QueueHaveSpace);

    return paContinue;
}

int Audio::paOutputCallback(const void *inputBuffer,
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    return (static_cast<Audio *>(userData))
        ->paOutputCallbackMethod(inputBuffer, outputBuffer, framesPerBuffer,
                                 timeInfo, statusFlags);
}

bool Audio::haveSourceActive(bool excludeExternal)
{
    pthread_mutex_lock(&m_SourcesLock);
    AudioSources *sources = m_Sources;
    while (sources != NULL) {
        AudioNode *node = sources->curr;
        if (node != NULL
            && (!excludeExternal
                || (excludeExternal && !sources->externallyManaged))) {
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

double Audio::getLatency()
{
    ring_buffer_size_t queuedAudio
        = PaUtil_GetRingBufferReadAvailable(m_rBufferOut);
    double nidiumAudioLatency
        = (((queuedAudio) * (1.0 / m_OutputParameters->m_SampleRate))
           / m_OutputParameters->m_Channels);
    double paLatency = 0;

    if (m_PlaybackStartTime != 0) {
        int64_t now = av_gettime();
        double playbackDuration
            = ((1.0 / m_OutputParameters->m_SampleRate)
               * static_cast<double>(m_PlaybackConsumedFrame));

        paLatency = playbackDuration
                    - static_cast<double>(now - m_PlaybackStartTime) / 1000000;
    }

    SPAM(("[Audio] latency=%f nidium=%f pa=%f\n", paLatency + nidiumAudioLatency,
          nidiumAudioLatency, paLatency));

    return paLatency + nidiumAudioLatency;
}

AudioNode *Audio::addSource(AudioNode *source, bool externallyManaged)
{
    AudioSources *sources = new AudioSources();

    this->lockSources();
    this->lockQueue();

    sources->curr              = source;
    sources->externallyManaged = externallyManaged;
    sources->prev              = NULL;
    sources->next              = m_Sources;

    if (m_Sources != NULL) {
        m_Sources->prev = sources;
    }

    m_Sources = sources;
    m_SourcesCount++;

    this->unlockSources();
    this->unlockQueue();

    return sources->curr;
}

void Audio::removeSource(AudioSource *source)
{
    this->lockSources();
    this->lockQueue();

    AudioSources *sources = m_Sources;

    while (sources != NULL) {
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

AudioNode *Audio::createNode(Audio::Node node, int input, int output)
{
    try {
        switch (node) {
            case SOURCE: {
                AudioSource *source = new AudioSource(output, this, false);
                this->addSource(source, false);
                return source;
            } break;
            case CUSTOM_SOURCE: {
                AudioSourceCustom *source = new AudioSourceCustom(output, this);
                this->addSource(source, true);
                return source;
            } break;
            case GAIN:
                return new AudioNodeGain(input, output, this);
                break;
            case CUSTOM:
                return new AudioNodeCustom(input, output, this);
                break;
            case REVERB:
                return new AudioNodeReverb(input, output, this);
                break;
            case DELAY:
                return new AudioNodeDelay(input, output, this);
                break;
            case STEREO_ENHANCER:
                return new AudioNodeStereoEnhancer(input, output, this);
                break;
            case TARGET:
                if (m_Output == NULL) {
                    m_Output = new AudioNodeTarget(input, output, this);
                    return m_Output;
                }
                return NULL;
                break;
            default:
                break;
        }
    } catch (AudioNodeException *e) {
        throw;
    }

    return NULL;
}

bool Audio::connect(NodeLink *input, NodeLink *output)
{
    return output->m_Node->queue(input, output);
}

bool Audio::disconnect(NodeLink *input, NodeLink *output)
{
    return output->m_Node->unqueue(input, output);
}

void Audio::setVolume(float volume)
{
    m_volume = volume;
}

float Audio::getVolume()
{
    return m_volume;
}

void Audio::wakeup(bool block)
{
    if (block) {
        m_SharedMsgFlush = true;
    }

    m_QueueFreeLock  = true;

    NIDIUM_PTHREAD_SIGNAL(&m_QueueHaveData);
    NIDIUM_PTHREAD_SIGNAL(&m_QueueHaveSpace);

    if (block) {
        NIDIUM_PTHREAD_WAIT(&m_QueueMessagesFlushed);
    }

    m_QueueFreeLock = false;
}

void Audio::shutdown()
{
    m_ThreadShutdown = true;

    NIDIUM_PTHREAD_SIGNAL(&m_QueueHaveData)
    NIDIUM_PTHREAD_SIGNAL(&m_QueueHaveSpace)
    NIDIUM_PTHREAD_SIGNAL(&m_QueueNeedData)

    pthread_join(m_ThreadQueue, NULL);
    pthread_join(m_ThreadDecode, NULL);
}

void Audio::lockQueue()
{
    m_QueueFreeLock = true;
    pthread_mutex_lock(&m_RecurseLock);
    m_QueueFreeLock = false;
}

void Audio::unlockQueue()
{
    pthread_mutex_unlock(&m_RecurseLock);
}

void Audio::sourceNeedWork(void *ptr)
{
    Audio *thiz            = static_cast<Audio *>(ptr);
    thiz->m_SourceNeedWork = true;
    NIDIUM_PTHREAD_SIGNAL(&thiz->m_QueueNeedData);
}

void Audio::lockSources()
{
    pthread_mutex_lock(&m_SourcesLock);
}

void Audio::unlockSources()
{
    pthread_mutex_unlock(&m_SourcesLock);
}

void Audio::postMessage(AudioMessageCallback cbk, void *custom, bool block)
{
    m_SharedMsg->postMessage(
        static_cast<void *>(new CallbackMessage(cbk, custom)),
        NIDIUM_AUDIO_CALLBACK);

    if (block) {
        this->wakeup();
    }
}
void Audio::postMessage(AudioMessageCallback cbk, void *custom)
{
    this->postMessage(cbk, custom, false);
}

bool Audio::canWriteFrame()
{
    return PaUtil_GetRingBufferWriteAvailable(m_rBufferOut)
           >= m_OutputParameters->m_FramesPerBuffer
                  * m_OutputParameters->m_Channels;
}

Audio::~Audio()
{
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
