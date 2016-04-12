/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsdebug_h__
#define nativejsdebug_h__

#include "JSExposer.h"

class NativeJSDebug : public Nidium::Binding::JSExposer<NativeJSDebug>
{
  public:
    NativeJSDebug(JS::HandleObject obj, JSContext *cx) :
    Nidium::Binding::JSExposer<NativeJSDebug>(obj, cx)
    {};
    virtual ~NativeJSDebug() {};

    static void registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "Debug";
    }

    static JSClass *jsclass;
};

#endif

