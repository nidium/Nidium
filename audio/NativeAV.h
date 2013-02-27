#ifndef nativeav_h__
#define nativeav_h__

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define NATIVE_AVIO_BUFFER_SIZE         32768 

#define SOURCE_EVENT_PLAY      0x01
#define SOURCE_EVENT_PAUSE     0x02
#define SOURCE_EVENT_STOP      0x03
#define SOURCE_EVENT_EOF       0x04
#define SOURCE_EVENT_ERROR     0x05
#define SOURCE_EVENT_BUFFERING 0x06
#define SOURCE_EVENT_BUFFERED  0x07

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
    NATIVE_AUDIO_SOURCE_CALLBACK,
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

class NativeAVSource;
// Used for event (play, pause, stop, error, buffered...)
typedef void (*NativeAVSourceEventCallback)(const struct NativeAVSourceEvent*ev); 

struct NativeAVSourceEvent {
    int ev;
    int *value;
    NativeAVSource *source;
    void *custom;
    bool fromThread;
    NativeAVSourceEvent(NativeAVSource *source, int ev, int value, void *custom, bool fromThread)
        : ev(ev), source(source), custom(custom), fromThread(fromThread) 
    {
        this->value = new int;
        memcpy(this->value, (void *)&value, sizeof(int));
    };
};

class NativeAVSource 
{
    public :
        NativeAVSource() : eventCbk(NULL), eventCbkCustom(NULL) 
        {
        }
        void eventCallback(NativeAVSourceEventCallback cbk, void *custom) 
        {
            this->eventCbk = cbk;
            this->eventCbkCustom = custom;
        }

        void sendEvent(int ev, int value, bool fromThread) {
            if (this->eventCbk != NULL) {
                NativeAVSourceEvent *event = new NativeAVSourceEvent(this, ev, value, this->eventCbkCustom, fromThread);
                this->eventCbk(event);
            }
        }
        
        NativeAVSourceEventCallback eventCbk;
        void *eventCbkCustom;

};

#endif
