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


static int nidium_timerng_wrapper(void *arg);


struct nidium_sm_timer
{
    JSContext *cx;

    JS::PersistentRootedObject global;
    JS::PersistentRootedValue **argv;
    JS::PersistentRootedValue func;

    int argc;
    int ms;

    nidium_sm_timer(JSContext *cx)
        : cx(cx), global(cx), argv(NULL), func(cx), argc(0), ms(0)
    {
    }
    ~nidium_sm_timer()
    {
    }
};

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

static int nidium_timerng_wrapper(void *arg)
{
    struct nidium_sm_timer *params = static_cast<struct nidium_sm_timer *>(arg);

    JSAutoRequest ar(params->cx);
    JS::RootedValue rval(params->cx);
    JS::AutoValueVector arr(params->cx);
    JS::RootedValue func(params->cx, params->func);
    JS::RootedObject global(params->cx, params->global);

    arr.resize(params->argc);

    for (int i = 0; i < params->argc; i++) {
        arr[i].set(params->argv[i]->get());
    }
    JS_CallFunctionValue(params->cx, global, func, arr, &rval);

    // timers_stats_print(&((ape_global
    // *)JS_GetContextPrivate(params->cx))->timersng);

    return params->ms;
}

// }}}

// {{{ Implementation

bool JSGlobal::JSGetter___filename(JSContext *cx, JS::MutableHandleValue vp)
{
    char *filename = JSUtils::CurrentJSCaller(cx);
    vp.setString(JS_NewStringCopyZ(cx, filename));
    free(filename);

    return true;
}

bool JSGlobal::JSGetter___dirname(JSContext *cx, JS::MutableHandleValue vp)
{
    Path path(JSUtils::CurrentJSCaller(cx), false, true);
    vp.setString(JS_NewStringCopyZ(cx, path.dir()));

    return true;
}

bool JSGlobal::JSGetter_global(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setObjectOrNull(m_Instance);

    return true;
}

bool JSGlobal::JS_load(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString script(cx);
    char *content;
    size_t len;

    if (!JS_ConvertArguments(cx, args, "S", script.address())) {
        return false;
    }

    JSAutoByteString scriptstr(cx, script);
    Path scriptpath(scriptstr.ptr());

    Path::schemeInfo *schemeCwd = Path::GetCwdScheme();

    if (scriptpath.path() == NULL) {
        JS_ReportErrorUTF8(cx, "script error : invalid file location");
        return false;
    }

    /* only embed are allowed in an http context */
    if (SCHEME_MATCH(schemeCwd, "http")
        && !URLSCHEME_MATCH(scriptstr.ptr(), "embed")) {
        JS_ReportErrorUTF8(cx, "script access error : cannot load in this context");
        return false;
    }

    if (!scriptpath.GetScheme()->AllowSyncStream()) {
        JS_ReportErrorUTF8(cx,
                       "script error : \"%s\" scheme can't load in a sync way",
                       schemeCwd->str);
        return false;
    }

    PtrAutoDelete<Stream *> stream(scriptpath.CreateStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&content, &len, true)) {
        JS_ReportErrorUTF8(cx, "load() failed read script");
        return false;
    }

    if (!m_JS->LoadScriptContent(content, len, scriptpath.path())) {
        JS_ReportErrorUTF8(cx, "load() failed to load script");
        return false;
    }

    return true;
}

// {{{ Timers

bool JSGlobal::JS_setImmediate(JSContext *cx, JS::CallArgs &args)
{
    struct nidium_sm_timer *params;
    int i;
    int argc = args.length();

    params = new nidium_sm_timer(cx);

    if (params == NULL || argc < 1) {
        if (params) delete params;
        return true;
    }

    params->cx     = cx;
    params->global = m_Instance;
    params->argc   = argc - 1;
    params->ms     = 0;

    params->argv = new JS::PersistentRootedValue *[argc - 1];

    for (i = 0; i < argc - 1; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    if (!JSUtils::ReportIfNotFunction(cx, args[0])) {
        delete[] params->argv;
        delete params;
        return false;
    }

    params->func = args[0];

    for (i = 0; i < static_cast<int>(argc) - 1; i++) {
        *params->argv[i] = args[i + 1];
    }

    ape_timer_async_t *async
        = APE_async(APE_get()),
                    nidium_timerng_wrapper, static_cast<void *>(params));

    APE_async_setclearfunc(async, nidium_timer_deleted);

    args.rval().setNull();

    return true;
}

