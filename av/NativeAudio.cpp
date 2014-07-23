#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>
#include "pa_ringbuffer.h"
#include "NativeAudio.h"
#include "NativeAudioNode.h"
#include <NativeSharedMessages.h>
#include "Coro.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

NativeAudio::NativeAudio(ape_global *n, int bufferSize, int channels, int sampleRate)
    : net(n), sourcesCount(0), output(NULL), inputStream(NULL),
      outputStream(NULL), rBufferOutData(NULL), cbkBuffer(NULL), volume(1),
      m_SourceNeedWork(false), m_SharedMsgFlush(false), threadShutdown(false), sources(NULL), m_MainCtx(NULL)
{
    NATIVE_PTHREAD_VAR_INIT(&this->queueHaveData);
    NATIVE_PTHREAD_VAR_INIT(&this->queueHaveSpace);
    NATIVE_PTHREAD_VAR_INIT(&this->queueNeedData);
    NATIVE_PTHREAD_VAR_INIT(&this->queueMessagesFlushed);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&this->sourcesLock, &mta);
    pthread_mutex_init(&this->recurseLock, &mta);

    this->sharedMsg = new NativeSharedMessages();

    av_register_all();

    this->outputParameters = new NativeAudioParameters(bufferSize, channels, NativeAudio::FLOAT32, sampleRate);

    this->rBufferOut = new PaUtilRingBuffer();

    if (!(this->rBufferOutData = (float *)calloc(bufferSize * channels, NativeAudio::FLOAT32))) {
        printf("Failed to init ouput ringbuffer\n");
        throw;
    }

    if (!(this->cbkBuffer = (float *)calloc(bufferSize, channels * NativeAudio::FLOAT32))) {
        printf("Failed to init cbkBUffer\n");
        free(this->rBufferOutData);
        free(this->cbkBuffer);
        throw;
    }

    if (0 > PaUtil_InitializeRingBuffer(this->rBufferOut, 
            NativeAudio::FLOAT32,
            bufferSize * channels,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        free(this->rBufferOutData);
        free(this->cbkBuffer);
        throw;
    }

    pthread_create(&this->threadDecode, NULL, NativeAudio::decodeThread, this);
    pthread_create(&this->threadQueue, NULL, NativeAudio::queueThread, this);
}

void *NativeAudio::queueThread(void *args) 
{
    NativeAudio *audio = (NativeAudio *)args;
    bool wrote;

    while (true) {
        bool msgFlush = audio->m_SharedMsgFlush;

        audio->readMessages(msgFlush);

        if (msgFlush && audio->m_SharedMsgFlush) {
            audio->m_SharedMsgFlush = false;
            NATIVE_PTHREAD_SIGNAL(&audio->queueMessagesFlushed);
        }

        // Using a trylock because we don't want to wait for another thread to
        // finish working with the recurseLock for being able to read shared messages.
        //
        // Example : For shutdown, we need to suspend the queue thread (using
        // NativeAudio::lockQueue()) but we also need the ability to read
        // shared messages for shuting down various nodes.
        int lockErr = pthread_mutex_trylock(&audio->recurseLock);

        if (lockErr == 0 && audio->output != NULL && msgFlush != true) {
            wrote = false;

            if (audio->threadShutdown) {
                pthread_mutex_unlock(&audio->recurseLock);
                break;
            }

            for (;;) {
                if (!audio->canWriteFrame()) {
                    break;
                }

                audio->processQueue();

                if (!audio->output->processed) {
                    break;
                } 

                wrote = true;
                audio->output->processed = false;

                // Copy output node frame data to output ring buffer
                // XXX : Find a more efficient way to copy data to output right buffer
                for (int i = 0; i < audio->outputParameters->framesPerBuffer; i++) {
                    for (int j = 0; j < audio->outputParameters->channels; j++) {
                        audio->output->frames[j][i] *= audio->volume;
                        PaUtil_WriteRingBuffer(audio->rBufferOut, &audio->output->frames[j][i], 1);
                    }
                }
            }

            if (wrote) {
                SPAM(("Sending queueNeedData\n"));
                NATIVE_PTHREAD_SIGNAL(&audio->queueNeedData);
            }
        } 

        if (lockErr == 0) {
            pthread_mutex_unlock(&audio->recurseLock);
        }

        if (audio->threadShutdown) break;

        if (!audio->canWriteFrame()) {
            SPAM(("Waiting for more space\n"));
            NATIVE_PTHREAD_WAIT(&audio->queueHaveSpace)
        } else {
            SPAM(("Waiting for more data\n"));
            NATIVE_PTHREAD_WAIT(&audio->queueHaveData)
        }

        if (audio->threadShutdown) break;

        SPAM(("Queue thead is now working shutdown=%d\n", audio->threadShutdown));
    }

    audio->readMessages(true);

    SPAM(("Exiting queueThread\n"));

    return NULL;
}

