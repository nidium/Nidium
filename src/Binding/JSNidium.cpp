#include "Binding/JSNidium.h"

#include <SystemInterface.h>

namespace Nidium {
namespace Binding {

// {{{ Preamble
static void Native_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Native_class = {
    "native", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Native_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *JSNidium::jsclass = &Native_class;
static bool nidium_nidium_language(JSContext *cx, unsigned argc, jsval *vp);

template<>
JSClass *JSExposer<JSNidium>::jsclass = &Native_class;


static JSFunctionSpec Native_funcs[] = {
    JS_FN("language", nidium_nidium_language, 0, 0),
    JS_FS_END
};

static bool nidium_nidium_language(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    Nidium::Interface::NativeSystemInterface* interface = Nidium::Interface::NativeSystemInterface::GetInstance();
    const char *clang = interface->getLanguage();

    args.rval().setString(JS_NewStringCopyZ(cx, clang));

    return true;

}

void Native_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSNidium *jnative = JSNidium::GetObject(obj);

    if (jnative != NULL) {
        delete jnative;
    }
}
// }}}

// {{{ Registration
void JSNidium::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject NativeObj(cx, JS_DefineObject(cx, global,
        JSNidium::GetJSObjectName(), &Native_class , nullptr, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    JSNidium *jnative = new JSNidium(NativeObj, cx);
    JS_DefineFunctions(cx, NativeObj, Native_funcs);
    JS_SetPrivate(NativeObj, jnative);

    NidiumJS::GetObject(cx)->jsobjects.set(JSNidium::GetJSObjectName(), NativeObj);

    //JS::RootedObject titleBar(cx, JSCanvas::GenerateJSObject(cx, width, 35));
    //((CanvasHandler *)JS_GetPrivate(canvas))->translate(0, 35);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    //NJS->rootHandler->addChild((CanvasHandler *)JS_GetPrivate(titleBar));

    /*JS::RootedValue titleVal(cx, OBJECT_TO_JSVAL(titleBar));
    JS_DefineProperty(cx, NativeObj, "titleBar", titleVal, JSPROP_READONLY | JSPROP_PERMANENT);*/
}
// }}}

} // namespace Nidium
} // namespace Binding

