#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NativeAudio.h>

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// Compile with : 
// g++ audio.cpp -D__STDC_CONSTANT_MACROS -I../audio/ -L../audio/ -lnativeaudio -lpthread -lavformat -lavcodec -lavutil -lz -lportaudio -o audio
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
    NativeAudio audio;

    // 1) Create thread for I/O
    pthread_create(&threadIO, NULL, thread_io, &audio);

    // 2) Create thread for decoding
    pthread_create(&threadDecode, NULL, NativeAudio::decodeThread, &audio);

    // 2) Open ouput
    int ret = audio.openOutput(2048, 2, NativeAudio::FLOAT32, 44100);
    if (ret == 0) {
        printf("Audio ouput is ok\n");
    } else {
        printf("Failed to open audio ouput : %d\n", ret);
    }

    // 4) Play a file 
    int bufferSize = sizeof(uint8_t)*4*1024*1024;
    uint8_t *buffer1, *buffer2;

    buffer1 = (uint8_t *)malloc(bufferSize);
    buffer2 = (uint8_t *)malloc(bufferSize);

    load("/tmp/test2.mp3", buffer1, bufferSize);
    load("/tmp/test.mp3", buffer2, bufferSize);


    NativeAudioTrack *track1 = audio.addTrack();
    NativeAudioTrack *track2 = audio.addTrack();

    track1->open(buffer1, bufferSize);
    track2->open(buffer2, bufferSize);

    track1->play();
    track2->play();

    pthread_join(threadIO, NULL);
    pthread_join(threadDecode, NULL);

    free(buffer1);
    free(buffer2);
}
