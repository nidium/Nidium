/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsconsole_h__
#define nativejsconsole_h__

#include "JSExposer.h"

class NativeJSconsole : public Nidium::Binding::JSExposer<NativeJSconsole>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

