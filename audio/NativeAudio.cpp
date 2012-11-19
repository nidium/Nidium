#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>
#include "pa_ringbuffer.h"
#include "NativeAudio.h"
#include "NativeAudioNode.h"
#include <NativeSharedMessages.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

// TODO : use Singleton
// if multiple NativeAudio is asked with different bufferSize/channels/sampleRate
// should reset all parameters and buffers to fit last asked audio context
NativeAudio::NativeAudio(int bufferSize, int channels, int sampleRate)
    :inputStream(NULL), outputStream(NULL), threadShutdown(false), tracks(NULL), tracksCount(0)
{
    pthread_cond_init(&this->bufferNotEmpty, NULL);
    pthread_mutex_init(&this->decodeLock, NULL);

    pthread_cond_init(&this->queueHaveData, NULL);
    pthread_cond_init(&this->queueHaveSpace, NULL);
    pthread_mutex_init(&this->queueLock, NULL);

    pthread_mutex_init(&this->shutdownLock, NULL);

    this->sharedMsg = new NativeSharedMessages();
    this->rBufferOut = new PaUtilRingBuffer();

    // Save output parameters
    this->outputParameters = new NativeAudioParameters(bufferSize, channels, NativeAudio::FLOAT32, sampleRate);

    // Init libav
	av_register_all();

    // Init buffers
    if (!(this->rBufferOutData = (float *)calloc(sizeof(float), NATIVE_AVDECODE_BUFFER_SAMPLES * NativeAudio::FLOAT32))) {
        printf("Failed to init ouput ringbuffer\n");
        return;
    }

    if (!(this->nullBuffer = (float *)calloc(sizeof(float), bufferSize/channels))) {
        printf("Failed to init nullBuffer\n");
        return;
    }

    if (!(this->cbkBuffer= (float *)calloc(sizeof(float), bufferSize))) {
        printf("Failed to init cbkBUffer\n");
        return;
    }

    if (0 < PaUtil_InitializeRingBuffer(this->rBufferOut, 
            (NativeAudio::FLOAT32),
            NATIVE_AVDECODE_BUFFER_SAMPLES,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        return;
    }

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

void *NativeAudio::queueThread(void *args) {
    NativeAudio *audio = (NativeAudio *)args;
    bool wrote;
    int cause;

    wrote = false;
    cause = 0;

    while (true && !audio->threadShutdown) {
        NativeSharedMessages::Message msg;

        if (cause == 0) {
            SPAM(("Waiting for more data\n"));
            pthread_cond_wait(&audio->queueHaveData, &audio->queueLock);
        } else {
            SPAM(("Waiting for more space\n"));
            pthread_cond_wait(&audio->queueHaveSpace, &audio->queueLock);
        }

        // Process input message
        while (audio->sharedMsg->readMessage(&msg)) {
            switch (msg.event()) {
                case NATIVE_AUDIO_NODE_SET : {
                    NativeAudioNode::Message *nodeMsg =  static_cast<NativeAudioNode::Message *>(msg.dataPtr());
                    memcpy(nodeMsg->dest, nodeMsg->source, nodeMsg->size);
                    delete nodeMsg;
                }
                    break;
                case NATIVE_AUDIO_SHUTDOWN :
                    SPAM(("shutdown received"));
                    audio->threadShutdown = true;
                    break;
            }
        }

        if (audio->output != NULL) {
            for (;;) {
                if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) >= audio->outputParameters->framesPerBuffer * audio->outputParameters->channels) {
                    SPAM(("Write avail %lu\n", PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut)));
                    if (!audio->output->recurseGetData()) {
                        SPAM(("break cause of false\n"));
                        break;
                    } else {
                        wrote = true;
                    }

                    audio->output->nodeProcessed = 0;
                    cause = 0;

                    SPAM(("----------------------\n"));

                    if (wrote) {
                        //SPAM(("output data is at %p and %p (node = %p)\n",audio->output->inQueue[0]->frame, audio->output->inQueue[1]->frame, audio->output->inQueue[0]->node));
                        /*
                        for (int i = 0; i < 256; i++) {
                            SPAM(("write data = %f/%f\n", audio->output->frames[0][i], audio->output->frames[1][i]));
                        }
                        */


                        /*
                        for (int i = 0; i < audio->outputParameters->framesPerBuffer; i++) {
                            SPAM(("frame data %f/%f\n", audio->output->frames[0][i], audio->output->frames[1][i]));
                        }
                        */

                        for (int i = 0; i < audio->output->inCount; i++) {
                            if (audio->output->frames[i] != NULL) {
                                PaUtil_WriteRingBuffer(audio->rBufferOut, audio->output->frames[i], audio->outputParameters->framesPerBuffer);
                            } else {
                                PaUtil_WriteRingBuffer(audio->rBufferOut, audio->nullBuffer, audio->outputParameters->framesPerBuffer);
                            }
                        }
                        pthread_cond_signal(&audio->bufferNotEmpty);
                    }
                } else {
                    SPAM(("no more space to write\n"));
                    cause = 1;
                    break;
                }
            }
            SPAM(("Finished FX queue\n"));
        } 
        /*
        pthread_mutex_lock(&audio->shutdownLock);
        if(audio->threadShutdown) {
            finished = true;
        }
        pthread_mutex_unlock(&audio->shutdownLock);
        */
    }

    SPAM(("Exiting"));

    return NULL;
}

