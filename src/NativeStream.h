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
            INTERFACE_FRAMEWORK,
            INTERFACE_DATA,
            INTERFACE_UNKNOWN
        } IInterface;
        NativeStream(ape_global *net, const char *location);
        virtual ~NativeStream();
        const char *getLocation() const { return this->location; }

        void getContent();
        void getFileSize();

        NativeStreamDelegate *delegate;

        void setDelegate(NativeStreamDelegate *delegate) {
            this->delegate = delegate;
        }

        /* HTTP */
        void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
        void onProgress(size_t offset, size_t len,
            NativeHTTP::HTTPData *h, NativeHTTP::DataType);
        void onError(NativeHTTP::HTTPError err);

        /* File */
        void onNFIOOpen(NativeFileIO *);
        void onNFIOError(NativeFileIO *, int errno);
        void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
        void onNFIOWrite(NativeFileIO *, size_t written);
    private:
        char *location;
        NativeIStreamer *interface;
        NativeIStreamer *getInterface();
        void setInterface(StreamInterfaces interface, int path_offset);
        ape_global *net;

};

class NativeStreamDelegate
{
    public:
        virtual void onGetContent(const char *data, size_t len)=0;
};


#endif
