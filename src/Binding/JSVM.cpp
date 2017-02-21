/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSVM.h"
#include "JSModules.h"
#include "JSStream.h"
#include "NidiumJS.h"
#include "IO/Stream.h"

#include <js/CharacterEncoding.h>
#include <jsfriendapi.h>

namespace Nidium {
namespace Binding {

// {{{ JSVMAutoArgs helper
class JSVMAutoArgs
{
public:
    JSVMAutoArgs(JSContext *cx, JS::HandleValue args) : m_Args(cx), m_Cx(cx)
    {
        JS::Rooted<JS::IdVector> ida(cx, JS::IdVector(cx));
        JS::RootedObject funArgs(cx, args.toObjectOrNull());
        JS::RootedId id(cx);

        JS_Enumerate(cx, funArgs, &ida);

        m_Names = (char **)malloc(sizeof(char *) * ida.length());
        for (size_t i = 0; i < ida.length(); i++) {
            id = ida[i];

            if (!JSID_IS_STRING(id)) {
                continue;
            }

            JS::RootedString argName(cx, JSID_TO_STRING(id));
            JS::RootedValue val(cx);

            if (!JS_GetPropertyById(cx, funArgs, id, &val)) {
                continue;
            }

            m_Names[i] = JS_EncodeString(cx, argName);
            m_Args.append(val);
        }
    }

    const char **getNames()
    {
        const char **ret = const_cast<const char **>(m_Names);
        return ret;
    }

    JS::AutoValueVector &getValues()
    {
        return m_Args;
    }

    size_t length()
    {
        return m_Args.length();
    }

    ~JSVMAutoArgs()
    {
        for (size_t i = 0; i < this->length(); i++) {
            JS_free(m_Cx, m_Names[i]);
        }
        free(m_Names);
    }

private:
    char **m_Names = nullptr;
    JS::AutoValueVector m_Args;
    JSContext *m_Cx;
};
// }}}

// {{{ JSVM
bool JSVM::JSStatic_runInFunction(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue rval(cx);

    JS::RootedObject scope(cx);
    JS::RootedObject bind(cx);
    JS::RootedString filename(cx);
    JS::RootedString funName(cx);
    JSVMCompartment *cpt = nullptr;
    Core::PtrAutoDelete<JSVMAutoArgs *> _funArgs;
    JSAutoByteString filenameStr;
    JSVMAutoArgs *funArgs = nullptr;
    int32_t lineOffset    = 1;

    char16_t *data = nullptr;
    size_t len     = 0;

    if (!JSVM::ParseRunData(cx, args[0], &data, &len)) {
        return false;
    }

    if (args.length() > 1) {
        if (!JSVM::ParseCommonOptions(cx, args[1], &cpt, &scope, &filename, &lineOffset)) {
            return false;
        }

        JS::RootedObject options(cx, args[1].toObjectOrNull());

        NIDIUM_JS_INIT_OPT();

        NIDIUM_JS_GET_OPT_TYPE(options, "args", Object)
        {
            _funArgs.set(new JSVMAutoArgs(cx, __curopt));
            funArgs = _funArgs.ptr();
        }

        NIDIUM_JS_GET_OPT_TYPE(options, "bind", Object)
        {
            bind.set(__curopt.toObjectOrNull());
        }

        NIDIUM_JS_GET_OPT_TYPE(options, "name", String)
        {
            funName.set(__curopt.toString());
        }
    }

    {
        JS::RootedObject gbl(cx);
        if (!cpt) {
            gbl = JS::CurrentGlobalOrNull(cx);
        } else {
            gbl = cpt->getGlobal();
        }

        JSAutoCompartment ac(cx, gbl);

        JS::AutoObjectVector scopeChain(cx);
        if (scope) {
            if (JS_IsGlobalObject(scope)) {
                JS_ReportError(cx, "scope options cannot be a global object");
                return false;
            }
            scopeChain.append(scope);
        }

        JS::CompileOptions options(cx);
        options.setUTF8(true);

        if (filename) {
            filenameStr.encodeUtf8(cx, filename);
            options.setFileAndLine(filenameStr.ptr(), lineOffset);
        } else {
            options.setFileAndLine("VM.runInFunction()", lineOffset);
        }

        char *name = nullptr;
        JSAutoByteString funNameStr;

        if (funName) {
            funNameStr.encodeUtf8(cx, funName);
            name = funNameStr.ptr();
        }

        if (!bind) {
            bind = JS::CurrentGlobalOrNull(cx);
        }

        JS::RootedFunction fun(cx);
        if (!JS::CompileFunction(
                cx, scopeChain, options, name ? name : "VM.runInFunction",
                funArgs ? funArgs->length() : 0,
                funArgs ? funArgs->getNames() : nullptr, data, len, &fun)) {
            return false;
        }

        if (!JS_CallFunction(cx, bind, fun,
                             (funArgs
                                  ? JS::HandleValueArray(funArgs->getValues())
                                  : JS::HandleValueArray::empty()),
                             &rval)) {
            return false;
        }
    }

    JS_WrapValue(cx, &rval);

    args.rval().set(rval);

    return true;
}

bool JSVM::JSStatic_run(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue rval(cx);

    JS::RootedObject scope(cx);
    JS::RootedString filename(cx);
    JSVMCompartment *cpt = nullptr;
    int32_t lineOffset   = 1;

    char16_t *data = nullptr;
    size_t len     = 0;

    if (!JSVM::ParseRunData(cx, args[0], &data, &len)) {
        return false;
    }

    if (args.length() > 1) {
        if (!JSVM::ParseCommonOptions(cx, args[1], &cpt, &scope, &filename,
                                      &lineOffset)) {
            return false;
        }
    }

    {
        JS::RootedObject gbl(cx);
        if (!cpt) {
            gbl = JS::CurrentGlobalOrNull(cx);
        } else {
            gbl = cpt->getGlobal();
        }

        JSAutoCompartment ac(cx, gbl);

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
            JSAutoByteString filenameStr(cx, filename);
            options.setFileAndLine(filenameStr.ptr(), lineOffset);
        } else {
            options.setFileAndLine("VM.run()", lineOffset);
        }

        if (!JS::Evaluate(cx, scopeChain, options, data, len, args.rval())) {
            return false;
        }
    }

