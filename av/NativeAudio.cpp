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
    : net(n), output(NULL), tracksCount(0), 
      inputStream(NULL), outputStream(NULL), 
      threadShutdown(false), tracks(NULL)
{
    pthread_cond_init(&this->bufferNotEmpty, NULL);
    pthread_cond_init(&this->queueHaveData, NULL);
    pthread_cond_init(&this->queueHaveSpace, NULL);


    pthread_mutex_init(&this->decodeLock, NULL);
    pthread_mutex_init(&this->queueLock, NULL);
    pthread_mutex_init(&this->shutdownLock, NULL);
    pthread_mutex_init(&this->recurseLock, NULL);

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

#if 0
void NativeAudio::bufferData() {
    NativeAudioTracks *tracks = this->tracks;
    bool wrote = false;

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
#endif

void *NativeAudio::queueThread(void *args) {
#define MAX_MSG_IN_ROW 20
    NativeAudio *audio = (NativeAudio *)args;
    bool wrote;
    int nread;
    int cause = 0;

    while (true) {
        NativeSharedMessages::Message msg;
        nread = 0;

        while (++nread < MAX_MSG_IN_ROW && audio->sharedMsg->readMessage(&msg)) {
            switch (msg.event()) {
                case NATIVE_AUDIO_NODE_CALLBACK : {
                    NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg.dataPtr());
                    cbkMsg->cbk(cbkMsg->node, cbkMsg->custom);
                    delete cbkMsg;
                }
                break;
                case NATIVE_AUDIO_NODE_SET : {
                    NativeAudioNode::Message *nodeMsg =  static_cast<NativeAudioNode::Message *>(msg.dataPtr());
                    memcpy(nodeMsg->dest, nodeMsg->source, nodeMsg->size);
                    delete nodeMsg;
                }
                break;
                case NATIVE_AUDIO_SHUTDOWN :
                    NativeAudioNode::CallbackMessage *cbkMsg = static_cast<NativeAudioNode::CallbackMessage*>(msg.dataPtr());
                    cbkMsg->cbk(NULL, cbkMsg->custom);
                    delete cbkMsg;
                    return NULL;
                break;
            }
        }

        if (audio->output != NULL) {
            int sourceFailed;
            wrote = false;
            cause = 0;

            pthread_mutex_lock(&audio->recurseLock);

            for (;;) {
                sourceFailed = 0;
/*
                if (PaUtil_GetRingBufferReadAvailable(audio->rBufferOut) >= (audio->outputParameters->framesPerBuffer * audio->outputParameters->channels)*32) {
                    printf("Output have enought data\n");
                    break;
                }
*/
                if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) >= audio->outputParameters->framesPerBuffer * audio->outputParameters->channels) {

                    if (!audio->output->recurseGetData(&sourceFailed)) {
                        break;
                    } else {
                        wrote = true;
                    }

                    audio->output->nodeProcessed = 0;
                    cause = 0;

                    // Copy output's frame data to output ring buffer
                    for (int i = 0; i < audio->output->inCount; i++) {
                        if (audio->output->frames[i] != NULL) {
                            PaUtil_WriteRingBuffer(audio->rBufferOut, audio->output->frames[i], audio->outputParameters->framesPerBuffer);
                        } else {
                            PaUtil_WriteRingBuffer(audio->rBufferOut, audio->nullBuffer, audio->outputParameters->framesPerBuffer);
                        }
                    }
                } else {
                    cause = 1;
                    break;
                }

            }

            pthread_mutex_unlock(&audio->recurseLock);

            if (audio->haveSourceActive(true)) {
                pthread_cond_signal(&audio->bufferNotEmpty);
            }
        } 

        if (!audio->threadShutdown) {
            if (cause == 0) {
                SPAM(("Waiting for more data\n"));
                pthread_cond_wait(&audio->queueHaveData, &audio->queueLock);
            } else {
                SPAM(("Waiting for more space\n"));
                pthread_cond_wait(&audio->queueHaveSpace, &audio->queueLock);
            }
        }
        SPAM(("Queue thead is now working\n"));
    }

    SPAM(("Exiting queueThread\n"));

    return NULL;
#undef MAX_MSG_IN_ROW
}

