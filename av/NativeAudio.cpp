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
    : net(n), sourcesCount(0), readFlag(false), output(NULL), 
      inputStream(NULL), outputStream(NULL), 
      rBufferOutData(NULL), cbkBuffer(NULL), volume(1),
      m_FlushMessages(false), threadShutdown(false), sources(NULL)
{
    pthread_cond_init(&this->bufferNotEmpty, NULL);
    pthread_cond_init(&this->queueHaveData, NULL);
    pthread_cond_init(&this->queueHaveSpace, NULL);

    pthread_mutex_init(&this->decodeLock, NULL);
    pthread_mutex_init(&this->queueLock, NULL);
    pthread_mutex_init(&this->shutdownLock, NULL);

    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&this->sourcesLock, &mta);
    pthread_mutex_init(&this->recurseLock, &mta);

    this->sharedMsg = new NativeSharedMessages();

    av_register_all();

    this->outputParameters = new NativeAudioParameters(bufferSize, channels, NativeAudio::FLOAT32, sampleRate);

    this->rBufferOut = new PaUtilRingBuffer();

    if (!(this->rBufferOutData = (float *)calloc(NativeAudio::FLOAT32, NATIVE_AVDECODE_BUFFER_SAMPLES * NativeAudio::FLOAT32))) {
        printf("Failed to init ouput ringbuffer\n");
        throw;
    }

    if (!(this->nullBuffer = (float *)calloc(NativeAudio::FLOAT32, bufferSize/channels))) {
        printf("Failed to init nullBuffer\n");
        free(this->rBufferOutData);
        throw;
    }

    if (!(this->cbkBuffer = (float *)calloc(NativeAudio::FLOAT32, bufferSize))) {
        printf("Failed to init cbkBUffer\n");
        free(this->rBufferOutData);
        free(this->cbkBuffer);
        throw;
    }

    if (0 > PaUtil_InitializeRingBuffer(this->rBufferOut, 
            (NativeAudio::FLOAT32),
            NATIVE_AVDECODE_BUFFER_SAMPLES,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        free(this->rBufferOutData);
        free(this->cbkBuffer);
        throw;
    }

    pthread_create(&this->threadDecode, NULL, NativeAudio::decodeThread, this);
    pthread_create(&this->threadQueue, NULL, NativeAudio::queueThread, this);
}

void *NativeAudio::queueThread(void *args) {
    NativeAudio *audio = (NativeAudio *)args;
    bool wrote;
    int cause = 0;

    while (true) {
        audio->readMessages(audio->m_FlushMessages);

        if (audio->output != NULL) {
            int sourceFailed;
            wrote = false;
            cause = 0;

            pthread_mutex_lock(&audio->recurseLock);

            if (audio->threadShutdown) {
                pthread_mutex_unlock(&audio->recurseLock);
                break;
            }

            for (;;) {
                sourceFailed = 0;
/*
                if (PaUtil_GetRingBufferReadAvailable(audio->rBufferOut) >= (audio->outputParameters->framesPerBuffer * audio->outputParameters->channels)*32) {
                    printf("Output have enought data\n");
                    break;
                }
*/
                if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) >= audio->outputParameters->framesPerBuffer * audio->outputParameters->channels) {
                    audio->processQueue();

                    if (!audio->output || !audio->output->processed) {
                        break;
                    } 

                    wrote = true;
                    audio->output->processed = false;
                    cause = 0;

                    // Copy output frame data to output ring buffer
                    for (int i = 0; i < audio->output->inCount; i++) {
                        if (audio->volume != 1) {
                            int frameSize = audio->outputParameters->bufferSize/audio->outputParameters->channels;
                            for (int j = 0; j < frameSize; j++) {
                                audio->output->frames[i][j] *= audio->volume;
                            }
                        }
                        PaUtil_WriteRingBuffer(audio->rBufferOut, audio->output->frames[i], audio->outputParameters->framesPerBuffer);
                    }
                } else {
                    cause = 1;
                    break;
                }
            }

            pthread_mutex_unlock(&audio->recurseLock);

            //if (audio->haveSourceActive(true)) {
                pthread_cond_signal(&audio->bufferNotEmpty);
            //}
        } 

        if (audio->threadShutdown) break;

        if (cause == 0) {
            SPAM(("Waiting for more data\n"));
            pthread_cond_wait(&audio->queueHaveData, &audio->queueLock);
        } else {
            //do {
                SPAM(("Waiting for more space\n"));
                pthread_cond_wait(&audio->queueHaveSpace, &audio->queueLock);
            //} while (!audio->haveSourceActive(false));

        }

        if (audio->threadShutdown) break;

        SPAM(("Queue thead is now working\n"));
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
    NativeSharedMessages::Message msg;
    int nread = 0;
    while (((!flush && ++nread < MAX_MSG_IN_ROW) || flush) && this->sharedMsg->readMessage(&msg)) {
        switch (msg.event()) {
            case NATIVE_AUDIO_NODE_CALLBACK : {
                NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg.dataPtr());
                cbkMsg->cbk(cbkMsg->node, cbkMsg->custom);
                if (cbkMsg->node != NULL) {
                    cbkMsg->node->unref();
                }
                delete cbkMsg;
            }
            break;
            case NATIVE_AUDIO_NODE_SET : {
                NativeAudioNode::Message *nodeMsg =  static_cast<NativeAudioNode::Message *>(msg.dataPtr());
                if (nodeMsg->arg->ptr == NULL) {
                    nodeMsg->arg->cbk(nodeMsg->node, nodeMsg->arg->id, nodeMsg->val, nodeMsg->size);
                } else {
                    memcpy(nodeMsg->arg->ptr, nodeMsg->val, nodeMsg->size);
                }

                if (nodeMsg->node != NULL) {
                    nodeMsg->node->unref();
                }

                delete nodeMsg;
            }
            break;
            case NATIVE_AUDIO_CALLBACK :
                NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg.dataPtr());
                cbkMsg->cbk(NULL, cbkMsg->custom);
                if (cbkMsg->node != NULL) {
                    cbkMsg->node->unref();
                }
                delete cbkMsg;
            break;
        }
    }

    if (flush && m_FlushMessages) {
        m_FlushMessages = false;
    }