    JS_WrapValue(cx, &rval);

    args.rval().set(rval);

    return true;
}

JSFunctionSpec *JSVM::ListStaticMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN_STATIC(JSVM, run, 1),
            CLASSMAPPER_FN_STATIC(JSVM, runInFunction, 1),
            JS_FS_END };

    return funcs;
}

void JSVM::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("VM", JSVM::RegisterModule);
}

JSObject *JSVM::RegisterModule(JSContext *cx)
{
    JS::RootedObject vm(cx, JSVM::ExposeObject(cx, "VM"));

    JSVMCompartment::RegisterObject(cx, vm);

    return vm;
}
// }}}

// {{{ JSVMCompartment
JSVMCompartment *JSVMCompartment::Constructor(JSContext *cx,
                                              JS::CallArgs &args,
                                              JS::HandleObject obj)
{
    JS::RootedObject sharedObj(cx);
    bool defineDebugger = false;

    if (args.length() > 0) {
        if (!args[0].isObject()) {
            JS_ReportError(
                cx,
                "Compartment constructor, first parameter must be an object");
            return nullptr;
        } else {
            sharedObj = args[0].toObjectOrNull();
        }
    } else {
        sharedObj = JS_NewPlainObject(cx);
    }

    if (args.length() > 1) {
        defineDebugger = JS::ToBoolean(args[1]);
    }

    JSVMCompartment *cpt = new JSVMCompartment(cx, sharedObj, defineDebugger);

    if (JS_IsExceptionPending(cx)) {
        JS_ReportPendingException(cx);
        return nullptr;
    }

    return cpt;
}

JSVMCompartment::JSVMCompartment(JSContext *cx,
                                 JS::HandleObject obj,
                                 bool defineDebugger)
{
    JS::RootedObject mainGbl(cx, JS::CurrentGlobalOrNull(cx));
    if (!mainGbl) {
        JS_ReportError(cx, "Cannot get current global");
        return;
    }

    JS::RootedObject gbl(cx, NidiumJS::CreateJSGlobal(cx));
    if (!gbl) {
        JS_ReportError(cx, "Cannot create global for Debugger compartment");
        return;
    }

    m_Global = gbl;
    m_Obj    = obj;

    if (defineDebugger) {
        JSAutoCompartment ac(cx, m_Global);
        JS_DefineDebuggerObject(cx, gbl);
    }

    // Wrap & copy the properties of the given
    // object to the new compartment
    if (obj) {
        JS::Rooted<JS::IdVector> ida(cx, JS::IdVector(cx));
        JS_Enumerate(cx, obj, &ida);
        JS::RootedId id(cx);
        JS::RootedValue val(cx);

        for (size_t i = 0; i < ida.length(); i++) {
            id = ida[i];

            if (!JS_GetPropertyById(cx, obj, id, &val)) {
                continue;
            }

            {
                JSAutoCompartment ac(cx, m_Global);
                JS::RootedObject cGlobal(cx, m_Global);

                JS_WrapValue(cx, &val);

                JS_SetPropertyById(cx, cGlobal, id, val);
            }
        }
    }
}

