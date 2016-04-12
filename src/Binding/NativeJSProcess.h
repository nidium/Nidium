/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsprocess_h__
#define nativejsprocess_h__

#include "JSExposer.h"

class NativeJSProcess : public Nidium::Binding::JSExposer<NativeJSProcess>
{
  public:
    NativeJSProcess(JS::HandleObject obj, JSContext *cx) :
        Nidium::Binding::JSExposer<NativeJSProcess>(obj, cx), m_SignalFunction(cx) {};
    virtual ~NativeJSProcess() {};

    static void registerObject(JSContext *cx, char **argv, int argc, int workerId = 0);
    static const char *getJSObjectName() {
        return "process";
    }

    static JSClass *jsclass;

    JS::PersistentRootedValue m_SignalFunction;
};

#endif

