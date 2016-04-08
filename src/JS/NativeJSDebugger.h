#ifndef nativejsdebugger_h__
#define nativejsdebugger_h__

#include "NativeJSExposer.h"

class NativeJSDebugger : public NativeJSExposer<NativeJSDebugger>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

