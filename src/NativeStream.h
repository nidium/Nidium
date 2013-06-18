#ifndef nativestream_h__
#define nativestream_h__

#include <native_netlib.h>
#include "NativeHTTP.h"
#include "NativeFileIO.h"
#include "NativeIStreamer.h"

class NativeStreamDelegate;

class NativeStream : public NativeHTTPDelegate, public NativeFileIODelegate
{
    public:
        enum StreamInterfaces {
            INTERFACE_HTTP,
            INTERFACE_FILE,
            INTERFACE_PRIVATE,
            INTERFACE_DATA,
            INTERFACE_UNKNOWN
        } IInterface;

        enum StreamDataStatus {
            STREAM_EAGAIN = -1,
            STREAM_EOF = -2,
            STREAM_ERROR = -3,
            STREAM_END = -4
        };

        NativeStream(ape_global *net, const char *location);
        virtual ~NativeStream();
        const char *getLocation() const { return this->location; }

        void getContent();
        void getFileSize();
        void seek(size_t pos);
        void start(size_t packets = 4096, size_t seek=0);

        bool hasDataAvailable() const {
            return !dataBuffer.alreadyRead || (dataBuffer.ended && dataBuffer.back->used);
        }
        const unsigned char *getNextPacket(size_t *len, int *err);

        /*****************************/
        NativeStreamDelegate *delegate;

        void setDelegate(NativeStreamDelegate *delegate) {
            this->delegate = delegate;
        }

        /* HTTP */
        void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
        void onProgress(size_t offset, size_t len,
            NativeHTTP::HTTPData *h, NativeHTTP::DataType);
        void onError(NativeHTTP::HTTPError err);
        void onHeader();

        /* File */
        void onNFIOOpen(NativeFileIO *);
        void onNFIOError(NativeFileIO *, int errno);
        void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
        void onNFIOWrite(NativeFileIO *, size_t written);

        char *location;

        size_t getPacketSize() const {
            return this->packets;
        }
    private:
        NativeIStreamer *interface;
        NativeIStreamer *getInterface();
        void setInterface(StreamInterfaces interface, int path_offset);
        ape_global *net;
        size_t packets;
        struct {
            buffer *back;
            buffer *front;
            bool alreadyRead;
            bool fresh;
            bool ended;
        } dataBuffer;

        struct {
            int fd;
            void *addr;
            size_t size;
        } mapped;

        bool needToSendUpdate;

        void swapBuffer();
};

class NativeStreamDelegate
{
    public:
        virtual void onGetContent(const char *data, size_t len)=0;
        //virtual void onStreamRead()=0;
        //virtual void onStreamEnd()=0;
        virtual void onAvailableData(size_t len)=0;
};

#endif
