/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSConsole.h"
#include "Core/Context.h"

#include <string.h>
#include <math.h>

namespace Nidium {
namespace Binding {

// {{{ Implementation

bool JSConsole::JS_hide(JSContext *cx, JS::CallArgs &args)
{

    NidiumJS::GetObject(cx)->getContext()->logHide();

    return true;
}

bool JSConsole::JS_show(JSContext *cx, JS::CallArgs &args)
{

    NidiumJS::GetObject(cx)->getContext()->logShow();

    return true;
}

bool JSConsole::JS_clear(JSContext *cx, JS::CallArgs &args)
{

    NidiumJS::GetObject(cx)->getContext()->logClear();

    return true;
}

bool JSConsole::JS_log(JSContext *cx, JS::CallArgs &args)
{
    unsigned i;
    const char *filename_parent;
    unsigned lineno;

    JS::AutoFilename filename;
    JS::DescribeScriptedCaller(cx, &filename, &lineno);

    NidiumJS *js        = NidiumJS::GetObject();
    Core::Context *nctx = js->getContext();

    Core::AutoContextLogBuffering aclb(nctx);

    filename_parent = filename.get();
    if (filename_parent == NULL) {
        filename_parent = "(null)";
    }

    const char *fname = strrchr(filename_parent, '/');
    if (fname != NULL) {
        filename_parent = &fname[1];
    }

    for (i = 0; i < args.length(); i++) {
        JS::RootedString str(cx, JS::ToString(cx, args[i]));
        if (!str) {
            return false;
        }

        char * bytes = JS_EncodeStringToUTF8(cx, str);
        if (!bytes) {
            return false;
        }

        if (i) {
            nctx->log(" ");
        } else {
#if 0
#ifdef NIDIUM_PRODUCT_SERVER
            Context *nctx = Context::GetObject(cx);
            if (!nctx->isREPL()) {
                js->logf("(worker %d) [%s:%d] ", nctx->getWorker()->getIdentifier(), filename_parent, lineno);
            }
#else
            js->logf("[%s:%d] ", filename_parent, lineno);
#endif
#endif
        }

        nctx->log(bytes);

        JS_free(cx, bytes);
    }

    nctx->log("\n");

    return true;
}

bool JSConsole::JS_write(JSContext *cx, JS::CallArgs &args)
{
    NidiumJS *js        = NidiumJS::GetObject(cx);
    Core::Context *nctx = js->getContext();

    JS::RootedString str(cx, args[0].toString());
    if (!str) {
        JS_ReportErrorUTF8(cx, "Bad argument");
        return false;
    }

    JSAutoByteString cstr;

    cstr.encodeUtf8(cx, str);

    nctx->log(cstr.ptr());

    args.rval().setUndefined();
    return true;
}
// }}}

JSFunctionSpec *JSConsole::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSConsole, log, 0),
        CLASSMAPPER_FN(JSConsole, write, 1),
        CLASSMAPPER_FN(JSConsole, hide, 0),
        CLASSMAPPER_FN(JSConsole, clear, 0),
        CLASSMAPPER_FN(JSConsole, show, 0),
        CLASSMAPPER_FN_ALIAS(JSConsole, info, 0, log),
        CLASSMAPPER_FN_ALIAS(JSConsole, error, 0, log),
        CLASSMAPPER_FN_ALIAS(JSConsole, warn, 0, log),
        JS_FS_END
    };

    return funcs;
}

void JSConsole::RegisterObject(JSContext *cx)
{
    JSConsole::ExposeClass(cx, "Console");
    JSConsole::CreateUniqueInstance(cx, new JSConsole(), "console");
}

} // namespace Binding
} // namespace Nidium
