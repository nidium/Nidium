/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsfs_h__
#define nativejsfs_h__

#include "Core/NativeMessages.h"
#include "Core/NativeTaskManager.h"
#include "JSExposer.h"

class NativeJSFS :     public Nidium::Binding::JSExposer<NativeJSFS>,
                       public NativeManaged
{
public:
    static void registerObject(JSContext *cx);
};

#endif