void *NativeAudio::decodeThread(void *args) {
    NativeAudio *audio = (NativeAudio *)args;

    while (true && !audio->threadShutdown) 
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

        if (audio->tracksCount > 0 /*&& haveEnought == audio->tracksCount*/) {
            if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) > 0) {
                pthread_cond_signal(&audio->queueHaveData);
            } 
        }

        // Wait for work to do
        pthread_cond_wait(&audio->bufferNotEmpty, &audio->decodeLock);
    }

    SPAM(("Exiting"));
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

        // Data was read from ring buffer
        // need to process more data
        pthread_cond_signal(&this->queueHaveSpace);
    } else {
        SPAM(("-----------------------------------NO DATA\n"));
        for (unsigned int i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0;
            *out++ = 0;
        }
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

NativeAudioTrack *NativeAudio::addTrack(int out) {
    NativeAudioTracks *tracks = (NativeAudioTracks *)malloc(sizeof(NativeAudioTracks));

    tracks->curr = new NativeAudioTrack(out, this);

    tracks->prev = NULL;
    tracks->next = this->tracks;

    if (this->tracks != NULL) {
        this->tracks->prev = tracks;
    }

    this->tracks = tracks;

    this->tracksCount++;

    return tracks->curr;
}

NativeAudioNode *NativeAudio::createNode(NativeAudio::Node node, int input, int output) 
{
    switch (node) {
        case SOURCE:
                return this->addTrack(output);
            break;
        case GAIN:
                return new NativeAudioNodeGain(input, output, this);
            break;
        case TARGET:
                if (this->openOutput() == 0) {
                    this->output = new NativeAudioNodeTarget(input, output, this);
                    return this->output;
                } else {
                    return NULL;
                }
            break;
        default :
            break;
    }

    return NULL;
}

void NativeAudio::connect(NodeLink *input, NodeLink *output) 
{
    output->node->queue(input, output);
}

void NativeAudio::disconnect(NodeLink *input, NodeLink *output) 
{
    output->node->unqueue(input, output);
}

void NativeAudio::shutdown()
{
    pthread_mutex_lock(&this->shutdownLock);
    this->threadShutdown = true;
    pthread_mutex_unlock(&this->shutdownLock);

    pthread_cond_signal(&this->queueHaveData);
    pthread_cond_signal(&this->bufferNotEmpty);
}

NativeAudio::~NativeAudio() {
    if (this->outputStream != NULL) {
        Pa_StopStream(this->outputStream); 
    }

    if (this->inputStream != NULL) {
        Pa_StopStream(this->inputStream);
    }

    Pa_Terminate(); 

    /*
    while (tracks != NULL) 
    {
        if (tracks->curr != NULL) {
            delete tracks->curr;

            tracks = tracks->next;
        }
    }

    this->tracks = NULL;

    // TODO : recursivily delete all nodes
    delete this->output;
    */
    free(this->nullBuffer);
    free(this->cbkBuffer);

}