bool JSGlobal::JS_setTimeout(JSContext *cx, JS::CallArgs &args)
{
    struct nidium_sm_timer *params;
    int ms = 0, i;
    int argc = args.length();

    params = new nidium_sm_timer(cx);

    if (!params) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    params->cx     = cx;
    params->global = m_Instance;
    params->argc   = nidium_max(0, argc - 2);
    params->ms     = 0;

    params->argv = new JS::PersistentRootedValue *[params->argc];

    for (i = 0; i < argc - 2; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    if (!JSUtils::ReportIfNotFunction(cx, args[0])) {
        delete[] params->argv;
        delete params;
        return false;
    }

    if (argc > 1 && !JS::ToInt32(cx, args[1], &ms)) {
        ms = 0;
    }

    params->func = args[0];

    for (i = 0; i < static_cast<int>(argc) - 2; i++) {
        *params->argv[i] = args[i + 2];
    }

    ape_timer_t *timer = APE_timer_create(
        APE_get(), nidium_max(ms, 8),
        nidium_timerng_wrapper, static_cast<void *>(params));

    APE_timer_unprotect(timer);
    APE_timer_setclearfunc(timer, nidium_timer_deleted);

    args.rval().setNumber(static_cast<double>(APE_timer_getid(timer)));

    return true;
}

bool JSGlobal::JS_setInterval(JSContext *cx, JS::CallArgs &args)
{
    struct nidium_sm_timer *params;
    int ms = 0, i;
    int argc = args.length();

    params = new nidium_sm_timer(cx);

    if (!params) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    params->cx     = cx;
    params->global = m_Instance;
    params->argc   = nidium_max(0, argc - 2);

    params->argv = new JS::PersistentRootedValue *[params->argc];

    for (i = 0; i < argc - 2; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    if (!JSUtils::ReportIfNotFunction(cx, args[0])) {
        delete[] params->argv;
        delete params;
        return false;
    }

    params->func = args[0];

    if (argc > 1 && !JS::ToInt32(cx, args[1], &ms)) {
        ms = 0;
    }

    params->ms = nidium_max(8, ms);

    for (i = 0; i < static_cast<int>(argc) - 2; i++) {
        *params->argv[i] = args.array()[i + 2];
    }

    ape_timer_t *timer = APE_timer_create(
        APE_get(), params->ms,
        nidium_timerng_wrapper, static_cast<void *>(params));

    APE_timer_unprotect(timer);
    APE_timer_setclearfunc(timer, nidium_timer_deleted);

    args.rval().setNumber(static_cast<double>(APE_timer_getid(timer)));

    return true;
}

bool JSGlobal::JS_clearTimeout(JSContext *cx, JS::CallArgs &args)
{
    double identifier;

    if (!JS_ConvertArguments(cx, args, "d", &identifier)) {
        return false;
    }

    APE_timer_clearbyid(APE_get(), static_cast<uint64_t>(identifier), 0);

    return true;
}

// }}}

// {{{ Conversions
bool JSGlobal::JS_btoa(JSContext *cx, JS::CallArgs &args)
{
    if (args[0].isString()) {

        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        char *ret = Utils::B64Encode(
            reinterpret_cast<unsigned char *>(cdata.ptr()), cdata.length());

        args.rval().setString(JS_NewStringCopyZ(cx, ret));

        free(ret);

    } else {
        args.rval().setNull();
        JS_ReportWarningUTF8(cx, "btoa() non-string given");
    }

    return true;
}
// }}}
// }}}

// {{{ Registration

JSClass *JSGlobal::GetJSClass()
{
    static JSClass global_class = {
        "global",
        JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(16) | JSCLASS_HAS_PRIVATE,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  JS_GlobalObjectTraceHook
    };

    return &global_class;
}

JSPropertySpec *JSGlobal::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_G(JSGlobal, __filename),
        CLASSMAPPER_PROP_G(JSGlobal, __dirname),
        CLASSMAPPER_PROP_G(JSGlobal, global),
#ifndef NIDIUM_DISABLE_WINDOW_GLOBAL
        CLASSMAPPER_PROP_G_ALIAS(JSGlobal, window, global),
#endif
        JS_PS_END
    };

    return props;
}

JSFunctionSpec *JSGlobal::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSGlobal, load, 1),
        CLASSMAPPER_FN(JSGlobal, setTimeout, 1),
        CLASSMAPPER_FN(JSGlobal, setImmediate, 1),
        CLASSMAPPER_FN(JSGlobal, setInterval, 1),
        CLASSMAPPER_FN(JSGlobal, clearTimeout, 1),
        CLASSMAPPER_FN_ALIAS(JSGlobal, clearInterval, 1, clearTimeout),
        CLASSMAPPER_FN_ALIAS(JSGlobal, clearImmediate, 1, clearTimeout),
        CLASSMAPPER_FN(JSGlobal, btoa, 1),
        JS_FS_END
    };

    return funcs;
}

void JSGlobal::RegisterObject(JSContext *cx,
    JS::HandleObject global, NidiumJS *njs)
{
    JSGlobal::AssociateObject(cx, new JSGlobal(njs), global, true);
}

// }}}

} // namespace Binding
} // namespace Nidium
