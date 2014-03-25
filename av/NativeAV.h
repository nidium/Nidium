#ifndef nativeav_h__
#define nativeav_h__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "NativeStreamInterface.h"
#include "NativeMessages.h"
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

#define NATIVE_PTHREAD_VAR_DECL(name)\
pthread_cond_t name;\
pthread_mutex_t name##Lock;\
bool name##Cond;\

#define NATIVE_PTHREAD_VAR_INIT(name)\
pthread_cond_init(name, NULL);\
pthread_mutex_init(name##Lock, NULL);\
*(name##Cond) = false;

#define NATIVE_PTHREAD_WAIT(mutexRef)\
pthread_mutex_lock(mutexRef##Lock);\
while (*(mutexRef##Cond) == false) {\
    pthread_cond_wait(mutexRef, mutexRef##Lock);\
}\
pthread_mutex_unlock(mutexRef##Lock);\
*(mutexRef##Cond) = false;

#define NATIVE_PTHREAD_SIGNAL(mutexRef)\
*(mutexRef##Cond) = true;\
pthread_cond_signal(mutexRef);

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

typedef void (*NativeAVStreamReadCallback)(void *callbackPrivate);
class NativeAVStreamReader : public NativeAVReader, public NativeMessages
{
    public:
        NativeAVStreamReader(const char *src, NativeAVStreamReadCallback readCallback, void *callbackPrivate, NativeAVSource *source, ape_global *net);

        NativeAVSource *source;
        int64_t totalRead;

        static int read(void *opaque, uint8_t *buffer, int size);
        static int64_t seek(void *opaque, int64_t offset, int whence);
        
        void onAvailableData(size_t len);
        ~NativeAVStreamReader();
    private:
        enum {
            MSG_SEEK,
            MSG_READ,
            MSG_STOP
        };
        void onMessage(const NativeSharedMessages::Message &msg);
        NATIVE_PTHREAD_VAR_DECL(m_ThreadCond);

        NativeBaseStream *stream;
        NativeAVStreamReadCallback readCallback;
        void *callbackPrivate;

        bool opened;

        size_t streamRead;
        size_t streamPacketSize;
        int streamErr;
        size_t streamSeekPos;
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


// Shared messages enum
enum {
    NATIVE_AUDIO_NODE_SET,
    NATIVE_AUDIO_NODE_ONSET_CALLBACK,
    NATIVE_AUDIO_NODE_CALLBACK,
    NATIVE_AUDIO_SOURCE_CALLBACK, // XXX : Not used?
    NATIVE_AUDIO_CALLBACK,
};

// Error code and description
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

const char * const NativeAVErrorsStr[ERR_MAX] = {
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
    NativeAVSource *source;
    int ev;
    NativeArgs args;
    void *custom;
    bool fromThread;
    NativeAVSourceEvent(NativeAVSource *source, int ev, void *custom, bool fromThread)
        : source(source), ev(ev), custom(custom), fromThread(fromThread) 
    {
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
        NativeAVSourceEvent *createEvent(int ev, bool fromThread);
        void sendEvent(int ev, int value, bool fromThread);
        void sendEvent(NativeAVSourceEvent *ev);

        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual void close() = 0;
        virtual int open(const char *src) = 0;
        virtual int open(void *buffer, int size) = 0;
        virtual int openInit() = 0;
        
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

        bool m_SourceDoOpen;
        bool m_SourceDoClose;

        int readError(int err);
};

#endif
