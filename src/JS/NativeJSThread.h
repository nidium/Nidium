/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef nativejsthread_h__
#define nativejsthread_h__

#include <pthread.h>

#include "NativeSharedMessages.h"
#include "NativeMessages.h"
#include "NativeJSExposer.h"

class NativeJS;

class NativeJSThread : public NativeJSExposer<NativeJSThread>, public NativeMessages
{
  public:
    NativeJSThread(JS::HandleObject obj, JSContext *cx);
    ~NativeJSThread();
    static void registerObject(JSContext *cx);
    void onComplete(JS::HandleValue vp);
    void onMessage(const NativeSharedMessages::Message &msg);

    JS::PersistentRootedString jsFunction;

    JSRuntime *jsRuntime;
    JSContext *jsCx;
    JSObject *jsObject;
    NativeJS *njs;
    struct {
        int argc;
        uint64_t **argv;
        size_t *nbytes;
    } params;
    bool markedStop;

    pthread_t threadHandle;

    char *m_CallerFileName;
    uint32_t m_CallerLineno;
};

enum {
    NATIVE_THREAD_MESSAGE = 0,
    NATIVE_THREAD_COMPLETE = 1
};

#endif

