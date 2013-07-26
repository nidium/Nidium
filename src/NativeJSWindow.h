#ifndef nativejswindow_h__
#define nativejswindow_h__

#include "NativeJSExposer.h"


class NativeJSwindow : public NativeJSExposer<NativeJSwindow>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif
