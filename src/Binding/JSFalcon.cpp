#include <Binding/JSModules.h>
#include <Graphics/CanvasHandler.h>
#include <Graphics/CanvasContext.h>
#include <Graphics/SkiaContext.h>
#include <Binding/JSCanvas2DContext.h>
#include <Frontend/Context.h>
#include "JSFalcon.h"
#include <Binding/JSCanvasProperties.h>
#include <Interface/UIInterface.h>
#include <Binding/JSCanvas.h>

using Nidium::Graphics::CanvasContext;
using Nidium::Graphics::CanvasHandler;
using Nidium::Graphics::SkiaContext;

// TODO : 
// - Inherited values

namespace Nidium {
namespace Binding {

// {{{ Preamble
// {{{ JSFalcon
#define GET_FALCON(cx) Frontend::Context::GetObject<Frontend::Context>(NidiumJS::GetObject(cx))->getJSFalcon()

static void Falcon_Finalize(JSFreeOp *fop, JSObject *obj);
JSClass JSFalcon_class = {
    "Falcon", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Falcon_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};
// }}}
// {{{ JSNSS 
static void JSNSS_Finalize(JSFreeOp *fop, JSObject *obj);
static bool JSNSS_addProperty(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass JSNSS_class = {
    "NSS", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JSNSS_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};
static JSFunctionSpec JSNSS_funcs[] = {
    JS_FN("addProperty", JSNSS_addProperty, 2, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// {{{ ProxyStyle (definition of styles proxied to canvas)
extern bool nidium_canvas_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
struct ProxyStyleDefinition {
    const char *name;
    uint8_t id;
    NSSValue *defaultValue;
    unsigned int flags;

    ProxyStyleDefinition(const char *name, uint8_t id, NSSValue *value, unsigned int flags = 0)
        : name(name), id(id), defaultValue(value), 
          flags(NSSProperty::kFlag_Native|NSSProperty::kFlag_CanvasProxy) {}
};

static ProxyStyleDefinition ProxyStyle[] = {
    {"width", CANVAS_PROP_WIDTH, new NSSValue(1)},
    {"height", CANVAS_PROP_HEIGHT, new NSSValue(1)},
    {"position", CANVAS_PROP_POSITION, new NSSValue("relative")},
    {"top", CANVAS_PROP_TOP, new NSSValue()},
    {"left", CANVAS_PROP_LEFT, new NSSValue()},
    {"right", CANVAS_PROP_RIGHT, new NSSValue()},
    {"bottom", CANVAS_PROP_BOTTOM, new NSSValue()},
    {"visible", CANVAS_PROP_VISIBLE, new NSSValue(true)},
    {"coating", CANVAS_PROP_COATING, new NSSValue(0)},
    {"opacity", CANVAS_PROP_OPACITY, new NSSValue(1)},
    {"overflow", CANVAS_PROP_OVERFLOW, new NSSValue(true)},
    {"scrollTop", CANVAS_PROP_SCROLLTOP, new NSSValue(0)},
    {"scrollLeft", CANVAS_PROP_SCROLLLEFT, new NSSValue(0)},
    {"allowNegativeScroll", CANVAS_PROP_ALLOWNEGATIVESCROLL, new NSSValue(false)},
    {"staticLeft", CANVAS_PROP_STATICLEFT, new NSSValue(true)},
    {"staticRight", CANVAS_PROP_STATICRIGHT, new NSSValue(false)},
    {"staticTop", CANVAS_PROP_STATICTOP, new NSSValue(true)},
    {"staticBottom", CANVAS_PROP_STATICBOTTOM, new NSSValue(false)},
    {"minWidth", CANVAS_PROP_MINWIDTH, new NSSValue(1)},
    {"minHeight", CANVAS_PROP_MINHEIGHT, new NSSValue(1)},
    {"maxWidth", CANVAS_PROP_MAXWIDTH, new NSSValue()},
    {"maxHeight", CANVAS_PROP_MAXHEIGHT, new NSSValue()},
    {"fluidHeight", CANVAS_PROP_FLUIDHEIGHT, new NSSValue(false)},
    {"fuildWidth", CANVAS_PROP_FLUIDWIDTH, new NSSValue(false)},
    {"marginLeft", CANVAS_PROP_MARGINLEFT, new NSSValue(0)},
    {"marginRight", CANVAS_PROP_MARGINRIGHT, new NSSValue(0)},
    {"marginTop", CANVAS_PROP_MARGINTOP, new NSSValue(0)},
    {"marginBottom", CANVAS_PROP_MARGINBOTTOM, new NSSValue(0)},
    {"cursor", CANVAS_PROP_CURSOR, new NSSValue("default")},
#if 0
    // Still relevant ? 
    {"__visible", CANVAS_PROP___VISIBLE},
    {"__top", CANVAS_PROP___TOP},
    {"__left", CANVAS_PROP___LEFT},
    {"__fixed", CANVAS_PROP___FIXED},
    {"__outofbound", CANVAS_PROP___OUTOFBOUND},

    // Getter only
    {"clientLeft", CANVAS_PROP_CLIENTLEFT},
    {"clientTop", CANVAS_PROP_CLIENTTOP},
    {"clientWidth", CANVAS_PROP_CLIENTWIDTH},
    {"clientHeight", CANVAS_PROP_CLIENTHEIGHT},
    {"contentWith", CANVAS_PROP_CONTENTWIDTH},
    {"innerWith", CANVAS_PROP_INNERWIDTH},
    {"contentHeight", CANVAS_PROP_CONTENTHEIGHT},
    {"innerHeight", CANVAS_PROP_INNERHEIGHT},
#endif
    {nullptr, 0, nullptr}
};
// }}}
// {{{ Native Styles
typedef enum NativeStyle {
    kStyle_None = -1,
    kStyle_BackgroundColor = 0,
    kStyle_End,
} _NativeStyle;

typedef struct NativeStyleDefinition {
    NativeStyle styleID;
    const char *name;
    NSSValue::Type type;
    NSSValue *defaultValue;
    unsigned int flags;

    NativeStyleDefinition(NativeStyle styleID, const char *name, 
            NSSValue::Type type, NSSValue *defaultValue, unsigned int flags = 0)
        : styleID(styleID), name(name), type(type), defaultValue(defaultValue), 
          flags(NSSProperty::kFlag_Native) {}
    
} _NativeStyleDefinition;

static NativeStyleDefinition NativeStyle[] = {
    {kStyle_BackgroundColor, "backgroundColor", NSSValue::kType_String, new NSSValue(), NSSProperty::kFlag_Redraw},
    {kStyle_None, nullptr, NSSValue::kType_Unset, nullptr}
};
// }}}
// }}}
// {{{ JSElement
extern JSClass Canvas_class;
extern JSClass Canvas2DContext_class;
static bool nidium_Element_invalidate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_Element_setParent(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_Element_getParent(JSContext *cx, unsigned argc, JS::Value *vp);

static void Element_Finalize(JSFreeOp *fop, JSObject *obj);

JSClass JSElement_class = {
    "Element", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Element_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec JSElement_funcs[] = {
    JS_FN("invalidate", nidium_Element_invalidate, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setParent", nidium_Element_setParent, 1, NIDIUM_JS_FNPROPS),
    JS_FN("getParent", nidium_Element_getParent, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

typedef struct JSInvalidateReason {
    JSElement::InvalidateReason flag;
    const char *name;
} _JSInvalidateReason;

static JSInvalidateReason JSElement_InvalidatReason[] = {
    {JSElement::kInvalidate_Added,          "ELEMENT_ADDED"},
    {JSElement::kInvalidate_Removed,        "ELEMENT_REMOVED"},
    {JSElement::kInvalidate_Reflow,         "ELEMENT_REFLOW"},
    {JSElement::kInvalidate_ChildAdded,     "CHILDREN_ADDED"},
    {JSElement::kInvalidate_ChildRemoved,   "CHILDREN_REMOVED"},
    {JSElement::kInvalidate_ChildReflow,    "CHILDREN_REFLOW"},
    {JSElement::kInvalidate_None,           nullptr}
};

template<>
JSClass *JSExposer<JSElement>::jsclass = &JSElement_class;
// }}}
// {{{ JSElementStyle
static bool JSElementStyle_setter(JSContext *cx, 
    JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp);
static bool JSElementStyle_getter(JSContext *cx, 
    JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);

JSClass JSElementStyle_class = {
    "ElementStyle", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JSElementStyle_setter,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};
// }}}

// }}}

// {{{ Bindings
// {{{ JSFalcon
static void Falcon_Finalize(JSFreeOp *fop, JSObject *obj) 
{
    JSFalcon *falcon = static_cast<JSFalcon *>(JS_GetPrivate(obj));
    if (falcon) {
        delete falcon;
    }
}
// }}}
// {{{ JSElement
static bool nidium_Element_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CONSTRUCTOR_PROLOGUE()

    // Element
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &JSElement_class, args));
    JSElement *el = new JSElement(ret, cx);

    JS_SetPrivate(ret, el);

    args.rval().setObjectOrNull(ret);

    // Canvas
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedValue canvasValue(cx);
    JS::RootedObject canvas(cx);
    JS::AutoValueArray<2> canvasArgs(cx);

    if (!JS_GetProperty(cx, global, "Canvas", &canvasValue) || 
            !canvasValue.isObject()) {
        JS_ReportError(cx, "Invalid context, no Canvas object found");
        return false;
    }

    canvasArgs[0].setInt32(64);
    canvasArgs[1].setInt32(64);

    canvas.set(canvasValue.toObjectOrNull());

    JS::RootedObject canvasInstance(cx, JS_New(cx, canvas, canvasArgs));
    JS::RootedValue canvasInstanceValue(cx);
    if (!canvasInstance) {
        JS_ReportError(cx, "Couldn't create Canvas");
        return false;
    }

    canvasInstanceValue.setObjectOrNull(canvasInstance);
    JS_DefineProperty(cx, ret, "canvas", canvasInstanceValue,
            JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY, nullptr, nullptr);

    JSCanvas *jscanvas = static_cast<JSCanvas *>(
            JS_GetInstancePrivate(cx, canvasInstance, &Canvas_class, nullptr));
    if (!jscanvas) {
        JS_ReportError(cx, "Canvas should be an instance of Canvas");
        return false;
    }

    el->setCanvas(jscanvas);

    // Style
    JS::RootedObject style(cx, 
        JS_NewObject(cx, &JSElementStyle_class, JS::NullPtr(), JS::NullPtr()));
    
    JS_SetPrivate(style, el);

    JS_DefineProperty(cx, 
        ret, "style", style, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    return true;
}

static void Element_Finalize(JSFreeOp *fop, JSObject *obj) 
{
    JSElement *el= static_cast<JSElement *>(JS_GetPrivate(obj));
    if (el) {
        delete el;
    }
}

bool nidium_Element_invalidate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSElement, &JSElement_class)

    NIDIUM_JS_CHECK_ARGS("invalidate", 1);

    if (!args[0].isInt32()) {
        JS_ReportError(cx, "invalidate expect an integer");
        return false;
    }

    // JS bitwise operators operate on 32 bits signed integers.
    // Keep the same norme when calling invalidate().
    CppObj->invalidate(args[0].toInt32());

    return true;
}

bool nidium_Element_setParent(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSElement, &JSElement_class)

    NIDIUM_JS_CHECK_ARGS("setParent", 1);
    
    if (!args[0].isObject()) {
        JS_ReportError(cx, "Parent should be an object");
        return false;
    }

    JS::RootedObject obj(cx, args[0].toObjectOrNull());
    JSElement *el = static_cast<JSElement *>(
            JS_GetInstancePrivate(cx, obj, &JSElement_class, nullptr));
    if (!el) {
        JS_ReportError(cx, "Parent should be an instance of Element");
        return false;
    }

    CppObj->setParent(el);

    return true;
}

bool nidium_Element_getParent(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSElement, &JSElement_class)

    JSElement *parent = CppObj->getParent();
    if (parent) {
        JS::RootedObject obj(cx, parent->getJSObject());

        args.rval().setObjectOrNull(obj);
    } else {
        args.rval().setUndefined();
    }

    return true;
}
// }}}
// {{{ JSNSS 
static void JSNSS_Finalize(JSFreeOp *fop, JSObject *obj) 
{
    JSNSS *nss = static_cast<JSNSS*>(JS_GetPrivate(obj));
    if (nss) {
        delete nss;
    }
}

bool JSNSS_addProperty(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSNSS, &JSNSS_class)

    NIDIUM_JS_CHECK_ARGS("addProperty", 2);

    if (!args[0].isString()) {
        JS_ReportError(cx, "First argument \"name\" must be a string");
        return false;
    }

    if (!args[1].isObjectOrNull()) {
        JS_ReportError(cx, "Second argument \"options\" must be an object");
        return false;
    }

    JSAutoByteString name(cx, args[0].toString());

    if (CppObj->getProperty(name.ptr())) {
        JS_ReportError(cx, "The property \"%s\" is already defined", name.ptr());
        return false;
    }

    NSSProperty::JSOptions *nssOptions = new NSSProperty::JSOptions(cx);
    JS::RootedObject opt(cx, args[1].toObjectOrNull());

    NIDIUM_JS_INIT_OPT();

    JS::RootedValue defaultValue(cx);
    NIDIUM_JS_GET_OPT(opt, "default") {
        defaultValue = __curopt;
    }

    NIDIUM_JS_GET_OPT(opt, "onSet") {
        nssOptions->setSetter(__curopt);
    }

    NIDIUM_JS_GET_OPT(opt, "onReflow") {
        nssOptions->setOnReflow(__curopt);
    }

    NIDIUM_JS_GET_OPT(opt, "preDraw") {
        nssOptions->setPreDraw(__curopt);
    }

    NIDIUM_JS_GET_OPT(opt, "postDraw") {
        nssOptions->setPostDraw(__curopt);
    }

    NSSProperty *prop = new NSSProperty(cx, name.ptr(), defaultValue, nssOptions);
    if (!CppObj->addProperty(prop)) {
        delete prop;
        JS_ReportError(cx, "Failed to add property \"%s\"", name.ptr());
        return false;
    }

    return true;
}
// }}}
// {{{ JSElementStyle
bool JSElementStyle_setter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
    // If the property is not defined, return immediately.
    bool defined = false;
    if (!JS_HasPropertyById(cx, obj, id, &defined) || !defined) {
        return true;
    }

    JSElement *el = static_cast<JSElement *>(JS_GetPrivate(obj));
    if (!el || !el->getCanvas()) {
        JS_ReportError(cx, "Unexpected error");
        return false;
    }

    JSAutoByteString name(cx, JSID_TO_STRING(id));
    printf("[StyleSetter] Setting property name=%s ", name.ptr());


    NSSProperty *prop = (GET_FALCON(cx))->getNSS()->getProperty(name.ptr());
    if (!prop || !prop->hasFlag(NSSProperty::kFlag_CanvasProxy | 
                                NSSProperty::kFlag_Native | 
                                NSSProperty::kFlag_Setter | 
                                NSSProperty::kFlag_PreDrawCbk | 
                                NSSProperty::kFlag_PostDrawCbk | 
                                NSSProperty::kFlag_Reflow | 
                                NSSProperty::kFlag_Redraw)) {
        printf(" => Unknown property, or no flag\n");
        return true;
    }
    

    if (prop->hasFlag(NSSProperty::kFlag_CanvasProxy)) {
        printf("=> CanvasProxy\n");
        JS::RootedObject canvasObj(cx, el->getCanvas()->getJSObject());
        JS::RootedValue val(cx, vp);

        if (!canvasObj) return true;

        nidium_canvas_prop_set(cx, canvasObj, prop->getPrivate<ProxyStyleDefinition *>()->id, false, &val);
    } else {
        printf("=> calling setStyle()\n");
        el->setStyle(prop, vp);
    }

    return true;
}

bool JSElementStyle_getter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
    return true;
}
// }}}
// }}}

// {{{ Implementation
// {{{ NSSValue
NSSValue::NSSValue(int value) 
    : m_IntValue(value), m_Type(kType_Int) {};

NSSValue::NSSValue(double value) 
    : m_DoubleValue(value), m_Type(kType_Double) {};

NSSValue::NSSValue(bool value)
    : m_BoolValue(value), m_Type(kType_Bool) {};

NSSValue::NSSValue(void *value)
    : m_PointerValue(value), m_Type(kType_Pointer) {};

NSSValue::NSSValue(char *value)
    : m_StringValue(strdup(value)), m_Type(kType_String) {};

NSSValue::NSSValue()
    : m_Type(kType_Unset) {};

NSSValue::NSSValue(JSContext *cx, JS::HandleValue val) 
    : m_JSValue(new JS::PersistentRootedValue(cx, val)), 
      m_IsNative(false), m_Type(kType_JSVal), m_JSContext(cx)
{
}

NSSValue::~NSSValue()
{

    delete m_JSValue;
    free(m_StringValue);
}

bool NSSValue::isNative()
{
    return m_IsNative;
}

bool NSSValue::isSet() 
{
    return m_Type == kType_Unset;
}

NSSValue::Type NSSValue::getType() 
{
    return m_Type;
}

int NSSValue::getInt() 
{
    return m_IntValue;
}

double NSSValue::getDouble() 
{
    return m_DoubleValue;
}

bool NSSValue::getBool() 
{
    return m_BoolValue;
}

void *NSSValue::getPointer() 
{
    return m_PointerValue;
}

const char *NSSValue::getString()
{
    return m_StringValue;
}

bool NSSValue::toNative(NSSValue::Type type)
{
    if (m_IsNative) return true;
    if (!m_JSContext || !m_JSValue) {
        return false;
    }

    JS::RootedValue val(m_JSContext, m_JSValue->get());

    switch(type) {
        case kType_Int:
            if (!JS::ToInt32(m_JSContext, val, &m_IntValue)) {
                return false;
            }
            break;
        case kType_Bool:
            m_BoolValue = JS::ToBoolean(val);
            break;
        case kType_Double:
            if (!JS::ToNumber(m_JSContext, val, &m_DoubleValue)) {
                return false;
            }
            break;
        case kType_String: {
                JSAutoByteString str(m_JSContext, 
                    JS::ToString(m_JSContext, val));
                m_StringValue = strdup(str.ptr());
                break;
            }
        case kType_Pointer:
        case kType_Unset:
        case kType_JSVal:
            // Unsupported ?
            return false;
            break;
    }

    m_IsNative = true;

    delete m_JSValue;
    m_JSValue = nullptr;

    return true;
}

JS::Value NSSValue::jsval(JSContext *cx)
{
    JS::RootedValue val(cx);

    switch(m_Type) {
        case kType_Int:
            val.setInt32(this->getInt());
            break;
        case kType_Bool:
            val.setBoolean(this->getBool());
            break;
        case kType_Double:
            val.setDouble(this->getDouble());
            break;
        case kType_String:
            JSUtils::StrToJsval(cx, this->getString(), 
                strlen(this->getString()), &val, "utf8");
            break;
        case kType_Pointer:
            // Unsupported ?
            break;
        case kType_JSVal:
            val = m_JSValue->get();
        case kType_Unset:
            break;
    }

    return val;
}
// }}}
// {{{ NSSProperty
NSSProperty::NSSProperty(JSContext *cx, const char *name, JS::HandleValue value, 
        NSSProperty::JSOptions *options = nullptr)
    : m_Name(strdup(name)), m_JSContext(cx), m_JSOptions(options),
      m_NativeValue(new NSSValue(cx, value)), m_Type(NSSValue::kType_JSVal)
{
    if (!options) return;

    if (!options->setter.isUndefined()) m_Flags |= kFlag_Setter;
    if (!options->preDraw.isUndefined()) m_Flags |= kFlag_PreDrawCbk;
    if (!options->postDraw.isUndefined()) m_Flags |= kFlag_PostDrawCbk;
}

NSSProperty::NSSProperty(const char *name, DrawCallback cbk, NSSValue *val, 
    NSSValue::Type type, unsigned int flags, void *priv) 
    : m_Name(strdup(name)), m_NativeCallback(cbk), m_NativeValue(val), 
      m_Type(type), m_Private(priv), m_Flags(flags|NSSProperty::kFlag_Native)
{
}

#define CALL_JS(FUN, ARGS) \
    JS::RootedValue rval(m_JSContext); \
    JS::RootedValue funValue(m_JSContext, FUN); \
    JS::RootedFunction fun(m_JSContext, JS_ValueToFunction(m_JSContext, funValue)); \
    JS::RootedObject obj(m_JSContext, el->getJSObject());\
    JS::RootedObject stylePrivate(m_JSContext, el->getStylePrivate());\
    ARGS[ARGS.length() - 1].setObjectOrNull(stylePrivate); \
    JS_CallFunction(m_JSContext, obj, fun, ARGS, &rval); \
    if (JS_IsExceptionPending(m_JSContext)) { \
        if (!JS_ReportPendingException(m_JSContext)) { \
            JS_ClearPendingException(m_JSContext); \
        } \
    }

void NSSProperty::callSetter(JSElement *el, NSSValue *val)
{
    if (!this->hasFlag(NSSProperty::kFlag_Setter)) return;

    if (this->isNative()) {
        // Not supported yet
    } else {
        JS::AutoValueArray<3> args(m_JSContext);

        JSUtils::StrToJsval(m_JSContext, m_Name, strlen(m_Name),  args[0], "utf8");
        args[1].set(val->jsval(m_JSContext));

        CALL_JS(m_JSOptions->setter, args);
    }
}

void NSSProperty::callPreDraw(JSElement *el)
{
    if (!this->hasFlag(kFlag_PreDrawCbk)) return;

    if (this->isNative()) {
        // Not supported yet
    } else {
        JS::AutoValueArray<3> args(m_JSContext);
        JS::RootedObject canvasCtx(m_JSContext, el->getCanvasContext()->m_JsObj);

        args[0].setObjectOrNull(el->getCanvas()->getJSObject());
        args[1].setObjectOrNull(canvasCtx);

        CALL_JS(m_JSOptions->preDraw, args);
    }
}

void NSSProperty::callPostDraw(JSElement *el)
{
    if (!this->hasFlag(kFlag_PostDrawCbk)) return;

    if (this->isNative()) {
        // Not supported yet
    } else {
        JS::AutoValueArray<3> args(m_JSContext);
        JS::RootedObject canvasCtx(m_JSContext, el->getCanvasContext()->m_JsObj);

        args[0].setObjectOrNull(el->getCanvas()->getJSObject());
        args[1].setObjectOrNull(canvasCtx);

        CALL_JS(m_JSOptions->postDraw, args);
    }
}

void NSSProperty::callReflow(uint32_t reason, JSElement *el)
{
    if (!this->hasFlag(kFlag_ReflowCbk)) return;

    if (this->isNative()) {
        // Not supported yet
    } else {
        JS::AutoValueArray<4> args(m_JSContext);
        JS::RootedObject canvasCtx(m_JSContext, el->getCanvasContext()->m_JsObj);

        args[0].setInt32(reason);
        args[1].setObjectOrNull(el->getCanvas()->getJSObject());
        args[2].setObjectOrNull(canvasCtx);

        CALL_JS(m_JSOptions->onReflow, args);
    }
}

NSSProperty::~NSSProperty()
{
    delete m_NativeValue;
    delete m_JSOptions;
    free(m_Name);
}
// }}}
// {{{ JSFalcon
JSFalcon::JSFalcon(JSContext *cx) 
{
    JS::RootedObject obj(cx, JS_NewObject(cx, &JSFalcon_class, JS::NullPtr(), JS::NullPtr()));
    m_JSObject = obj;
    JS_SetPrivate(obj, this);

    // Element
    JS::RootedObject elementObj(cx, 
        JS_InitClass(cx, obj, JS::NullPtr(), &JSElement_class, 
            nidium_Element_constructor, 0, nullptr, JSElement_funcs, nullptr, nullptr));
    JS::RootedObject elementProto(cx);

    JS_GetPrototype(cx, elementObj, &elementProto);

    // Element.prototype.InvalidateReason
    JS::RootedObject invalidateReason(cx, JS_NewObject(cx, 
        nullptr, JS::NullPtr(), JS::NullPtr()));
    JSInvalidateReason *reason = &JSElement_InvalidatReason[0]; 
    JS::RootedValue invalidateReasonValue(cx);

    invalidateReasonValue.setObjectOrNull(invalidateReason);
    
    while (reason->name != nullptr) {
        JS::RootedValue val(cx);

        val.setInt32(reason->flag);

        JS_DefineProperty(cx, invalidateReason, reason->name, val,
            JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY, nullptr, nullptr);
        
        reason++;
    }

    JS_DefineProperty(cx, elementProto, "InvalidateReason", invalidateReasonValue,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY, nullptr, nullptr);

    // NSS
    JS::RootedObject styleObject(cx, 
        JS_InitClass(cx, obj, JS::NullPtr(), &JSElementStyle_class, 
            nullptr, 0, nullptr, nullptr, nullptr, nullptr));
    JS::RootedObject styleProto(cx);

    JS_GetPrototype(cx, styleObject, &styleProto);

    m_NSS = new JSNSS(cx, styleProto);

    JS::RootedObject nssObject(cx, m_NSS->getJSObject());
    JS_DefineProperty(cx, obj, "NSS", nssObject,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY, nullptr, nullptr);
}

JSFalcon::~JSFalcon()
{
}
// }}}
// {{{ JSNSS
JSNSS::JSNSS(JSContext *cx, JS::HandleObject proto) 
    : m_JSContext(cx), m_StyleProto(proto) {

    JS::RootedObject nssObject(cx, JS_NewObject(cx, 
        &JSNSS_class, JS::NullPtr(), JS::NullPtr()));

    m_PendingElements.reserve(4096);

    m_JSObject = nssObject;

    JS_DefineFunctions(cx, nssObject, JSNSS_funcs);

    JS_SetPrivate(nssObject, this);

    this->initNativeProperties();
}

JSNSS::~JSNSS()
{
    for (int i = 0; i < ELEMENT_STYLE_MAX_PROPERTIES && i < m_PropertyIdx; i++) {
        delete m_Properties[i];
    }
}

void JSNSS::initNativeProperties()
{
    NativeStyleDefinition *style = &NativeStyle[0];
    while (style->name != nullptr) {
        NSSProperty *prop = new NSSProperty(style->name, nullptr, style->defaultValue, 
            style->type, style->flags);

        prop->m_ID = style->styleID;

        this->addProperty(prop);

        style++;
    }

    ProxyStyleDefinition *proxyStyle = &ProxyStyle[0];
    while (proxyStyle->name != nullptr) {
        this->addProperty(proxyStyle->name, nullptr, 
            proxyStyle->defaultValue,  NSSValue::kType_JSVal, 
            proxyStyle->flags, proxyStyle);

        proxyStyle++;
    }
}

NSSProperty *JSNSS::addProperty(const char *name, NSSProperty::DrawCallback fn,
        NSSValue *defaultValue, NSSValue::Type type, unsigned int flags, void *priv)
{
    NSSProperty *prop = new NSSProperty(name, fn, defaultValue, type, flags, priv);
    if (!this->addProperty(prop)) {
        return nullptr;
    }

    return prop;
}

bool JSNSS::addProperty(NSSProperty *property)
{
    if (m_PropertyIdx >= ELEMENT_STYLE_MAX_PROPERTIES ||
            property->m_ID >= ELEMENT_STYLE_MAX_PROPERTIES) {
        return false;
    }

    JS::RootedValue val(m_JSContext, property->getValue()->jsval(m_JSContext));
    JS::RootedObject proto(m_JSContext, m_StyleProto);

    JS_DefineProperty(m_JSContext, proto, property->getName(), val,
            JSPROP_PERMANENT | JSPROP_ENUMERATE, nullptr, nullptr);

    if (property->m_ID >= 0) {
        // Special case for built-in native properties. They are assigned 
        // to predifined position so we can query them by index.
        m_Properties[property->m_ID] = property;
    } else {
        property->m_ID = m_PropertyIdx;
        m_Properties[m_PropertyIdx] = property;
    }

    m_PropertyIdx++;

    return true;
}

NSSProperty *JSNSS::getProperty(const char *name)
{
    for (int i = 0; i < ELEMENT_STYLE_MAX_PROPERTIES && i < m_PropertyIdx; i++) {
        if (strcmp(m_Properties[i]->getName(), name) == 0) {
            return m_Properties[i];
        }
    }

    return nullptr;
}

void JSNSS::invalidate(JSElement *el) {
    if (!el->isReady()) return;

    printf("[NSS] invalidated\n");
    m_PendingElements.push_back(el);
}

void JSNSS::onFrame() 
{
    for (auto p : m_PendingElements) {
        p->onFrame();
    }

    m_PendingElements.clear();
}
// }}}
// {{{ JSElement
JSElement::JSElement(JS::HandleObject obj, JSContext *cx) 
    : JSExposer<JSElement>(obj, cx)
{
    JS::RootedObject stylePrivate(cx,
        JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    m_StylePrivate = stylePrivate;

    m_StyleProperties.resize(kStyle_End);

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(stylePrivate);
    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(m_JSObject);
}

JSElement::~JSElement()
{
    for (auto &p : m_StyleProperties) {
        delete p.value;
    }

    NidiumJS::GetObject(m_Cx)->unrootObject(m_StylePrivate);
    NidiumJS::GetObject(m_Cx)->rootObjectUntilShutdown(this->getJSObject());
}

bool JSElement::isReady() {
    bool canvasReady = this->getCanvasContext() != nullptr;
    bool isInUse = m_Canvas->getHandler() && m_Canvas->getHandler()->getParent();

    return canvasReady && isInUse;
}

void JSElement::setParent(JSElement *el)
{
    m_Parent = el;

    this->invalidate(kInvalidate_Added);
    m_Parent->invalidate(kInvalidate_ChildAdded);

    NidiumJS::GetObject(m_Cx)->unrootObject(m_JSObject);
}

void JSElement::setCanvas(JSCanvas *canvas)
{
    m_Canvas = canvas;
}

Canvas2DContext *JSElement::getCanvasContext()
{
    if (m_CanvasContext) return m_CanvasContext;

    if (!m_Canvas) {
        return nullptr;
    }

    m_CanvasHandler = m_Canvas->getHandler();
    if (!m_CanvasHandler) {
        return nullptr;
    }

    CanvasContext *ctx = m_CanvasHandler->getContext();
    if (!ctx) {
        JS::RootedObject jscanvas(m_Cx, this->getCanvas()->getJSObject());
        JS::RootedValue rval(m_Cx);
        JS::AutoValueArray<1> args(m_Cx);

        JSUtils::StrToJsval(m_Cx, "2d", 2, args[0], "utf8");

        if (!JS_CallFunctionName(m_Cx, jscanvas, "getContext", args, &rval) ||
                !JS_InstanceOf(m_Cx, jscanvas, &Canvas2DContext_class, nullptr)) {
            return nullptr;
        }
    } 

    if (ctx->getContextType() != CanvasContext::CONTEXT_2D) {
        return nullptr;
    }

    m_CanvasContext = static_cast<Canvas2DContext *>(ctx);

    m_SkiaContext = m_CanvasContext->getSurface();
    if (!m_SkiaContext) {
        m_CanvasContext = nullptr;
        return nullptr;
    }

    return m_CanvasContext;
}

void JSElement::draw()
{
    SkiaContext *ctx = m_SkiaContext;
    CanvasHandler *handler = m_CanvasHandler;

    NSSValue *backgroundColor = this->getStyle(kStyle_BackgroundColor);  

    if (backgroundColor && backgroundColor->getString()) {
        ctx->setFillColor(backgroundColor->getString());
        ctx->drawRect(0, 0, handler->getWidth(), handler->getHeight(), 0);
    }
}

void JSElement::onFrame()
{
    printf("[JSElement] onFrame %p!\n", this);

    if (m_InvalidateReason & (
        kInvalidate_Added | kInvalidate_Removed |
        kInvalidate_Reflow | kInvalidate_ChildAdded |
        kInvalidate_ChildRemoved | kInvalidate_ChildReflow)) {

        for (auto p : m_StylePostDraw) {
            p->callReflow(m_InvalidateReason, this);
        }
        
    }

    if (m_InvalidateReason & (kInvalidate_Redraw|kInvalidate_Reflow)) {
        for (auto p : m_StylePreDraw) {
            p->callPreDraw(this);
        }

        this->draw();

        for (auto p : m_StylePostDraw) {
            p->callPostDraw(this);
        }
    }

    m_InvalidateReason = kInvalidate_None;
}

void JSElement::setStyle(NSSProperty *prop, JS::HandleValue value)
{
    this->setStyle(prop, new NSSValue(m_Cx, value));
}

void JSElement::setStyle(NSSProperty *prop, NSSValue *value)
{
    NSSValue::Type propType = prop->getType();

    printf("[SetStyle] called : ");
    printf("native prop=%d value type=%d native value=%d\n", prop->isNative(), value->getType(), value->isNative());

    if (prop->isNative() && propType != NSSValue::kType_JSVal &&
            !value->isNative()) {
        if (!value->toNative(propType)) {
            printf("[SetStyle] couldn't cast to native\n");
            delete value;
            return;
        }

        if (prop->getID() != -1) {
            // Built-in native properties are stored 
            // at a specific index for a fast lookup
            m_StyleProperties[prop->getID()].name = prop->getName();
            m_StyleProperties[prop->getID()].value = value;
        } else {
            bool updated = false;
            for (auto &p : m_StyleProperties) {
                if (strcmp(p.name, prop->getName()) == 0) {
                    delete p.value;
                    p.value = value;
                    updated = true;
                    break;
                }
            }

            if (!updated) {
                m_StyleProperties.push_back(StyleTuple(prop->getName(), value));
            }
        }

    }

    prop->callSetter(this, value);

    if (prop->hasFlag(NSSProperty::kFlag_PreDrawCbk)) {
        m_StylePreDraw.push_back(prop);
    }

    if (prop->hasFlag(NSSProperty::kFlag_PostDrawCbk)) {
        m_StylePostDraw.push_back(prop);
    }

    unsigned int invalidateReason = NSSProperty::kFlag_None;
    if (prop->hasFlag(NSSProperty::kFlag_Redraw)) {
        invalidateReason |= NSSProperty::kFlag_Redraw;
    }

    if (prop->hasFlag(NSSProperty::kFlag_Reflow)) {
        invalidateReason |= NSSProperty::kFlag_Reflow;
    }

    if (invalidateReason != NSSProperty::kFlag_None) {
        this->invalidate(invalidateReason);
    }
}

void JSElement::invalidate(int32_t reason)
{
    printf("[JSElement] invalidate called\n");
    m_InvalidateReason |= reason;

    (GET_FALCON(m_Cx))->getNSS()->invalidate(this);
/*
    JS::RootedValue val(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedObject style(m_Cx);
    JSNSS *nss = (GET_FALCON(m_Cx))->getNSS();

    JS_GetProperty(m_Cx, obj, "style", &val);

    style = val.toObjectOrNull();

    JS::AutoIdVector props(m_Cx);
    printf("invalidate\n");

    if (!js::GetPropertyNames(m_Cx, style, JSITER_ENUMERATE, &props)) {
        return;
    }

    for (auto &p : props) {
        JS::RootedValue styleValue(m_Cx);
        JS::RootedId id(m_Cx, p);
        JSAutoByteString name(m_Cx, JSID_TO_STRING(id));
        printf("got %s\n", name.ptr());

        if (!JS_GetPropertyById(m_Cx, style, id, &styleValue)) {
            continue;
        }

        if (styleValue.isUndefined()) {
            continue;
        }

        JSAutoByteString str(m_Cx, 
            JS::ToString(m_Cx, styleValue));

        printf("value is %s\n", str.ptr());
        
        nss->setStyle(name.ptr(), styleValue, this);
    }
*/
}
// }}}
// }}}

// {{{ Registration
static JSObject *registerCallback(JSContext *cx) 
{
    JSFalcon *falcon = GET_FALCON(cx);

    return falcon->getObject();
}

JSFalcon *JSFalcon::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("NativeFalcon", registerCallback);
    return new JSFalcon(cx);
}
// }}}

} // namespace Binding
} // namespace Nidium

