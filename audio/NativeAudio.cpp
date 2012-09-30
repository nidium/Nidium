#include <stdio.h>
#include <stdlib.h>
#include "NativeAudio.h"

NativeAudio::NativeAudio()
    :tracks(NULL), tracksCount(0)
{
    pthread_cond_init(&this->bufferNotEmpty, NULL);
    pthread_mutex_init(&this->decodeLock, NULL);

    // Init libav
	av_register_all();
}

void NativeAudio::bufferData() {
    NativeAudioTracks *tracks = this->tracks;
    NativeAudioTrack *track;

    bool wrote = false;

    // XXX : Should I lock the track list while working on it?

    // Buffer data for each tracks
    while (tracks != NULL) 
    {
        if (tracks->curr != NULL && tracks->curr->opened) {
            int i, n;
            track = tracks->curr;

            n = PaUtil_GetRingBufferWriteAvailable(&track->rBufferIn);

            if (n > 0) {
                AVPacket pkt;

                av_init_packet(&pkt);

                for (i = 0; i < n; i++) 
                {
                    if (av_read_frame(track->container, &pkt) < 0) {
                        break;
                    } else {
                        av_dup_packet(&pkt);
                        PaUtil_WriteRingBuffer(&track->rBufferIn, &pkt, 1);

                        wrote = true;
                    }
                }
            }
        }
        tracks = tracks->next;
    }

    if (wrote == true) {
        // Send signal to decoding thread
        pthread_cond_signal(&this->bufferNotEmpty);
    }

    tracks = this->tracks;
}

void *NativeAudio::decodeThread(void *args) {
    NativeAudio *audio = (NativeAudio *)args;

    while (true) {
        NativeAudioTracks *tracks = audio->tracks;
        NativeAudioTrack *track;

        // Go through all the tracks that need data to be decoded
        while (tracks != NULL) 
        {
            if (tracks->curr != NULL && tracks->curr->opened) {
                ring_buffer_size_t len;
	            AVCodecContext *avctx;
                AVPacket pkt;

                track = tracks->curr;

                avctx = track->container->streams[track->audioStream]->codec;
                av_init_packet(&pkt);


                // Loop as long as there is data to read and write
                for (int got_frame = 0;;) {
                    // If there is data to read
                    if (0 < PaUtil_GetRingBufferReadAvailable(&track->rBufferIn)|| !track->frameConsumed) {
                        // No last frame, get a new one
                        if (track->frameConsumed) {
                            PaUtil_ReadRingBuffer(&track->rBufferIn, (void *)&pkt, 1);
                        }

                        // Decode packet 
                        // TODO : One packet might contains more than one frame
                        // need to check if len == pkt.size otherwise loop
                        len = avcodec_decode_audio4(avctx, track->tmpFrame, &got_frame, &pkt);

                        if (len < 0) {
                            fprintf(stderr, "Error while decoding\n");
                            // TODO : Better error handling (events?)
                            exit(1);
                        } else {
                            //printf("Read len = %lu\n", len);
                        }

                        av_free_packet(&pkt);

                        // Is there enought space in ouput to write the frame?
                        if (PaUtil_GetRingBufferWriteAvailable(&track->rBufferOut) >= track->tmpFrame->nb_samples) {
                            // Write data to ring buffer
                            if (0 < PaUtil_WriteRingBuffer(&track->rBufferOut, track->tmpFrame->data[0], track->tmpFrame->nb_samples)) {
                                track->frameConsumed = true;
                            } else {
                                track->frameConsumed = false;
                            }
                        } else {
                            // No more space to write, exit;
                            track->frameConsumed = false;
                            break;
                        }
                    } else {
                        // No more to read, exit
                        break;
                    }
                }
            }
            tracks = tracks->next;
        }

        // Wait for more work to do
        pthread_cond_wait(&audio->bufferNotEmpty, &audio->decodeLock);
    }

}

