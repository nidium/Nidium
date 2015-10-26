#ifndef nativejsprocess_h__
#define nativejsprocess_h__

#include "NativeJSExposer.h"

class NativeJSProcess : public NativeJSExposer<NativeJSProcess>
{
  public:
    NativeJSProcess(JS::HandleObject obj, JSContext *cx) :
        NativeJSExposer<NativeJSProcess>(obj, cx), m_SignalFunction(cx) {};
    virtual ~NativeJSProcess(){};

    static void registerObject(JSContext *cx, char **argv, int argc, int workerId = 0);
    static const char *getJSObjectName() {
        return "process";
    }

    static JSClass *jsclass;

    JS::PersistentRootedValue m_SignalFunction;
};


#endif
