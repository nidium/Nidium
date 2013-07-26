#ifndef nativejsdocument_h__
#define nativejsdocument_h__

#include "NativeJSExposer.h"


class NativeJSdocument : public NativeJSExposer<NativeJSdocument>
{
  public:
    NativeJSdocument(){};
    ~NativeJSdocument(){};
    bool populateStyle(JSContext *cx, const char *filename);
    static void registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "document";
    }
    JSObject *stylesheet;
};

#endif
