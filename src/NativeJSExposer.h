#ifndef nativejsexposer_h__
#define nativejsexposer_h__

#include <jsapi.h>
#include <jsfriendapi.h>

template <typename T>
class NativeJSExposer
{
  public:
    JSContext *cx;
    JSObject *getJSObject() const {
        return this->jsobj;
    }
    JSObject *jsobj;
    T *getObject() const {
        if (jsobj == NULL) {
            return NULL;
        }
        return (T *)JS_GetPrivate(jsobj);
    }

    static const char *getJSObjectName() { return NULL; };

    static JSObject *getJSGlobalObject(JSContext *cx) {
        jsval obj;
        const char *name = T::getJSObjectName();
        if (name == NULL || !JS_GetProperty(cx, JS_GetGlobalObject(cx),
            name, &obj) || obj == JSVAL_VOID || obj == JSVAL_NULL) {
            return NULL;
        }

        return JSVAL_TO_OBJECT(obj);
    }

    static T* getNativeClass(JSContext *cx) {
        JSObject *obj = T::getJSGlobalObject(cx);
        if (obj == NULL) {
            return NULL;
        }
        return (T *)JS_GetPrivate(obj);
    } 
};

typedef bool (*register_module_t)(JSContext *cx, JSObject *exports);

#define NativeJSObj(cx) ((NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

#define NATIVE_OBJECT_EXPOSE(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &name ## _class, \
            native_ ## name ## _constructor, \
            0, NULL, NULL, NULL, NULL); \
    }

#define NATIVE_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JSObject *name ## Obj; \
        name ## Obj = JS_DefineObject(cx, JS_GetGlobalObject(cx), #name, \
            &name ## _class , NULL, 0); \
        JS_DefineFunctions(cx, name ## Obj, name ## _funcs); \
        JS_DefineProperties(cx, name ## Obj, name ## _props); \
    }

#define NATIVE_REGISTER_MODULE(constructor) \
    extern "C" bool __NativeRegisterModule(JSContext *cx, JSObject *exports) \
    { \
        return constructor(cx, exports); \
    }

#define NATIVE_CHECK_ARGS(fnname, minarg) \
    if (argc < minarg) { \
                         \
        char numBuf[12];  \
        snprintf(numBuf, sizeof numBuf, "%u", argc);  \
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,  \
                             fnname, numBuf, (argc > 1 ? "s" : ""));  \
        return JS_FALSE;  \
    }

#endif
