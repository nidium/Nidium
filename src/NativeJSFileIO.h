#ifndef nativejsfileio_h__
#define nativejsfileio_h__

#include "NativeJSExposer.h"
#include "NativeFileIO.h"

class NativeJSFileIO : public NativeJSExposer, public NativeFileIODelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSFileIO() {};
    ~NativeJSFileIO() {};

    void onNFIOOpen(NativeFileIO *);
    void onNFIOError(NativeFileIO *, int errno);
    void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);

    NativeFileIO *getNFIO() const { return NFIO; }
    void setNFIO(NativeFileIO *nfio) { NFIO = nfio; }

    struct {
        jsval open;
        jsval getContents;
        jsval read;
    } callbacks;

    JSObject *jsobj;

  private:

    NativeFileIO *NFIO;


};

#endif