void NativeAudio::readMessages() 
{
    this->readMessages(false);
}

void NativeAudio::readMessages(bool flush)
{
#define MAX_MSG_IN_ROW 1024
    NativeSharedMessages::Message *msg;
    int nread = 0;
    while (((!flush && ++nread < MAX_MSG_IN_ROW) || flush) && (msg = this->sharedMsg->readMessage())) {
        switch (msg->event()) {
            case NATIVE_AUDIO_NODE_CALLBACK : {
                NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg->dataPtr());
                cbkMsg->cbk(cbkMsg->node, cbkMsg->custom);
                delete cbkMsg;
            }
            break;
            case NATIVE_AUDIO_NODE_SET : {
                NativeAudioNode::Message *nodeMsg =  static_cast<NativeAudioNode::Message *>(msg->dataPtr());
                if (nodeMsg->arg->ptr == NULL) {
                    nodeMsg->arg->cbk(nodeMsg->node, nodeMsg->arg->id, nodeMsg->val, nodeMsg->size);
                } else {
                    memcpy(nodeMsg->arg->ptr, nodeMsg->val, nodeMsg->size);
                }

                delete nodeMsg;
            }
            break;
            case NATIVE_AUDIO_CALLBACK :
                NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg->dataPtr());
                cbkMsg->cbk(NULL, cbkMsg->custom);
                delete cbkMsg;
            break;
        }
        delete msg;
    }
#undef MAX_MSG_IN_ROW
}