#undef MAX_MSG_IN_ROW
}


void NativeAudio::processQueue()
{
    SPAM(("process queue\n"));
    NativeAudioSources *sources = this->sources;

    while (sources!= NULL) 
    {
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
                source = static_cast<NativeAudioSource *>(sources->curr);

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
            if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) > 0) {
                SPAM(("Have data\n"));
                pthread_cond_signal(&audio->queueHaveData);
                //audio->haveData = true;
            }  else {
                SPAM(("dont Have data %lu\n",PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut)));
            }
        }

        NATIVE_AUDIO_CHECK_EXIT_THREAD

        // Wait for work to do unless some source need to wakeup
        if (!audio->readFlag) {
            SPAM(("Waitting for bufferNotEmpty readFlag=%d\n", audio->readFlag));
            pthread_cond_wait(&audio->bufferNotEmpty, &audio->decodeLock);
            SPAM(("Buffer not empty received\n"));
        } else {
            SPAM(("decodeThread not sleeping cause it need wakup\n"));
        }

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
    paOutputParameters.suggestedLatency = infos->defaultHighOutputLatency; 
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

// The instance callback, where we have access to every method/variable of NativeAudio
int NativeAudio::paOutputCallbackMethod(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags)
{
    float *out = (float *)outputBuffer;

    (void) timeInfo; 
    (void) statusFlags;
    (void) inputBuffer;

    if (PaUtil_GetRingBufferReadAvailable(this->rBufferOut) >= (ring_buffer_size_t) (framesPerBuffer * this->outputParameters->channels)) {
        //SPAM(("------------------------------------data avail\n"));
        //SPAM(("SIZE avail : %lu\n", PaUtil_GetRingBufferReadAvailable(this->rBufferOut)));
        PaUtil_ReadRingBuffer(this->rBufferOut, this->cbkBuffer, framesPerBuffer * this->outputParameters->channels);
        for (unsigned int i = 0; i < framesPerBuffer; i++)
        {
            for (int j = 0; j < this->outputParameters->channels; j++) {
                *out++ = this->cbkBuffer[i + (j * framesPerBuffer)];
                /*
                if (j%2 == 0) {
                SPAM(("play data %f", this->cbkBuffer[i + (j * framesPerBuffer)]));
                } else {
                SPAM(("/%f\n", this->cbkBuffer[i + (j * framesPerBuffer)]));
                }
                */
            }
        }
    } else {
        //SPAM(("-----------------------------------NO DATA\n"));
        for (unsigned int i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0.0f;
            *out++ = 0.0f;
        }
    }

    pthread_cond_signal(&this->queueHaveSpace);

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
    return true;
#if 0
    pthread_mutex_lock(&this->sourcesLock);
    NativeAudioSources *sources = this->sources;
    while (sources != NULL) 
    {
        NativeAudioSource *t = sources->curr;
        nf (t != NULL && (!excludeExternal || (excludeExternal && t->externallyManaged))) {
            if  (t->opened && t->playing) {
                pthread_mutex_unlock(&this->sourcesLock);
                return true;
            }
        }
        sources = sources->next;
    }
    pthread_mutex_unlock(&this->sourcesLock);
    return false;
#endif
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
    NativeAudioParameters *params = this->outputParameters;
    return ((double)queuedAudio * NativeAudio::FLOAT32 * params->channels) / (params->sampleRate * NativeAudio::FLOAT32 * params->channels);
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

void NativeAudio::wakeup() 
{
    this->m_FlushMessages = true;

    pthread_cond_signal(&this->queueHaveData);
    pthread_cond_signal(&this->queueHaveSpace);
}

void NativeAudio::shutdown()
{
    this->threadShutdown = true;

    pthread_cond_signal(&this->queueHaveSpace);
    pthread_cond_signal(&this->queueHaveData);
    pthread_cond_signal(&this->bufferNotEmpty);

    pthread_join(this->threadQueue, NULL);
    pthread_join(this->threadDecode, NULL);
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

    free(this->nullBuffer);
    free(this->cbkBuffer);
    free(this->rBufferOutData);

    delete this->outputParameters;
    delete this->rBufferOut;
    delete this->sharedMsg;
}

