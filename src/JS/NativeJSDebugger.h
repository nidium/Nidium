/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsdebugger_h__
#define nativejsdebugger_h__

#include "NativeJSExposer.h"

class NativeJSDebugger : public NativeJSExposer<NativeJSDebugger>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

