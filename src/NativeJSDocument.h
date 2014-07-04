#ifndef nativejsdocument_h__
#define nativejsdocument_h__

#include "NativeJSExposer.h"


class NativeJSdocument : public NativeJSExposer<NativeJSdocument>
{
  public:
    NativeJSdocument(){};
    ~NativeJSdocument(){};

    static bool showFPS;
    bool populateStyle(JSContext *cx, const char *data,
        size_t len, const char *filename);
    static void registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "document";
    }

    static JSClass *jsclass;

    JSObject *stylesheet;
};

#endif
