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

    if (!(this->rBufferOutData = (float *)calloc(sizeof(float), NATIVE_AVDECODE_BUFFER_SAMPLES * NativeAudio::FLOAT32))) {
        printf("Failed to init ouput ringbuffer\n");
        return;
    }

    if (!(this->nullBuffer = (float *)calloc(sizeof(float), bufferSize/channels))) {
        printf("Failed to init nullBuffer\n");
        return;
    }

    if (!(this->cbkBuffer = (float *)calloc(sizeof(float), bufferSize))) {
        printf("Failed to init cbkBUffer\n");
        return;
    }

    if (0 > PaUtil_InitializeRingBuffer(this->rBufferOut, 
            (NativeAudio::FLOAT32),
            NATIVE_AVDECODE_BUFFER_SAMPLES,
            this->rBufferOutData)) {
        fprintf(stderr, "Failed to init output ringbuffer\n");
        return;
    }

    pthread_create(&this->threadDecode, NULL, NativeAudio::decodeThread, this);
    pthread_create(&this->threadQueue, NULL, NativeAudio::queueThread, this);
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

    while (true) {
        NativeSharedMessages::Message msg;

        // Process input message
        // TODO : Limit message reading
        while (audio->sharedMsg->readMessage(&msg)) {
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
            int sourceFailed = 0;
            pthread_mutex_lock(&audio->recurseLock);
            for (;;) {
/*
                if (PaUtil_GetRingBufferReadAvailable(audio->rBufferOut) >= (audio->outputParameters->framesPerBuffer * audio->outputParameters->channels)*32) {
                    printf("Output have enought data\n");
                    break;
                }
*/
                if (PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut) >= audio->outputParameters->framesPerBuffer * audio->outputParameters->channels) {
                    //SPAM(("Write avail %lu\n", PaUtil_GetRingBufferWriteAvailable(audio->rBufferOut)));

                    printf("sourcefailed=%d\n", sourceFailed);
                    if (!audio->output->recurseGetData(&sourceFailed)) {
                        //SPAM(("break cause of false\n"));
                        //audio->haveData = false;
                        break;
                    } else {
                        wrote = true;
                    }

                    audio->output->nodeProcessed = 0;
                    cause = 0;

                    //SPAM(("----------------------\n"));

                    if (wrote) {
                        //SPAM(("output data is at %p and %p \n",audio->output->frames[0], audio->output->frames[1]));
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
                                //SPAM(("Writing data\n"));
                                PaUtil_WriteRingBuffer(audio->rBufferOut, audio->output->frames[i], audio->outputParameters->framesPerBuffer);
                            } else {
                                //SPAM(("Writing nullBuff cause frames is null\n"));
                                PaUtil_WriteRingBuffer(audio->rBufferOut, audio->nullBuffer, audio->outputParameters->framesPerBuffer);
                            }
                        }
                    }
                } else {
                    //SPAM(("no more space to write\n"));
                    cause = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&audio->recurseLock);
            //SPAM(("Finished FX queue\n"));

            if (audio->tracksCount > 0 && wrote) {
                pthread_cond_signal(&audio->bufferNotEmpty);
            }
            //audio->notEmpty = true;
        } 

        if (!audio->threadShutdown) {
            if (cause == 0) {
                //if (!audio->haveData) {
                    //SPAM(("Waiting for more data\n"));
                    pthread_cond_wait(&audio->queueHaveData, &audio->queueLock);
                //}
            } else {
                //SPAM(("Waiting for more space\n"));
                pthread_cond_wait(&audio->queueHaveSpace, &audio->queueLock);
            }
        }
        //SPAM(("Queue thead is now working\n"));

        /*
        pthread_mutex_lock(&audio->shutdownLock);
        if(audio->threadShutdown) {
            finished = true;
        }
        pthread_mutex_unlock(&audio->shutdownLock);
        */
    }

    //SPAM(("Exiting"));

    return NULL;
}

// TODO : Locking/unlocking for tracks list
void *NativeAudio::decodeThread(void *args) {
    NativeAudio *audio = static_cast<NativeAudio *>(args);

    NativeAudioTracks *tracks;
    NativeAudioTrack *track;
    int haveEnought, tracksCount;

    while (true) 
    {
        tracks = audio->tracks;
        haveEnought = 0;
        tracksCount = audio->tracksCount;

        // Go through all the tracks that need data to be decoded
        printf("tracks %p\n", tracks);
        while (tracks != NULL) 
        {
            haveEnought = 0;

            printf("tracks->curr %p\n", tracks->curr);
            if (tracks->curr != NULL) {
                printf("yep I should  buffer\n");

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
        //if (!audio->notEmpty) {
            pthread_cond_wait(&audio->bufferNotEmpty, &audio->decodeLock);
        //}
        SPAM(("Buffer not empty received\n"));

        NATIVE_AUDIO_CHECK_EXIT_THREAD
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

        // Data was read from ring buffer
        // need to process more data
        pthread_cond_signal(&this->queueHaveSpace);
        // TODO : The queueHaveSpace signal is sent even if all tracks are stopped
        // thus the thread is working for nothing. Need to fix this
    } else {
        SPAM(("-----------------------------------NO DATA\n"));
        for (unsigned int i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0.0f;
            *out++ = 0.0f;
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

double NativeAudio::getLatency() {
    ring_buffer_size_t queuedAudio = PaUtil_GetRingBufferReadAvailable(this->rBufferOut);
    NativeAudioParameters *params = this->outputParameters;
    return ((double)queuedAudio * NativeAudio::FLOAT32 * params->channels) / (params->sampleRate * NativeAudio::FLOAT32 * params->channels);
}

NativeAudioTrack *NativeAudio::addTrack(int out) {
    NativeAudioTracks *tracks = new NativeAudioTracks();

    tracks->curr = new NativeAudioTrack(out, this, false);

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

