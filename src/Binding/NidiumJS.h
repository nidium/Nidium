/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_nidiumjs_h__
#define binding_nidiumjs_h__

#include <stdint.h>
#include <stddef.h>

#include <jspubtd.h>
#include <jsapi.h>
#include <js/StructuredClone.h>

#include "Core/Hash.h"
#include "Core/Messages.h"
#include "Core/SharedMessages.h"

struct _ape_htable;

namespace Nidium {
namespace Binding {

class JSModules;

#define NATIVE_JS_FNPROPS JSPROP_ENUMERATE | JSPROP_PERMANENT

enum {
    NIDIUM_SCTAG_FUNCTION = JS_SCTAG_USER_MIN+1,
    NIDIUM_SCTAG_HIDDEN,
    NIDIUM_SCTAG_MAX
};

enum {
    NIDIUM_KEY_SHIFT = 1 << 0,
    NIDIUM_KEY_ALT = 1 << 1,
    NIDIUM_KEY_CTRL = 1 << 2,
    NIDIUM_KEY_META = 1 << 3
};

struct nidium_thread_msg
{
    uint64_t *data;
    size_t nbytes;
    class JSObject *callee;
};

typedef struct _ape_global ape_global;
typedef void (*nidium_thread_message_t)(JSContext *cx, Nidium::Core::SharedMessages::Message *msg);

typedef struct _NidiumBytecodeScript {
    const char *name;
    int size;
    const unsigned char *data;
} NidiumBytecodeScript;

class NidiumJSDelegate;

class NidiumJS
{
    public:
        explicit NidiumJS(ape_global *net);
        ~NidiumJS();

        typedef int (*logger)(const char *format);
        typedef int (*vlogger)(const char *format, va_list ap);
        typedef int (*logger_clear)();

        JSContext *cx;
        Nidium::Core::SharedMessages *messages;

        Nidium::Core::Hash<JSObject *> jsobjects;

        struct _ape_htable *rootedObj;
        struct _ape_global *net;

        nidium_thread_message_t *registeredMessages;
        int registeredMessagesIdx;
        int registeredMessagesSize;

        static NidiumJS *getNidiumClass(JSContext *cx = NULL);
        static ape_global *getNet();
        static void initNet(ape_global *net);

        static void Init();

        void setPrivate(void *arg) {
            this->privateslot = arg;
        }
        void *getPrivate() const {
            return this->privateslot;
        }
        const char *getPath() const {
            return this->relPath;
        }

        JSContext *getJSContext() const {
            return this->cx;
        }

        void setPath(const char *path);

        bool isShuttingDown() const {
            return this->shutdown;
        }

        void setLogger(logger lfunc) {
            m_Logger = lfunc;
        }
        void setLogger(vlogger lfunc) {
            m_vLogger = lfunc;
        }

        void setLogger(logger_clear clearfunc) {
            m_LogClear = clearfunc;
        }

        void setStrictMode(bool val) {
            m_JSStrictMode = val;
        }

        void loadGlobalObjects();

        static void copyProperties(JSContext *cx, JS::HandleObject source, JS::MutableHandleObject into);
        static int LoadScriptReturn(JSContext *cx, const char *data,
            size_t len, const char *filename, JS::MutableHandleValue ret);
        static int LoadScriptReturn(JSContext *cx,
            const char *filename, JS::MutableHandleValue ret);
        int LoadScriptContent(const char *data, size_t len,
            const char *filename);

        char *LoadScriptContentAndGetResult(const char *data,
            size_t len, const char *filename);
        int LoadScript(const char *filename);
        int LoadBytecode(NidiumBytecodeScript *script);
        int LoadBytecode(void *data, int size, const char *filename);

        void rootObjectUntilShutdown(JSObject *obj);
        void unrootObject(JSObject *obj);
        void gc();
        void bindNetObject(ape_global *net);

        int registerMessage(nidium_thread_message_t cbk);
        void registerMessage(nidium_thread_message_t cbk, int id);
        void postMessage(void *dataPtr, int ev);

        static JSStructuredCloneCallbacks *jsscc;
        static JSObject *readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                                   uint32_t tag, uint32_t data, void *closure);

        static bool writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                                 JS::HandleObject obj, void *closure);

        void logf(const char *format, ...);
        void log(const char *format);
        void logclear();

        void setStructuredCloneAddition(WriteStructuredCloneOp write,
            ReadStructuredCloneOp read)
        {
            m_StructuredCloneAddition.write = write;
            m_StructuredCloneAddition.read = read;
        }

        ReadStructuredCloneOp getReadStructuredCloneAddition() const {
            return m_StructuredCloneAddition.read;
        }
        WriteStructuredCloneOp getWriteStructuredCloneAddition() const {
            return m_StructuredCloneAddition.write;
        }

        static JSObject *CreateJSGlobal(JSContext *cx);
        static void SetJSRuntimeOptions(JSRuntime *rt);
    private:
        Nidium::Binding::JSModules *modules;
        void *privateslot;
        bool shutdown;
        const char *relPath;
        JSCompartment *m_Compartment;
        bool m_JSStrictMode;

        /* argument list (...) */
        logger m_Logger;

        /* va_list argument */
        vlogger m_vLogger;

        logger_clear m_LogClear;

        struct {
            WriteStructuredCloneOp write;
            ReadStructuredCloneOp read;
        } m_StructuredCloneAddition;
};

} // namespace Binding
} // namespace Nidium

#endif

