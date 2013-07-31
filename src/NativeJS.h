#ifndef nativejs_h__
#define nativejs_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeHash.h"
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

        static NativeJS *getNativeClass(JSContext *cx);


        void setPrivate(void *arg) {
            this->privateslot = arg;
        }
        void *getPrivate() const {
            return this->privateslot;
        }
        bool isShuttingDown() const {
            return this->shutdown;
        }

        void loadGlobalObjects();
        
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
    private:   
        NativeJSModules *modules;
        void *privateslot;
        bool shutdown;
};

#endif