void NativeAudio::processQueue()
{
    SPAM(("process queue\n"));
    NativeAudioSources *sources = this->sources;

    while (sources!= NULL) 
    {
        SPAM(("curr=%p connected=%d\n", sources->curr, sources->curr->isConnected));
        if (sources->curr != NULL && sources->curr->isConnected) {
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

    for(;;)
    {
        // Go through all the sources that need data to be decoded
        pthread_mutex_lock(&audio->sourcesLock);

        sources = audio->sources;
        haveEnought = 0;
        sourcesCount = audio->sourcesCount;

        while (sources != NULL) 
        {
            haveEnought = 0;

            if (sources->curr != NULL && !sources->externallyManaged) {
                source = static_cast<NativeAudioSource*>(sources->curr);

                // Loop as long as there is data to read and write
                while (source->work()) {}

                if (source->avail() >= audio->outputParameters->framesPerBuffer * 2) {
                    haveEnought++;
                    //audio->notEmpty = false;
                } 

                if (!source->opened || !source->playing) {
                    sourcesCount--;
                }
            }
            sources = sources->next;
        }
        pthread_mutex_unlock(&audio->sourcesLock);
        SPAM(("haveEnought %d / sourcesCount %d\n", haveEnought, sourcesCount));

        // FIXME : find out why when playing multiple song, 
        // the commented expression bellow fail to work
        if (audio->sourcesCount > 0 /*&& haveEnought == sourcesCount*/) {
            if (audio->canWriteFrame()) {
                SPAM(("Have data %lu\n", PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut)));
                NATIVE_PTHREAD_SIGNAL(&audio->queueHaveData);
                //audio->haveData = true;
            }  else {
                SPAM(("dont Have data %lu\n",PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut)));
            }
        }

        NATIVE_AUDIO_CHECK_EXIT_THREAD

        // Wait for work to do unless some source need to wakeup
        if (!audio->m_SourceNeedWork) {
            SPAM(("Waitting for queueNeedData m_SourceNeedWork=%d\n", audio->m_SourceNeedWork));
            NATIVE_PTHREAD_WAIT(&audio->queueNeedData);
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

int NativeAudio::openOutput() {
    const PaDeviceInfo *infos;
    PaDeviceIndex device;
    PaError error;
    PaStreamParameters paOutputParameters;

    // Start portaudio
    Pa_Initialize();
    
    // TODO : Device should be defined by user
    device = Pa_GetDefaultOutputDevice();
    if (device == paNoDevice) {
        return 1;
    }

    infos = Pa_GetDeviceInfo(device);

    // Set output parameters for PortAudio
    paOutputParameters.device = device; 
    paOutputParameters.channelCount = this->outputParameters->channels; 
    paOutputParameters.suggestedLatency = infos->defaultLowInputLatency;
    paOutputParameters.hostApiSpecificStreamInfo = 0; /* no api specific data */
    paOutputParameters.sampleFormat = paFloat32;

    // Try to open the output
    error = Pa_OpenStream(&this->outputStream,  
            0,
            &paOutputParameters,
            this->outputParameters->sampleRate,  
            this->outputParameters->framesPerBuffer,  
            paNoFlag,  
            &NativeAudio::paOutputCallback,  
            (void *)this); 

    if (error) {
        printf("[NativeAudio] Error opening output, error code = %i\n", error);
        Pa_Terminate();
        return error;
    }

    printf("[NativeAudio] output opened with latency to %f\n", infos->defaultHighOutputLatency);

    Pa_StartStream(this->outputStream);

    return 0;
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
    int channels = this->outputParameters->channels;
    ring_buffer_size_t avail = PaUtil_GetRingBufferReadAvailable(this->rBufferOut) / channels;
    avail = framesPerBuffer > avail ? avail : framesPerBuffer;
    int left = framesPerBuffer - avail;


    //PaUtil_ReadRingBuffer(this->rBufferOut, out, avail * channels);
    PaUtil_ReadRingBuffer(this->rBufferOut, out, avail * channels);

#if 0
    SPAM(("frames per buffer=%ld avail=%ld processing=%ld left=%d\n", framesPerBuffer, PaUtil_GetRingBufferReadAvailable(this->rBufferOut), avail, left));
    if (left > 0) {
        SPAM(("WARNING DROPING %d\n", left));
    }
#endif

    for (unsigned int i = 0; i < left * channels; i++)
    {
        *out++ = 0.0f;
    }

    NATIVE_PTHREAD_SIGNAL(&this->queueHaveSpace);

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

bool NativeAudio::haveSourceActive(bool excludeExternal) 
{
    pthread_mutex_lock(&this->sourcesLock);
    NativeAudioSources *sources = this->sources;
    while (sources != NULL) 
    {
        NativeAudioNode *node = sources->curr;
        if (node != NULL && (!excludeExternal || (excludeExternal && !sources->externallyManaged))) {
            if (node->isActive()) {
                pthread_mutex_unlock(&this->sourcesLock);
                return true;
            }
        }
        sources = sources->next;
    }
    pthread_mutex_unlock(&this->sourcesLock);
    return false;
}

int NativeAudio::getSampleSize(int sampleFormat) {
    switch (sampleFormat) {
        case AV_SAMPLE_FMT_U8 :
            return UINT8;
        case AV_SAMPLE_FMT_NONE:
        case AV_SAMPLE_FMT_S16 :
            return INT16;
        case AV_SAMPLE_FMT_S32 :
            return INT32;
        case AV_SAMPLE_FMT_FLT :
            return FLOAT32;
        case AV_SAMPLE_FMT_DBL :
            return FLOAT32;
        default :
            return INT16; // This will mostly fail
    }
}

double NativeAudio::getLatency() {
    ring_buffer_size_t queuedAudio = PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
    return (queuedAudio * ((double)1/this->outputParameters->sampleRate)) / this->outputParameters->channels;
}

NativeAudioNode *NativeAudio::addSource(NativeAudioNode *source, bool externallyManaged) 
{
    NativeAudioSources *sources = new NativeAudioSources();

    pthread_mutex_lock(&this->sourcesLock);
    pthread_mutex_lock(&this->recurseLock);

    sources->curr = source;
    sources->externallyManaged = externallyManaged;
    sources->prev = NULL;
    sources->next = this->sources;

    if (this->sources != NULL) {
        this->sources->prev = sources;
    }

    this->sources = sources;

    this->sourcesCount++;

    pthread_mutex_unlock(&this->recurseLock);
    pthread_mutex_unlock(&this->sourcesLock);

    return sources->curr;
}

void NativeAudio::removeSource(NativeAudioSource *source) 
{
    pthread_mutex_lock(&this->sourcesLock);
    pthread_mutex_lock(&this->recurseLock);

    NativeAudioSources *sources = this->sources;

    while (sources != NULL) 
    {
        if (sources->curr != NULL && sources->curr == source) {

            if (sources->prev != NULL) {
                sources->prev->next = sources->next;
            } else {
                this->sources = sources->next;
            }
            if (sources->next != NULL) {
                sources->next->prev = sources->prev;
            }

            delete sources;

            pthread_mutex_unlock(&this->recurseLock);
            pthread_mutex_unlock(&this->sourcesLock);

            return;
        }
        sources = sources->next;
    }

    pthread_mutex_unlock(&this->recurseLock);
    pthread_mutex_unlock(&this->sourcesLock);
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
                    if (this->output == NULL) {
                        this->output = new NativeAudioNodeTarget(input, output, this);
                        return this->output;
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
    this->volume = volume;
}

float NativeAudio::getVolume()
{
    return this->volume;
}

void NativeAudio::wakeup() 
{
    this->m_SharedMsgFlush = true;

    NATIVE_PTHREAD_SIGNAL(&this->queueHaveData);
    NATIVE_PTHREAD_SIGNAL(&this->queueHaveSpace);

    NATIVE_PTHREAD_WAIT(&this->queueMessagesFlushed);
}

void NativeAudio::shutdown()
{
    this->threadShutdown = true;

    NATIVE_PTHREAD_SIGNAL(&this->queueHaveData)
    NATIVE_PTHREAD_SIGNAL(&this->queueHaveSpace)
    NATIVE_PTHREAD_SIGNAL(&this->queueNeedData)

    pthread_join(this->threadQueue, NULL);
    pthread_join(this->threadDecode, NULL);
}

void NativeAudio::lockQueue()
{
    pthread_mutex_lock(&this->recurseLock);
}

void NativeAudio::unlockQueue()
{
    pthread_mutex_unlock(&this->recurseLock);
}

void NativeAudio::sourceNeedWork(void *ptr)
{
    NativeAudio *thiz = static_cast<NativeAudio*>(ptr);
    thiz->m_SourceNeedWork = true;
    NATIVE_PTHREAD_SIGNAL(&thiz->queueNeedData);
}

void NativeAudio::lockSources()
{
    pthread_mutex_lock(&this->sourcesLock);
}

void NativeAudio::unlockSources()
{
    pthread_mutex_unlock(&this->sourcesLock);
}

bool NativeAudio::canWriteFrame()
{
    return PaUtil_GetRingBufferWriteAvailable(this->rBufferOut) >=
        this->outputParameters->framesPerBuffer * this->outputParameters->channels;
}

NativeAudio::~NativeAudio() {
    if (this->threadShutdown == false) {
        this->shutdown();
    }

    if (this->outputStream != NULL) {
        Pa_StopStream(this->outputStream); 
        Pa_CloseStream(this->outputStream);
    }

    if (this->inputStream != NULL) {
        Pa_StopStream(this->inputStream);
        Pa_CloseStream(this->inputStream);
    }

    Pa_Terminate(); 

    free(this->cbkBuffer);
    free(this->rBufferOutData);

    delete this->outputParameters;
    delete this->rBufferOut;
    delete this->sharedMsg;
}

