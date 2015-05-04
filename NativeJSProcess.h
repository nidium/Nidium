#ifndef nativejsprocess_h__
#define nativejsprocess_h__

#include "NativeJSExposer.h"

class NativeJSProcess : public NativeJSExposer<NativeJSProcess>
{
  public:
    NativeJSProcess(JSObject *obj, JSContext *cx) :
        NativeJSExposer<NativeJSProcess>(obj, cx) {};
    virtual ~NativeJSProcess(){};

    static void registerObject(JSContext *cx, char **argv, int argc, int workerId = 0);
    static const char *getJSObjectName() {
        return "process";
    }

    static JSClass *jsclass;
};


#endif
