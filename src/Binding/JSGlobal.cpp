/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSGlobal.h"

#include <ape_timers_next.h>

#include "IO/Stream.h"
#include "Binding/JSUtils.h"

using Nidium::Core::Path;
using Nidium::Core::Utils;
using Nidium::IO::Stream;
using Nidium::Core::PtrAutoDelete;
using Nidium::Binding::JSUtils;

namespace Nidium {
namespace Binding {

// {{{ Preamble

enum {
    GLOBAL_PROP___DIRNAME,
    GLOBAL_PROP___FILENAME,
    GLOBAL_PROP_GLOBAL,
    GLOBAL_PROP_WINDOW
};

static bool nidium_global_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t, JS::MutableHandleValue vp);

static bool nidium_load(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_set_immediate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_set_timeout(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_set_interval(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_clear_timeout(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_btoa(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool nidium_readData(JSContext *cx, unsigned argc, JS::Value *vp);
//static void nidium_timer_wrapper(struct _nidium_sm_timer *params, int *last);
static int nidium_timerng_wrapper(void *arg);

JSFunctionSpec glob_funcs[] = {
    JS_FN("load", nidium_load, 2, NIDIUM_JS_FNPROPS),
    JS_FN("setTimeout", nidium_set_timeout, 2, NIDIUM_JS_FNPROPS),
    JS_FN("setImmediate", nidium_set_immediate, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setInterval", nidium_set_interval, 2, NIDIUM_JS_FNPROPS),
    JS_FN("clearTimeout", nidium_clear_timeout, 1, NIDIUM_JS_FNPROPS),
    JS_FN("clearInterval", nidium_clear_timeout, 1, NIDIUM_JS_FNPROPS),
    JS_FN("btoa", nidium_btoa, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

JSPropertySpec glob_props[] = {
    NIDIUM_JS_PSG("__filename", GLOBAL_PROP___FILENAME, nidium_global_prop_get),
    NIDIUM_JS_PSG("__dirname", GLOBAL_PROP___DIRNAME, nidium_global_prop_get),
    NIDIUM_JS_PSG("global", GLOBAL_PROP_GLOBAL, nidium_global_prop_get),
#ifndef NIDIUM_DISABLE_WINDOW_GLOBAL
    NIDIUM_JS_PSG("window", GLOBAL_PROP_WINDOW, nidium_global_prop_get),
#endif
    JS_PS_END
};

struct nidium_sm_timer
{
    JSContext *cx;

    JS::PersistentRootedObject global;
    JS::PersistentRootedValue **argv;
    JS::PersistentRootedValue func;

    unsigned argc;
    int ms;

    nidium_sm_timer(JSContext *cx) : cx(cx), global(cx), argv(NULL), func(cx), argc(0), ms(0) { }
    ~nidium_sm_timer() { }
};


JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(16) | JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook
};
// }}}

// {{{ Implementation
static bool nidium_global_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    switch(id) {
        case GLOBAL_PROP___FILENAME:
        {
            char *filename = JSUtils::CurrentJSCaller(cx);
            vp.setString(JS_NewStringCopyZ(cx, filename));
            free(filename);
            break;
        }
        case GLOBAL_PROP___DIRNAME:
        {
            Path path(JSUtils::CurrentJSCaller(cx), false, true);
            vp.setString(JS_NewStringCopyZ(cx, path.dir()));
            break;
        }
        case GLOBAL_PROP_WINDOW:
        case GLOBAL_PROP_GLOBAL:
        {
            JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
            vp.setObjectOrNull(global);
            break;
        }
        default:
            return false;
    }
    return true;
}

static bool nidium_load(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString script(cx);
    char *content;
    size_t len;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args, "S", script.address())) {
        return false;
    }

    NidiumJS *njs = NidiumJS::GetObject(cx);
    JSAutoByteString scriptstr(cx, script);
    Path scriptpath(scriptstr.ptr());

    Path::schemeInfo *schemeCwd = Path::GetCwdScheme();

    if (scriptpath.path() == NULL) {
        JS_ReportError(cx, "script error : invalid file location");
        return false;
    }

    /* only private are allowed in an http context */
    if (SCHEME_MATCH(schemeCwd, "http") &&
        !URLSCHEME_MATCH(scriptstr.ptr(), "private")) {
        JS_ReportError(cx, "script access error : cannot load in this context");
        return false;
    }

    if (!scriptpath.GetScheme()->AllowSyncStream()) {
        JS_ReportError(cx, "script error : \"%s\" scheme can't load in a sync way", schemeCwd->str);
        return false;
    }

    PtrAutoDelete<Stream *> stream(scriptpath.CreateStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&content, &len, true)) {
        JS_ReportError(cx, "load() failed read script");
        return false;
    }

    if (!njs->LoadScriptContent(content, len, scriptpath.path())) {
        JS_ReportError(cx, "load() failed to load script");
        return false;
    }

    return true;
}

// {{{ Timers
static int nidium_timer_deleted(void *arg)
{
    struct nidium_sm_timer *params = static_cast<struct nidium_sm_timer *>(arg);

    if (params == NULL) {
        return 0;
    }

    JSAutoRequest ar(params->cx);
    for (int i = 0; i < params->argc; i++) {
        delete params->argv[i];
    }
    delete[] params->argv;
    delete params;

    return 1;
}

static bool nidium_set_immediate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct nidium_sm_timer *params;
    int i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new nidium_sm_timer(cx);

    if (params == NULL || argc < 1) {
        if (params) delete params;
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-1;
    params->ms = 0;

    params->argv = new JS::PersistentRootedValue*[argc-1];

    for (i = 0; i < argc-1; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    JS::RootedValue func(cx);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        delete[] params->argv;
        delete params;
        return true;
    }

    params->func.set(func);

    for (i = 0; i < static_cast<int>(argc) - 1; i++) {
        params->argv[i]->set(args[i + 1]);
    }

    ape_timer_async_t *async = APE_async(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
                nidium_timerng_wrapper, static_cast<void *>(params));

    APE_async_setclearfunc(async, nidium_timer_deleted);

    args.rval().setNull();

    return true;
}

static bool nidium_set_timeout(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct nidium_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new nidium_sm_timer(cx);

    if (params == NULL || argc < 2) {
        if (params) delete params;
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->ms = 0;

    params->argv = new JS::PersistentRootedValue*[argc-2];

    for (i = 0; i < argc-2; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    JS::RootedValue func(cx);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        delete[] params->argv;
        delete params;
        return true;
    }

    if (!JS::ToInt32(cx, args[1], &ms)) {
        free(params->argv);
        delete params;
        return false;
    }

    params->func.set(func);

    for (i = 0; i < static_cast<int>(argc) - 2; i++) {
        params->argv[i]->set(args[i + 2]);
    }

    ape_timer_t *timer = APE_timer_create(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
        nidium_max(ms, 8), nidium_timerng_wrapper,
        static_cast<void *>(params));

    APE_timer_unprotect(timer);
    APE_timer_setclearfunc(timer, nidium_timer_deleted);

    args.rval().setNumber(static_cast<double>(APE_timer_getid(timer)));

    return true;
}

static bool nidium_set_interval(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct nidium_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new nidium_sm_timer(cx);

    if (params == NULL || argc < 2) {
        if (params) delete params;
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;

    params->argv = new JS::PersistentRootedValue*[argc-2];

    for (i = 0; i < argc-2; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    JS::RootedValue func(cx);
    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        delete[] params->argv;
        delete params;
        return true;
    }

    params->func.set(func);

    if (!JS::ToInt32(cx, args[1], &ms)) {
        delete[] params->argv;
        delete params;
        return false;
    }

    params->ms = nidium_max(8, ms);

    for (i = 0; i < static_cast<int>(argc) - 2; i++) {
        params->argv[i]->set(args.array()[i + 2]);
    }

    ape_timer_t *timer = APE_timer_create(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
        params->ms, nidium_timerng_wrapper, static_cast<void *>(params));

    APE_timer_unprotect(timer);
    APE_timer_setclearfunc(timer, nidium_timer_deleted);

    args.rval().setNumber(static_cast<double>(APE_timer_getid(timer)));

    return true;
}

static bool nidium_clear_timeout(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double identifier;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args, "d", &identifier)) {
        return false;
    }

    APE_timer_clearbyid(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
        static_cast<uint64_t>(identifier), 0);

    return true;
}

static int nidium_timerng_wrapper(void *arg)
{
    struct nidium_sm_timer *params = static_cast<struct nidium_sm_timer *>(arg);

    JSAutoRequest       ar(params->cx);
    JS::RootedValue     rval(params->cx);
    JS::AutoValueVector arr(params->cx);
    JS::RootedValue     func(params->cx, params->func);
    JS::RootedObject    global(params->cx, params->global);

    arr.resize(params->argc);
    for(size_t i = 0; i< params->argc; i++) {
        arr[i] = params->argv[i]->get();
    }
    JS_CallFunctionValue(params->cx, global, func, arr, &rval);

    //timers_stats_print(&((ape_global *)JS_GetContextPrivate(params->cx))->timersng);

    return params->ms;
}
// }}}

// {{{ Conversions
static bool nidium_btoa(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("btoa", 1);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (args[0].isString()) {

        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        char *ret = Utils::B64Encode(reinterpret_cast<unsigned char *>(cdata.ptr()), cdata.length());

        args.rval().setString(JS_NewStringCopyZ(cx, ret));

        free(ret);

    } else {
        args.rval().setNull();
        JS_ReportWarning(cx, "btoa() non-string given");
    }

    return true;
}
// }}}
// }}}

// {{{ Registration
// }}}

} // namespace Binding
} // namespace Nidium

