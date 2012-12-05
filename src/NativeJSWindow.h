#ifndef nativejswindow_h__
#define nativejswindow_h__

#include "NativeJSExposer.h"


class NativeJSwindow : public NativeJSExposer
{
  public:
    static void registerObject(JSContext *cx);
};

#endif
