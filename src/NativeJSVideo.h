#ifndef nativejsvideo_h__
#define nativejsvideo_h__

#include "NativeJSExposer.h"
#include "NativeVideo.h"
#include "NativeSkia.h"

class NativeJSVideo : public NativeJSExposer
{
    public :
        NativeJSVideo(char *file, NativeSkia *nskia, JSContext *cx);

        NativeVideo *video;
        JSObject *jsobj;
        JSContext *cx;
        NativeSkia *nskia;

        static void registerObject(JSContext *cx);
        
        ~NativeJSVideo();
};
#endif
