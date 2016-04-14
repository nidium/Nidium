/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsdebug_h__
#define nativejsdebug_h__

#include "NativeJSExposer.h"

class NativeJSDebug : public NativeJSExposer<NativeJSDebug>
{
  public:
    NativeJSDebug(JS::HandleObject obj, JSContext *cx) :
    NativeJSExposer<NativeJSDebug>(obj, cx)
    {};
    virtual ~NativeJSDebug() {};

    static void registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "Debug";
    }

    static JSClass *jsclass;
};

#endif

