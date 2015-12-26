#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "<NativeAudio.h"
#include "NativeAudioNode.h"

// Compile with : 
// g++ audio.cpp -D__STDC_CONSTANT_MACROS  -I../audio/ -L../audio/ -lnativeaudio -lavformat -lavcodec -lavutil -lz -lportaudio -lzita-resampler -o audio -I ../../portaudio/src/common/ ../../portaudio/src/common/pa_ringbuffer.o ../../portaudio/src/common/pa_converters.o ../../portaudio/src/common/pa_dither.o && ./audio
// g++ audio.cpp -D__STDC_CONSTANT_MACROS  -I../audio/ -L../audio/ -L../third-party/ffmpeg/libavformat/ -L../third-party/ffmpeg/libavcodec/ -L../third-party/ffmpeg/libavutil/ -lnativeaudio -lavformat -lavcodec -lavutil -lz -lportaudio -lzita-resampler -o audio -I ../third-party/portaudio/src/common/ ../third-party/portaudio/src/common/pa_ringbuffer.o ../third-party/portaudio/src/common/pa_converters.o ../third-party/portaudio/src/common/pa_dither.o ../src/NativeSharedMessages.o && ./audio

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

static void trackcb(const struct TrackEvent *ev) {
    printf("track cbk %d\n", ev->ev);
}
static void nodecb(const struct NodeEvent *ev) {
    //printf("node cbk\n");
    /*
    for (int i = 0; i < ev->size; i++) {
        ev->data[0][i] *= 0;
        ev->data[1][i] *= 0;
    }
    */
}

static void cbk(NativeAudioNode *node, void *custom) {
    printf("callback called\n");
}

int main(int argc, char *argv[]) {
    pthread_t threadIO;
    pthread_t threadDecode;
    pthread_t threadQueue;
    NativeAudio *audio;

    audio = new NativeAudio(256, 2, 48000);

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

    load("/tmp/foo.wav", buffer1, bufferSize);
    //load("/tmp/foo.mp3", buffer2, bufferSize);


    NativeAudioTrack *track1 = (NativeAudioTrack *)audio->createNode(NativeAudio::SOURCE, 0, 2);
    //NativeAudioNodeGain *gain = (NativeAudioNodeGain *)audio->createNode(NativeAudio::GAIN, 2, 2);
//    NativeAudioNodeGain *gain2 = (NativeAudioNodeGain *)audio->createNode(NativeAudio::GAIN, 2, 2);
    //NativeAudioTrack *track2 = (NativeAudioTrack *)audio->createNode(NativeAudio::SOURCE, 0, 2);
    /*
    NativeAudioNodeMixer *mixer = (NativeAudioNodeMixer*)audio->createNode("mixer", 4, 2);
    NativeAudioNodeGain *gain2 = (NativeAudioNodeGain *)audio->createNode("gain", 2, 2);
    */
    //NativeAudioNodeCustom *custom = (NativeAudioNodeCustom *)audio->createNode(NativeAudio::CUSTOM, 2, 2);
    NativeAudioNodeTarget *target= (NativeAudioNodeTarget *)audio->createNode(NativeAudio::TARGET, 2, 0);

    //custom->setCallback(nodecb, NULL);
    //track1->setCallback(trackcb, NULL);

    //gain->gain = 1;

    //audio->connect(gain->output[0], gain->input[0]);


    //audio->connect(track2->output[0], target->input[0]);
    //audio->connect(track2->output[1], target->input[1]);


    //double gainValue = 1;
    //double gainValue2 = 0.80;
    //gain->set("gain", DOUBLE, (void *)&gainValue, sizeof(double));
    //gain2->set("gain", DOUBLE, (void *)&gainValue2, sizeof(double));

    audio->connect(track1->output[0], target->m_Input[0]);
    audio->connect(track1->output[1], target->m_Input[1]);

//    audio->connect(custom->output[0], target->input[0]);
    //audio->connect(custom->output[1], target->input[1]);


 //   audio->connect(gain->output[0], target->input[0]);
    //audio->connect(gain->output[1], target->input[1]);

    /*
    audio->connect(gain->output[0], gain2->input[0]);
    audio->connect(gain->output[1], gain2->input[1]);

    audio->connect(gain2->output[0], gain->input[0]);
    audio->connect(gain2->output[1], gain->input[1]);
    */

    //gain->setCallback(cbk, (void*)NULL);
    track1->open(buffer1, bufferSize);
    //track2->open(buffer2, bufferSize);

    track1->play();
    //track2->play();

    //NativeAudioTrack *track2 = audio->addTrack();
    //track2->open(buffer2, bufferSize);
    //track2->play();
    /*

    printf("sleep\n");
    sleep(5);
    audio->disconnect(track1->output[0], target->input[0]);
    sleep(2);
    audio->disconnect(track1->output[1], target->input[1]);
    printf("disconnected\n");
    //audio->shutdown();
    */

    pthread_join(threadDecode, NULL);
    pthread_join(threadDecode, NULL);
    printf("post join\n");

    free(buffer1);
    free(buffer2);
}
