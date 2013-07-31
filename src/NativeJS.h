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
    private:   
        NativeJSModules *modules;
        void *privateslot;
    public:
        JSContext *cx;
        NativeSharedMessages *messages;

        NativeHash<JSObject *> jsobjects;
        bool shutdown;
        struct _ape_htable *rootedObj;
        struct _ape_global *net;

        static NativeJS *getNativeClass(JSContext *cx);

        NativeJS(ape_global *net);
        ~NativeJS();

        void setPrivate(void *arg) {
            this->privateslot = arg;
        }
        void *getPrivate() const {
            return this->privateslot;
        }

        void loadGlobalObjects();
        //int LoadApplication(const char *path);
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
        void forceLinking();
        uint32_t currentFPS;

        struct {
            uint64_t nframe;
            uint64_t starttime;
            uint64_t lastmeasuredtime;
            uint64_t lastdifftime;
            uint32_t cumulframe;
            float cumultimems;
            float samples[60];
            float fps;
            float minfps;
            float sampleminfps;
        } stats;

};

#endif
