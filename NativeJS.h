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

#ifndef nativejs_h__
#define nativejs_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeHash.h"
#include "NativeSharedMessages.h"
#include <jspubtd.h>

enum {
    NATIVE_KEY_SHIFT = 1 << 0,
    NATIVE_KEY_ALT = 1 << 1,
    NATIVE_KEY_CTRL = 1 << 2
};

struct native_thread_msg
{
    uint64_t *data;
    size_t nbytes;
    class JSObject *callee;
};

class NativeSharedMessages;
class NativeSkia;
class NativeCanvasHandler;
class NativeUIInterface;
class NativeJSModules;
struct _ape_htable;

typedef struct _ape_global ape_global;
typedef void (*native_thread_message_t)(JSContext *cx, NativeSharedMessages::Message *msg);

class NativeJS
{
    public:
        NativeJS(ape_global *net);
        ~NativeJS();

        JSContext *cx;
        NativeSharedMessages *messages;

        NativeHash<JSObject *> jsobjects;
        struct _ape_htable *rootedObj;
        struct _ape_global *net;

        native_thread_message_t *registeredMessages;
        int registeredMessagesIdx;
        int registeredMessagesSize;

        static NativeJS *getNativeClass(JSContext *cx);

        void setPrivate(void *arg) {
            this->privateslot = arg;
        }
        void *getPrivate() const {
            return this->privateslot;
        }
        const char *getPath() const {
            return this->relPath;
        }
        void setPath(const char *path) {
            this->relPath = path;
        }

        bool isShuttingDown() const {
            return this->shutdown;
        }

        void loadGlobalObjects();
        
        static char *buildRelativePath(JSContext *cx, const char *file = NULL);
        static void copyProperties(JSContext *cx, JSObject *source, JSObject *into);
        static int LoadScriptReturn(JSContext *cx, const char *data,
            size_t len, const char *filename, JS::Value *ret);
        static int LoadScriptReturn(JSContext *cx,
            const char *filename, JS::Value *ret);
        int LoadScriptContent(const char *data, size_t len,
            const char *filename);
        int LoadScript(const char *filename);
        int LoadScript(const char *filename, JSObject *gbl);

        void rootObjectUntilShutdown(JSObject *obj);
        void unrootObject(JSObject *obj);
        void gc();
        void bindNetObject(ape_global *net);

        int registerMessage(native_thread_message_t cbk);
        void registerMessage(native_thread_message_t cbk, int id);
        void postMessage(void *dataPtr, int ev);
    private:   
        NativeJSModules *modules;
        void *privateslot;
        bool shutdown;
        const char *relPath;
};

#endif
