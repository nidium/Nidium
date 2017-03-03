/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSVM.h"
#include "IO/Stream.h"
#include "JSModules.h"
#include "JSStream.h"
#include "NidiumJS.h"

#include <js/CharacterEncoding.h>
#include <jsfriendapi.h>

namespace Nidium {
namespace Binding {

// {{{ JSVM
bool JSVM::JSStatic_run(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue rval(cx);

    JS::RootedObject scope(cx);
    JS::RootedString filename(cx);
    JSVMSandbox *sandbox = nullptr;
    int32_t lineOffset   = 1;
    bool debugger        = false;

    char16_t *data = nullptr;
    size_t len     = 0;

    if (!JSVM::GetRunData(cx, args[0], &data, &len)) {
        return false;
    }

    if (args.length() > 1) {
        if (!JSVM::ParseOptions(cx, args[1], &sandbox, &scope, &filename,
                                &lineOffset, &debugger)) {
            return false;
        }
    }

    {
        JS::RootedObject gbl(cx);
        if (!sandbox) {
            gbl = JS::CurrentGlobalOrNull(cx);
        } else {
            gbl = sandbox->getGlobal();
        }

        JSAutoCompartment ac(cx, gbl);

        JSAutoByteString filenameStr;
        JS::AutoObjectVector scopeChain(cx);
        if (scope) {
            if (JS_IsGlobalObject(scope)) {
                JS_ReportError(cx, "scope options cannot be a global object");
                return false;
            }
            scopeChain.append(scope);
        }

        JS::CompileOptions options(cx);
        options.setUTF8(true).setIsRunOnce(true);

        if (filename) {
            filenameStr.encodeUtf8(cx, filename);
            options.setFileAndLine(filenameStr.ptr(), lineOffset);
        } else {
            options.setFileAndLine("VM.run()", lineOffset);
        }

        if (!JS::Evaluate(cx, scopeChain, options, data, len, &rval)) {
            return false;
        }
    }

    JS_WrapValue(cx, &rval);

    args.rval().set(rval);

    return true;
}

bool JSVM::ParseOptions(JSContext *cx,
                        JS::HandleValue optionsValue,
                        JSVMSandbox **sandbox,
                        JS::MutableHandleObject scope,
                        JS::MutableHandleString filename,
                        int32_t *lineOffset,
                        bool *debugger)
{
    if (!optionsValue.isObject()) {
        JS_ReportError(cx, "run() second argument must be an object");
        return false;
    }

    JS::RootedObject options(cx, optionsValue.toObjectOrNull());

    NIDIUM_JS_INIT_OPT();

    NIDIUM_JS_GET_OPT_TYPE(options, "scope", Object)
    {
        scope.set(__curopt.toObjectOrNull());
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "filename", String)
    {
        filename.set(__curopt.toString());
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "debugger", Boolean)
    {
        *debugger = __curopt.toBoolean();
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "sandbox", Object)
    {
        JS::RootedObject obj(cx, __curopt.toObjectOrNull());
        if (JSVMSandbox::InstanceOf(obj)) {
            *sandbox = JSVMSandbox::GetInstance(obj);
        } else {
            *sandbox = new JSVMSandbox(cx, obj, 0 /* flags */);
        }

        // The global object of the sandbox only capture/resolve
        // global variables. To capture/resolve var definition
        // we need to provide a scope.
        if (!scope) {
            JSAutoCompartment ac(cx, (*sandbox)->getGlobal());
            JS_WrapObject(cx, &obj);
            scope.set(obj);
        }
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "lineOffset", Int32)
    {
        *lineOffset = __curopt.toInt32();
    }

    if (*(debugger) && !(*sandbox)) {
        JS_ReportError(cx, "debugger can only be enabled within a sandbox");
        return false;
    }

    return true;
}

bool JSVM::GetRunData(JSContext *cx,
                      JS::HandleValue arg,
                      char16_t **outData,
                      size_t *outLen)
{
    JS::RootedString str(cx);
    IO::Stream *stream = nullptr;

    if (arg.isString()) {
        str = arg.toString();
    } else if (arg.isObject()) {
        JS::RootedObject obj(cx, arg.toObjectOrNull());
        if (JSStream::InstanceOf(obj)) {
            JSStream *tmp = JSStream::GetInstance(obj);
            if (!tmp || !(stream = tmp->getStream())) {
                JS_ReportError(cx, "run() invalid Stream object");
                return false;
            }
        } else {
            JS_ReportError(
                cx, "First argument is not a Stream instance");
            return false;
        }
    } else {
        JS_ReportError(cx, "First argument must be a String or Stream instance");
        return false;
    }

    size_t length;
    char *chars;
    JSAutoByteString code;

    if (stream) {
        if (!stream->getContentSync(&chars, &length)) {
            JS_ReportError(cx, "Failed to get Stream content");
            return false;
        }
    } else {
        code.encodeUtf8(cx, JS::RootedString(cx, str));

        length = code.length();
        chars  = code.ptr();
    }

    *outData = JS::UTF8CharsToNewTwoByteCharsZ(cx,
                    JS::UTF8Chars(chars, length), outLen).get();

    return true;
}

JSFunctionSpec *JSVM::ListStaticMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN_STATIC(JSVM, run, 1), JS_FS_END };

