#ifndef nativeav_h__
#define nativeav_h__

#include <stdint.h>
#include <string.h>

#define NATIVE_AVIO_BUFFER_SIZE         32768 

class NativeAVBufferReader
{
    public:
        NativeAVBufferReader(uint8_t *buffer, unsigned long bufferSize);

        static int read(void *opaque, uint8_t *buffer, int size);
        static int64_t seek(void *opaque, int64_t offset, int whence);

        ~NativeAVBufferReader() {};
    private:
        uint8_t *buffer;
        unsigned long bufferSize;
        unsigned long pos;
};

struct NativeAudioParameters {
    int bufferSize, channels, sampleFmt, sampleRate, framesPerBuffer;
    NativeAudioParameters(int bufferSize, int channels, 
                          int sampleFmt, int sampleRate)
        : bufferSize(bufferSize), channels(channels), 
          sampleFmt(sampleFmt), sampleRate(sampleRate), 
          framesPerBuffer(bufferSize/(sampleFmt * channels)) 
    {
    }
};


enum {
    NATIVE_AUDIO_NODE_SET,
    NATIVE_AUDIO_NODE_CALLBACK,
    NATIVE_AUDIO_TRACK_CALLBACK,
    NATIVE_AUDIO_SHUTDOWN
};

enum {
    ERR_FAILED_OPEN,
    ERR_NO_INFORMATION,
    ERR_NO_AUDIO,
    ERR_NO_VIDEO,
    ERR_NO_CODEC,
    ERR_OOM,
    ERR_NO_RESAMPLING_CONVERTER,
    ERR_NO_VIDEO_CONVERTER,
    ERR_INIT_VIDEO_AUDIO,
    ERR_DECODING,
    ERR_INTERNAL
};

#endif
