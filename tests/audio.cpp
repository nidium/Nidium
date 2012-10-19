#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NativeAudio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// Compile with : 
// g++ audio.cpp -D__STDC_CONSTANT_MACROS  -I../audio/ -L../audio/ -lnativeaudio -lavformat -lavcodec -lavutil -lz -lportaudio -lzita-resampler -o audio -I ../../portaudio/src/common/ ../../portaudio/src/common/pa_ringbuffer.o ../../portaudio/src/common/pa_converters.o ../../portaudio/src/common/pa_dither.o && ./audio
static void *thread_io(void *arg) {
    NativeAudio *audio = (NativeAudio *)arg;
    printf("Hello thread io\n");

    // Using while / usleep to demonstrate purpose
    // in Native, event system will be used
    while (true) {
        audio->bufferData();
        usleep(100);
    }
}

void load(const char *file, uint8_t *buffer, int bufferSize) {
    FILE *f = fopen(file, "r");
    int read = 0;
    int pos = 0;
    do {
        if (pos + 2048 >= bufferSize) break;
        read = fread(buffer + pos, 1, 2048, f);
        pos += read;
        uint8_t *cursor = buffer;
    } while (read > 0);
}

int main(int argc, char *argv[]) {
    pthread_t threadIO;
    pthread_t threadDecode;
    pthread_t threadQueue;
    NativeAudio *audio;

    audio = new NativeAudio(2048, 2, 44100);

    // 1) Create thread for I/O
//    pthread_create(&threadIO, NULL, thread_io, audio);

    // 2) Create thread for decoding
    pthread_create(&threadDecode, NULL, NativeAudio::decodeThread, audio);

    // 3) Create thread for JS processing
    pthread_create(&threadQueue, NULL, NativeAudio::queueThread, audio);

    // 2) Open ouput
    /*
    int ret = audio->openOutput();
    if (ret == 0) {
        printf("Audio ouput is ok\n");
    } else {
        printf("Failed to open audio ouput : %d\n", ret);
    }
    */

    // 4) Play a file 
    int bufferSize = sizeof(uint8_t)*4*1024*1024;
    uint8_t *buffer1, *buffer2;

    buffer1 = (uint8_t *)malloc(bufferSize);
    buffer2 = (uint8_t *)malloc(bufferSize);

    load("/tmp/test.mp3", buffer1, bufferSize);
    //load("/tmp/foo.mp3", buffer2, bufferSize);


    NativeAudioTrack *track1 = (NativeAudioTrack *)audio->createNode(NativeAudio::SOURCE, 0, 2);
    //NativeAudioNodeGain *gain = (NativeAudioNodeGain *)audio->createNode(NativeAudio::GAIN, 2, 2);
    /*
    NativeAudioTrack *track2 = (NativeAudioTrack *)audio->createNode("source", 0, 2);
    NativeAudioNodeMixer *mixer = (NativeAudioNodeMixer*)audio->createNode("mixer", 4, 2);
    NativeAudioNodeGain *gain2 = (NativeAudioNodeGain *)audio->createNode("gain", 2, 2);
    */
    NativeAudioNodeTarget *target= (NativeAudioNodeTarget *)audio->createNode(NativeAudio::TARGET, 2, 0);

    //gain->gain = 1;

    //audio->connect(gain->output[0], gain->input[0]);

    audio->connect(track1->output[0], target->input[0]);
//    audio->connect(track1->output[1], target->input[1]);


    //audio->connect(gain->output[0], target->input[0]);
    //audio->connect(gain->output[1], target->input[1]);
    /*
    gain2->gain = 0.2;

    audio->connect(track1->channel(0), gain->channel(0));
    audio->connect(track1->channel(1), gain->channel(1));

    audio->connect(gain->channel(0), mixer->channel(0));
    audio->connect(gain->channel(1), mixer->channel(1));

    //

    audio->connect(track2->channel(0), gain2->channel(0));
    audio->connect(track2->channel(1), gain2->channel(1));

    audio->connect(gain2->channel(0), mixer->channel(2));
    audio->connect(gain2->channel(1), mixer->channel(3));

    //

    audio->connect(mixer->channel(0), target->channel(0));
    audio->connect(mixer->channel(1), target->channel(1));
    */

    track1->open(buffer1, bufferSize);
    //track2->open(buffer2, bufferSize);

    track1->play();
    //track2->play();

    //NativeAudioTrack *track2 = audio->addTrack();
    //track2->open(buffer2, bufferSize);
    //track2->play();

    pthread_join(threadIO, NULL);
    pthread_join(threadDecode, NULL);

    free(buffer1);
    free(buffer2);
}