JSVMCompartment::~JSVMCompartment()
{
}

bool JSVMCompartment::Getter(JSContext *cx,
                             JS::HandleObject obj,
                             JS::HandleId id,
                             JS::MutableHandleValue vp)
{
    JSVMCompartment *cpt = JSVMCompartment::GetInstance(obj);
    if (!cpt) {
        JS_ReportError(cx, "Illegal invocation");
        return false;
    }

    return cpt->get(cx, id, vp);
}

bool JSVMCompartment::Setter(JSContext *cx,
                             JS::HandleObject obj,
                             JS::HandleId id,
                             JS::MutableHandleValue vp,
                             JS::ObjectOpResult &result)
{
    JSVMCompartment *cpt = JSVMCompartment::GetInstance(obj);
    if (!cpt) {
        JS_ReportError(cx, "Illegal invocation");
        return false;
    }

    cpt->set(cx, id, vp, result);

    return true;
}

bool JSVMCompartment::get(JSContext *cx,
                          JS::HandleId id,
                          JS::MutableHandleValue vp)
{
    JS::RootedValue val(cx);
    JS::RootedObject gbl(cx, m_Global);
    JS::RootedObject obj(cx, m_Obj);

    JSAutoCompartment ac(cx, m_Global);

    if (!(JS_GetPropertyById(cx, gbl, id, &val))) {
        JS_ReportError(cx, "Failed to get property");
        return false;
    }

    JS_WrapValue(cx, &val);

    vp.set(val);

    return true;
}

bool JSVMCompartment::set(JSContext *cx,
                          JS::HandleId id,
                          JS::MutableHandleValue vp,
                          JS::ObjectOpResult &result)
{
    JS::RootedObject gbl(cx, m_Global);
    JS::RootedObject obj(cx, m_Obj);

    JS_WrapValue(cx, vp);

    if (!JS_SetPropertyById(cx, obj, id, vp)) {
        JS_ReportError(cx, "Failed to set property");
        return false;
    }

    {
        JSAutoCompartment ac(cx, gbl);
        if (!JS_SetPropertyById(cx, gbl, id, vp)) {
            JS_ReportError(cx, "Failed to set property");
            return false;
        }
    }

    result.succeed();

    return true;
}

bool JSVM::ParseCommonOptions(JSContext *cx,
                              JS::HandleValue optionsValue,
                              JSVMCompartment **cpt,
                              JS::MutableHandleObject scope,
                              JS::MutableHandleString filename,
                              int32_t *lineOffset)
{
    if (!optionsValue.isObject()) {
        JS_ReportError(cx, "run() second argument must be an object");
        return false;
    }


    JS::RootedObject options(cx, optionsValue.toObjectOrNull());

    NIDIUM_JS_INIT_OPT();

    NIDIUM_JS_GET_OPT_TYPE(options, "compartment", Object)
    {
        JS::RootedObject obj(cx, __curopt.toObjectOrNull());
        *cpt = JSVMCompartment::GetInstance(obj);
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "scope", Object)
    {
        scope.set(__curopt.toObjectOrNull());
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "filename", String)
    {
        filename.set(__curopt.toString());
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "lineOffset", Int32)
    {
        *lineOffset = __curopt.toInt32();
    }

    return true;
}

bool JSVM::ParseRunData(JSContext *cx,
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
            JS_ReportError(cx, "First argument is not Stream instance");
            return false;
        }
    } else {
        JS_ReportError(cx, "First argument must be a String or Stream");
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

JSClass *JSVMCompartment::GetJSClass()
{
    static JSClass jsclass
        = { "Compartment", JSCLASS_HAS_PRIVATE,
            nullptr, nullptr,
            JSVMCompartment::Getter, JSVMCompartment::Setter };

    return &jsclass;
}

void JSVMCompartment::RegisterObject(JSContext *cx, JS::HandleObject vm)
{
    JSVMCompartment::ExposeClass<0>(cx, "Compartment", 0,
                                    ClassMapper::kEmpty_ExposeFlag, vm);
}
// }}}

} // namespace Binding
} // namespace Nidium