int NativeAudio::openOutput(int bufferSize, int channel, SampleFormat sampleFmt, int sampleRate) {
    const PaDeviceInfo *infos;
    PaDeviceIndex device;
    PaError error;
    PaStreamParameters paOutputParameters;

    // Save ouput parameters
    this->outputParameters.bufferSize = bufferSize;
    this->outputParameters.channel = channel;
    this->outputParameters.sampleFmt = sampleFmt;
    this->outputParameters.sampleRate = sampleRate;

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
    paOutputParameters.channelCount = channel; 
    paOutputParameters.suggestedLatency = infos->defaultHighOutputLatency; 
    paOutputParameters.hostApiSpecificStreamInfo = 0; /* no api specific data */
    paOutputParameters.sampleFormat = paInt16;

    // Try to open the ouput
    error = Pa_OpenStream(&this->outputStream,  
            0,  // No input
            &paOutputParameters,
            sampleRate,  
            (bufferSize / (channel * sampleFmt)),
            paNoFlag,  
            &NativeAudio::paOutputCallback,  
            (void *)this); 

    if (error)
    {
        printf("[NativeAudio] Error opening output, error code = %i\n", error);
        Pa_Terminate();
        return error;
    }
    printf("[NativeAudio] ouput opened with latency to %f\n", infos->defaultHighOutputLatency);

    Pa_StartStream(this->outputStream);
    return 0;
}