    return funcs;
}

void JSVM::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("VM", JSVM::RegisterModule);
}

JSObject *JSVM::RegisterModule(JSContext *cx)
{
    JS::RootedObject vm(cx, JSVM::ExposeObject(cx, "VM"));

    return vm;
}
// }}}

// {{{ JSVMSandbox
JSObject *JSVMSandbox::CreateObject(JSContext *cx, JSVMSandbox *instance)
{
    JS::CompartmentOptions options;
    JS::RootedObject gbl(
        cx, JS_NewGlobalObject(cx, JSVMSandbox::GetJSClass(), nullptr,
                               JS::DontFireOnNewGlobalHook, options));
    if (!gbl) {
        return nullptr;
    }

    JSVMSandbox::AssociateObject(cx, instance, gbl);

    return gbl;
}

JSClass *JSVMSandbox::GetJSClass()
{
    static JSClass JSVMSandboxGlobal {
        "SandboxGlobal", JSCLASS_GLOBAL_FLAGS | JSCLASS_HAS_PRIVATE,
        nullptr, nullptr,
        JSVMSandbox::Getter, JSVMSandbox::Setter,
        nullptr, nullptr,
        nullptr, nullptr,
        nullptr, nullptr,
        nullptr, JS_GlobalObjectTraceHook
    };

    return &JSVMSandboxGlobal;
}

JSVMSandbox::JSVMSandbox(JSContext *cx, JS::HandleObject obj, int flags)
    : m_FLags(flags | kSandbox_HasStdClass)
{
    JS::RootedObject gbl(cx, JSVMSandbox::CreateObject(cx, this));
    if (!gbl) {
        JS_ReportError(cx, "Cannot create global for Sandbox");
        return;
    }

    m_MainGlobal    = JS::CurrentGlobalOrNull(cx);
    m_SandboxGlobal = gbl;
    m_Obj           = obj;

    {
        JSAutoCompartment ac(cx, m_SandboxGlobal);

        if (this->hasDebugger()) {
            JS_DefineDebuggerObject(cx, gbl);
        }

        if (this->hasStdClass()) {
            JS_InitStandardClasses(cx, gbl);
            JS_InitCTypesClass(cx, gbl);
        }

        JS_FireOnNewGlobalObject(cx, gbl);
    }
}

JSVMSandbox::~JSVMSandbox()
{
}

bool JSVMSandbox::Getter(JSContext *cx,
                         JS::HandleObject obj,
                         JS::HandleId id,
                         JS::MutableHandleValue vp)
{
    JSVMSandbox *sandbox = JSVMSandbox::GetInstance(obj);
    if (!sandbox) {
        JS_ReportError(cx, "Illegal invocation");
        return false;
    }

    return sandbox->get(cx, id, vp);
}

bool JSVMSandbox::Setter(JSContext *cx,
                         JS::HandleObject obj,
                         JS::HandleId id,
                         JS::MutableHandleValue vp,
                         JS::ObjectOpResult &result)
{
    JSVMSandbox *sandbox = JSVMSandbox::GetInstance(obj);
    if (!sandbox) {
        JS_ReportError(cx, "Illegal invocation");
        return false;
    }

    sandbox->set(cx, id, vp, result);

    return true;
}

bool JSVMSandbox::get(JSContext *cx, JS::HandleId id, JS::MutableHandleValue vp)
{
    JS::RootedValue val(cx);
    JS::RootedObject obj(cx, m_Obj);

    {
        JSAutoCompartment ac(cx, m_MainGlobal);

        if (!(JS_GetPropertyById(cx, obj, id, &val))) {
            JS_ReportError(cx, "Failed to get property");
            return false;
        }

        JS_WrapValue(cx, &val);
    }

    vp.set(val);

    return true;
}

bool JSVMSandbox::set(JSContext *cx,
                      JS::HandleId id,
                      JS::MutableHandleValue vp,
                      JS::ObjectOpResult &result)
{
    JS::RootedObject obj(cx, m_Obj);

    JS_WrapValue(cx, vp);

    {
        JSAutoCompartment ac(cx, m_MainGlobal);

        if (!JS_SetPropertyById(cx, obj, id, vp)) {
            JS_ReportError(cx, "Failed to set property");
            return false;
        }
    }

    result.succeed();

    return true;
}
// }}}

} // namespace Binding
} // namespace Nidium
