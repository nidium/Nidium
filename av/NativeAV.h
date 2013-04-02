#ifndef nativeav_h__
#define nativeav_h__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "NativeFileIO.h"
#include "native_netlib.h"

#define NATIVE_AVIO_BUFFER_SIZE 32768 
#define NATIVE_AV_CO_STACK_SIZE 4096
#define CORO_STACK_SIZE         NATIVE_AV_CO_STACK_SIZE*16
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

class NativeAVFileReader : public NativeAVReader, public NativeFileIODelegate
{
    public:
        NativeAVFileReader(const char *src, pthread_cond_t *bufferCond, NativeAVSource *source, ape_global *net);

        NativeAVSource *source;

        static int read(void *opaque, uint8_t *buffer, int size);
        static int64_t seek(void *opaque, int64_t offset, int whence);
        
        void onNFIOError(NativeFileIO *, int err);
        void onNFIOOpen(NativeFileIO *);
        void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
        void onNFIOWrite(NativeFileIO *, size_t written);

        bool nfioRead;
        
        ~NativeAVFileReader();
    private:
        NativeFileIO *nfio;
        pthread_cond_t *bufferCond;

        int dataSize;
        uint8_t *buffer;

        int64_t totalRead;
        int error;

        inline int checkCoro();
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
    ERR_INTERNAL
};

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
        NativeAVSource();

        friend class NativeAVSource;
        friend class NativeAVFileReader;

        NativeAVSourceEventCallback eventCbk;
        void *eventCbkCustom;
        bool opened;
        bool eof;

        void eventCallback(NativeAVSourceEventCallback cbk, void *custom);
        void sendEvent(int ev, int value, bool fromThread);

        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual int open(const char *src) = 0;
        virtual int open(void *buffer, int size) = 0;
        virtual int openInit() = 0;
        
        virtual double getClock() = 0;
        virtual void seek(double time) = 0;
        double getDuration();
        AVDictionary *getMetadata() ;
    protected:
	    AVFormatContext *container;
       
        Coro *coro;
        Coro *mainCoro;

        bool seeking;
        bool doSeek;
        double doSeekTime;
        int error;

        int readError(int err);
};

#endif