// The instance callback, where we have access to every method/variable of NativeAudio
int NativeAudio::paOutputCallbackMethod(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags)
{
    int16_t *out = (int16_t*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    // TODO : Use dynamic sample format instead of int16_t 

    bool dataRead;
    int totalRead;

    dataRead = false;
    totalRead = 0;

    for (i = 0; i < framesPerBuffer; i++)
    {
        NativeAudioTracks *tracks = this->tracks;
        double left = 0, right = 0;

        // Loop all track and mix them 
        while (tracks != NULL) 
        {
            if (tracks->curr != NULL && tracks->curr->playing) {
                int16_t tmp[2] = {0, 0};
                ring_buffer_size_t read;

                read = PaUtil_ReadRingBuffer(&tracks->curr->rBufferOut, &tmp, 1);
                totalRead += read;
                dataRead = read > 0 ? true : false;
                       
                // TODO : Clip sound

                left += tmp[0];
                right += tmp[1];

            }

            tracks = tracks->next;
        }

        if (this->tracksCount > 0) {
            left /= this->tracksCount;
            right /= this->tracksCount;
        }

        // Send sound to output
        *out++ = (int16_t)left;
        *out++ = (int16_t)right;
    }

    // Data was read from ring buffer
    // need to decode more data
    if (dataRead) {
        if (totalRead < 512) {
            printf("FAILED TO READ ENOUGHT DATA %d\n", totalRead);
        }
        pthread_cond_signal(&this->bufferNotEmpty);
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

NativeAudioTrack *NativeAudio::addTrack() {
    NativeAudioTracks *tracks = (NativeAudioTracks *)malloc(sizeof(NativeAudioTracks));

    tracks->curr = new NativeAudioTrack(&this->outputParameters);

    tracks->prev = NULL;
    tracks->next = this->tracks;

    if (this->tracks != NULL) {
        this->tracks->prev = tracks;
    }

    this->tracks = tracks;

    this->tracksCount++;

    return tracks->curr;
}

NativeAudio::~NativeAudio() {
    printf("NativeAudio destructor\n");
    NativeAudioTracks *tracks = this->tracks;

    if (this->outputStream != NULL) {
        Pa_StopStream(this->outputStream); 
    }

    if (this->inputStream != NULL) {
        Pa_StopStream(this->inputStream);
    }

    Pa_Terminate(); 

    while (tracks != NULL) 
    {
        if (tracks->curr != NULL) {
            delete tracks->curr;

            tracks = tracks->next;
        }
    }

}

NativeAudioTrack::NativeAudioTrack(NativeAudio::OutputParameters *outputParameters) 
    : outputParameters(outputParameters), container(NULL), frameConsumed(true), audioStream(-1),
      opened(false), playing(false), paused(false)
{

    int framePerBuffer = this->outputParameters->bufferSize / (this->outputParameters->sampleFmt * this->outputParameters->channel);

    // TODO : Throw exception instead of return

    // Alloc buffers memory
    if (!(this->avioBuffer = (unsigned char *)av_malloc(NATIVE_AV_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE))) {// XXX : Pick better buffer size?
        return;
    }

    if (!(this->rBufferInData = malloc((sizeof(AVPacket) * framePerBuffer) * 8))) {
        return;
    }
    if (!(this->rBufferOutData = malloc((this->outputParameters->sampleFmt * this->outputParameters->channel) * framePerBuffer * 4))) {
        return;
    }

    if (!(this->tmpFrame = avcodec_alloc_frame())) {
        return;
    }

    // I/O packet list ring buffer
    // XXX : Is it better to use a linked list + mutex instead?
    if (0 < PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)&this->rBufferIn, 
            sizeof(AVPacket), 
            framePerBuffer * 8,
            this->rBufferInData)) {
        printf("[NativeAudio] Failed to init input ringbuffer");
        return;
    }

    // Decoding buffer
    if (0 < PaUtil_InitializeRingBuffer((PaUtilRingBuffer*)&this->rBufferOut, 
            (this->outputParameters->sampleFmt * this->outputParameters->channel),
            framePerBuffer * 4,
            this->rBufferOutData)) {
        printf("[NativeAudio] Failed to init input ringbuffer");
        return;
    }
}


int NativeAudioTrack::open(void *buffer, int size) 
{
    unsigned int i;
    AVCodecContext *avctx;
    AVCodec *codec;

    // If a previous file has been opened, close it
    if (this->container != NULL) {
        this->close(true);
    }

    // Setup libav context
    this->br = new BufferReader((uint8_t *)buffer, size);
    this->container = avformat_alloc_context();
    this->container->pb = avio_alloc_context(this->avioBuffer, NATIVE_AV_BUFFER_SIZE, 0, this->br, this->br->read, NULL, NULL);

	// Open input 
	int ret = avformat_open_input(&this->container, "dummyFile", NULL, NULL);
	if (ret != 0) {
		char error[1024];
		av_strerror(ret, error, 1024);
		fprintf(stderr, "Couldn't open file : %s\n", error);
        return -2;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(container, NULL) < 0) {
		fprintf(stderr, "Couldn't find stream information\n");
        return -3;
	}

	// Dump information about file onto standard error
	av_dump_format(container, 0, "Memory input", 0);

	// Find first audio stream
	for (i = 0; i < this->container->nb_streams; i++) {
		if (this->container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && this->audioStream < 0) {
			this->audioStream = i;
		}
	}

	if(this->audioStream == -1) {
		fprintf(stderr, "Couldn't find audio stream\n");
		return -4;
	}

	// Find the apropriate codec and open it
	avctx = container->streams[audioStream]->codec;
	codec = avcodec_find_decoder(avctx->codec_id);

	if (!avcodec_open2(avctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not find open the needed codec\n");
		return -5;
	}

    this->opened = true;

    return 0;
}

void NativeAudioTrack::close(bool reset) {
    if (reset) {
        this->playing = false;

        PaUtil_FlushRingBuffer(&this->rBufferIn);
        PaUtil_FlushRingBuffer(&this->rBufferOut);

        memset(this->avioBuffer, 0, NATIVE_AV_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
    } else {
        free(this->rBufferInData);
        free(this->rBufferOutData);

        av_free(this->avioBuffer);
    }

    if (!this->frameConsumed) {
        av_free(this->tmpFrame);
    }

    av_free(this->container->pb);
    avformat_free_context(this->container);

    delete this->br;
}

void NativeAudioTrack::play() 
{
    this->playing = true;
}

NativeAudioTrack::~NativeAudioTrack() {
    this->close(false);
}