// TODO : Locking/unlocking for tracks list
void *NativeAudio::decodeThread(void *args) {
    NativeAudio *audio = static_cast<NativeAudio *>(args);

    NativeAudioTracks *tracks;
    NativeAudioTrack *track;
    int haveEnought, tracksCount;

    for(;;)
    {
        tracks = audio->tracks;
        haveEnought = 0;
        tracksCount = audio->tracksCount;

        // Go through all the tracks that need data to be decoded
        while (tracks != NULL) 
        {
            haveEnought = 0;

            if (tracks->curr != NULL && !tracks->curr->externallyManaged) {
                track = tracks->curr;

                // Loop as long as there is data to read and write
                //while(track->avail() <= audio->outputParameters->framesPerBuffer * 2)
                for (;;)
                {
                    SPAM(("Buffering\n"));
                    if (!track->work()) {
                        // No data to process, exit. 
                        break;
                    }  
                }

                if (track->avail() >= audio->outputParameters->framesPerBuffer * 2) {
                    haveEnought++;
                    //audio->notEmpty = false;
                } 

                if (!track->opened || !track->playing) {
                    tracksCount--;
                }
            }
            tracks = tracks->next;
        }
        SPAM(("haveEnought %d / tracksCount %d\n", haveEnought, tracksCount));

        // FIXME : find out why when playing multiple song, 
        // the commented expression bellow fail to work
        if (audio->tracksCount > 0 /*&& haveEnought == tracksCount*/) {
            if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) > 0) {
                SPAM(("Have data\n"));
                pthread_cond_signal(&audio->queueHaveData);
                //audio->haveData = true;
            }  else {
                SPAM(("dont Have data %lu\n",PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut)));
            }
        }

        NATIVE_AUDIO_CHECK_EXIT_THREAD

        // Wait for work to do
        SPAM(("Waitting for bufferNotEmpty\n"));
        pthread_cond_wait(&audio->bufferNotEmpty, &audio->decodeLock);
        SPAM(("Buffer not empty received\n"));

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
        SPAM(("------------------------------------data avail\n"));
        SPAM(("SIZE avail : %lu\n", PaUtil_GetRingBufferReadAvailable(this->rBufferOut)));
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
        SPAM(("-----------------------------------NO DATA\n"));
        for (unsigned int i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0.0f;
            *out++ = 0.0f;
        }
    }

    // Wakeup queueThread if needed
    if (this->haveSourceActive(false)) {
        pthread_cond_signal(&this->queueHaveSpace);
    }

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

bool NativeAudio::haveSourceActive(bool excludeExternal) {
    return true;
    NativeAudioTracks *tracks = this->tracks;
    while (tracks != NULL) 
    {
        NativeAudioTrack *t = tracks->curr;
        if (t != NULL && (!excludeExternal || (excludeExternal && t->externallyManaged))) {
            if  (t->opened && t->playing) {
                return true;
            }
        }
        tracks = tracks->next;
    }
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
    NativeAudioParameters *params = this->outputParameters;
    return ((double)queuedAudio * NativeAudio::FLOAT32 * params->channels) / (params->sampleRate * NativeAudio::FLOAT32 * params->channels);
}

NativeAudioTrack *NativeAudio::addTrack(int out, bool external) {
    NativeAudioTracks *tracks = new NativeAudioTracks();

    tracks->curr = new NativeAudioTrack(out, this, external);

    tracks->prev = NULL;
    tracks->next = this->tracks;

    if (this->tracks != NULL) {
        this->tracks->prev = tracks;
    }

    this->tracks = tracks;

    this->tracksCount++;

    return tracks->curr;
}

void NativeAudio::removeTrack(NativeAudioTrack *track) 
{
    NativeAudioTracks *tracks = this->tracks;
    while (tracks != NULL) 
    {
        if (tracks->curr != NULL && tracks->curr == track) {

            if (tracks->prev != NULL) {
                tracks->prev->next = tracks->next;
            } else {
                this->tracks = tracks->next;
            }
            if (tracks->next != NULL) {
                tracks->next->next = tracks->prev;
            }
            delete tracks;
            return;
        }
        tracks = tracks->next;
    }
}

NativeAudioNode *NativeAudio::createNode(NativeAudio::Node node, int input, int output) 
{
    switch (node) {
        case SOURCE:
                return this->addTrack(output, false);
            break;
        case GAIN:
                return new NativeAudioNodeGain(input, output, this);
            break;
        case CUSTOM:
                return new NativeAudioNodeCustom(input, output, this);
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

void NativeAudio::wakeup() 
{
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

