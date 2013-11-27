#ifndef nativeav_h__
#define nativeav_h__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "NativeStream.h"
#include "native_netlib.h"

#define NATIVE_AVIO_BUFFER_SIZE 32768 
#define CORO_STACK_SIZE         4096*4
#define NAV_IO_BUFFER_SIZE      NATIVE_AVIO_BUFFER_SIZE*8

#define SOURCE_EVENT_PLAY      0x01
#define SOURCE_EVENT_PAUSE     0x02
#define SOURCE_EVENT_STOP      0x03
#define SOURCE_EVENT_EOF       0x04
#define SOURCE_EVENT_ERROR     0x05
#define SOURCE_EVENT_BUFFERING 0x06
#define SOURCE_EVENT_READY     0x07

struct AVDictionary;
struct AVFormatContext;
class NativeAudioTrack;
struct PaUtilRingBuffer;
class NativeAVSource;
struct Coro;

class NativeAVReader
{
    public:
        NativeAVReader() : pending(false), needWakup(false), async(false)  {};

        bool pending;
        bool needWakup;
        bool async;

        virtual ~NativeAVReader() {};
};

class NativeAVBufferReader : public NativeAVReader
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

class NativeAVStreamReader : public NativeAVReader, public NativeStreamDelegate 
{
    public:
        NativeAVStreamReader(const char *chroot, const char *src, bool *readFlag, pthread_cond_t *bufferCond, NativeAVSource *source, ape_global *net);

        NativeAVSource *source;
        int64_t totalRead;

        static int read(void *opaque, uint8_t *buffer, int size);
        static int64_t seek(void *opaque, int64_t offset, int whence);
        
        void onGetContent(const char *data, size_t len) {}
        void onAvailableData(size_t len);
        void onProgress(size_t buffered, size_t len);
        void onError(NativeStream::StreamError err);
        ~NativeAVStreamReader();
    private:
        NativeStream *stream;
        bool *readFlag;
        bool opened;
        pthread_cond_t *bufferCond;

        size_t streamRead;
        size_t streamPacketSize;
        off_t streamSize;
        unsigned const char* streamBuffer;
        int error;
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
    NATIVE_AUDIO_NODE_ONSET_CALLBACK,
    NATIVE_AUDIO_NODE_CALLBACK,
    NATIVE_AUDIO_SOURCE_CALLBACK, // XXX : Not used?
    NATIVE_AUDIO_CALLBACK,
};

enum {
    ERR_FAILED_OPEN,
    ERR_READING,
    ERR_NO_INFORMATION,
    ERR_NO_AUDIO,
    ERR_NO_VIDEO,
    ERR_NO_CODEC,
    ERR_OOM,
    ERR_NO_RESAMPLING_CONVERTER,
    ERR_NO_VIDEO_CONVERTER,
    ERR_INIT_VIDEO_AUDIO,
    ERR_DECODING,
    ERR_SEEKING,
    ERR_INTERNAL,
    ERR_STREAMING_NOT_SUPPORTED,
    ERR_UNKNOWN,
    ERR_MAX
};

static const char *NativeAVErrorsStr[ERR_MAX] = {
    "Failed to open media file",
    "Failed to read media file",
    "Failed to find stream information",
    "No audio stream",
    "No video stream",
    "No codec available to decode stream",
    "Out of memory",
    "No resampling converter available",
    "No video converter available",
    "Failed to init audio stream from video",
    "Failed to decode stream",
    "Failed to seek",
    "Internal error",
    "HTTP Streaming on unknown file size is not supported yet",
    "Unknown error"
}; 

// Used for event (play, pause, stop, error, buffered...)
typedef void (*NativeAVSourceEventCallback)(const struct NativeAVSourceEvent*ev); 

struct NativeAVSourceEvent {
    int ev;
    int value1;
    int value2;
    NativeAVSource *source;
    void *custom;
    bool fromThread;
    NativeAVSourceEvent(NativeAVSource *source, int ev, int value1, int value2, 
            void *custom, bool fromThread)
        : ev(ev), value1(value1), value2(value2), source(source), custom(custom), 
            fromThread(fromThread) 
    {
        //this->value = new int;
        //memcpy(this->value, (void *)&value, sizeof(int));
    };
};

class NativeAVSource 
{
    public :
        NativeAVSource();

        friend class NativeAVSource;
        friend class NativeAVStreamReader;

        NativeAVSourceEventCallback eventCbk;
        void *eventCbkCustom;
        bool opened;
        bool eof;
        static pthread_mutex_t ffmpegLock;

        void eventCallback(NativeAVSourceEventCallback cbk, void *custom);
        void sendEvent(int ev, int value1, int value2, bool fromThread);

        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual void close() = 0;
        virtual int open(const char *chroot, const char *src) = 0;
        virtual int open(void *buffer, int size) = 0;
        virtual int openInit() = 0;
        virtual void onProgress(size_t buffered, size_t total) = 0;
        
        virtual double getClock() = 0;
        virtual void seek(double time) = 0;
        double getDuration();
        int getBitrate();
        AVDictionary *getMetadata();

        virtual ~NativeAVSource() = 0;
    protected:
	    AVFormatContext *container;
       
        Coro *coro;
        Coro *mainCoro;

        bool seeking;
        bool doSeek;
        double doSeekTime;
        int seekFlags;
        int error;

        int readError(int err);
};

#endif
