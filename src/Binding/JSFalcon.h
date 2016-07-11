/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef bindings_jsfalcon_h__
#define bindings_jsfalcon_h__

#include <Binding/JSCanvas.h>
#include <Binding/JSUtils.h>
#include <vector>

#define ELEMENT_STYLE_MAX_PROPERTIES 8192

namespace Nidium {
namespace Graphics {
    class CanvasHandler;
    class SkiaContext;
}
namespace Binding {

class JSElement;
class Canvas2DContext;

// {{{ NSSValue
class NSSValue {
  public:
    typedef enum type {
      kType_Int,
      kType_Double,
      kType_Bool,
      kType_Pointer,
      kType_String,
      kType_JSVal,
      kType_Unset
    } Type;

    NSSValue(int value);
    NSSValue(double value);
    NSSValue(bool value);
    NSSValue(void *value);
    NSSValue(char *value);

    NSSValue(JSContext *cx, JS::HandleValue val);

    NSSValue();

    ~NSSValue();

    bool isNative();
    bool isSet();
    Type getType();

    bool toNative(Type type);
    JS::Value jsval(JSContext *cx);
 
    int getInt();
    double getDouble();
    bool getBool();
    void *getPointer();
    const char *getString();

    int m_IntValue = 0;
    double m_DoubleValue = 0;
    bool m_BoolValue = false;
    void *m_PointerValue = nullptr;
    char *m_StringValue = nullptr;
    JS::PersistentRootedValue *m_JSValue = nullptr;

    bool m_IsNative = true;
    Type m_Type = kType_Unset;
    JSContext *m_JSContext;
};
// }}}

// {{{ NSSProperty
class NSSProperty {
  friend class JSNSS;
  public:
    // TODO : Replace Setter & DrawCallback with heritage
    typedef void (*Setter)(JSElement *el, NSSValue *val);
    typedef void (*DrawCallback)(JSElement *el, Graphics::CanvasHandler *handler,
        Graphics::SkiaContext *skiaCtx);

    typedef enum _Flag {
        kFlag_None          = 0,
        kFlag_CanvasProxy   = 1 << 0, // Property is proxied to canvas
        kFlag_Native        = 1 << 1, // Property is native
        kFlag_Deferrable    = 1 << 2, // Property can be deffered (not used yet)
        kFlag_Setter        = 1 << 3, // Property has a setter
        kFlag_PreDrawCbk    = 1 << 4, // Property has a pre draw callback
        kFlag_PostDrawCbk   = 1 << 5, // Property has a post draw callback
        kFlag_ReflowCbk     = 1 << 6, // Property has a reflow callback
        kFlag_Reflow        = 1 << 7, // Property update triggers reflow
        kFlag_Redraw        = 1 << 8  // Property update triggers redraw
    } Flag;

    class JSOptions {
      friend class NSSProperty;
      public:
        JSOptions(JSContext *cx) : m_Cx(cx) {};
        ~JSOptions () {
            NidiumJS *njs = NidiumJS::GetObject(m_Cx);

            if (setter.isObject()) njs->unrootObject(setter.toObjectOrNull());
            if (preDraw.isObject()) njs->unrootObject(preDraw.toObjectOrNull());
            if (postDraw.isObject()) njs->unrootObject(postDraw.toObjectOrNull());
            if (onReflow.isObject()) njs->unrootObject(onReflow.toObjectOrNull());
        }

        void setSetter(JS::HandleValue val) {
printf("adding setter\n");
            this->set(&this->setter, val);
        }

        void setPreDraw(JS::HandleValue val) {
            this->set(&this->preDraw, val);
        }

        void setPostDraw(JS::HandleValue val) {
            this->set(&this->postDraw, val);
        }

        void setOnReflow(JS::HandleValue val) {
            this->set(&this->onReflow, val);
        }
      private:
        void set(JS::Heap<JS::Value> *src, JS::HandleValue val) {
printf("isobject=%d callable=%d!\n", val.isObject(), JS_ObjectIsCallable(m_Cx, val.toObjectOrNull()));
            if (val.isObject() && JS_ObjectIsCallable(m_Cx, val.toObjectOrNull())) {
                NidiumJS *njs = NidiumJS::GetObject(m_Cx);
printf("setter added !\n");

                njs->rootObjectUntilShutdown(val.toObjectOrNull());
                src->set(val);
            }
        }

        JSContext *m_Cx;

        JS::Heap<JS::Value> setter;
        JS::Heap<JS::Value> onReflow;
        JS::Heap<JS::Value> preDraw;
        JS::Heap<JS::Value> postDraw;
    };

    NSSProperty(JSContext *cx, const char *name, 
        JS::HandleValue value, JSOptions *options);
    NSSProperty(const char *name, DrawCallback cbk, NSSValue *value, 
        NSSValue::Type type, unsigned int flags = 0, void *priv = nullptr);

    ~NSSProperty();

    bool hasFlag(unsigned int flag) {
        return m_Flags & flag;
    }
    
    void addFlag(unsigned int flag) {
        m_Flags |= flag;
    }

    NSSValue *getValue() {
        return m_NativeValue;
    }

    const char *getName() {
        return m_Name;
    }

    NSSValue::Type getType() {
        return m_Type;
    }

    template <typename T>
    T getPrivate() {
        return static_cast<T>(m_Private);
    }

    int getID() {
        return m_ID;
    }

    bool isNative() {
        return this->hasFlag(kFlag_Native);
    }

    void callSetter(JSElement *el, NSSValue *val);
    void callReflow(uint32_t reason, JSElement *el);
    void callPreDraw(JSElement *el);
    void callPostDraw(JSElement *el);
  private:
    char *m_Name;

    JSContext *m_JSContext = nullptr;
    JSOptions *m_JSOptions = nullptr;

    DrawCallback m_NativeCallback = nullptr;
    NSSValue *m_NativeValue = nullptr;
    
    NSSValue::Type m_Type = NSSValue::kType_Unset;
    void *m_Private; 
    unsigned int m_Flags = 0;
    int m_ID = -1;
};
// }}}

// {{{ JSNSS
class JSNSS 
{
  public:
    JSNSS(JSContext *cx, JS::HandleObject proto);

    ~JSNSS();

    NSSProperty *addProperty(JSContext *cx, const char *name, 
        JS::HandleValue fn, JS::HandleValue defaultValue);

    NSSProperty *addProperty(const char *name, NSSProperty::DrawCallback fn,
        NSSValue *defaultValue, NSSValue::Type type, unsigned int flags = 0, 
        void *priv = nullptr);

    bool addProperty(NSSProperty *property);

    NSSProperty *getProperty(const char *name);

    void invalidate(JSElement *el);

    void onFrame();

    JSObject *getJSObject() {
        return m_JSObject;
    }

  private:
    JSContext *m_JSContext;
    JS::Heap<JSObject *> m_StyleProto;
    JS::Heap<JSObject *> m_JSObject;

    NSSProperty *m_Properties[ELEMENT_STYLE_MAX_PROPERTIES];
    int m_PropertyIdx = 0;

    std::vector<JSElement *> m_PendingElements;

    void initNativeProperties();
};
// }}}

// {{{ JSElement
class JSElement: public JSExposer<JSElement> 
{
  public:
    typedef enum _InvalidateReason {
        kInvalidate_None            = 0,
        kInvalidate_Added           = 1 << 0,
        kInvalidate_Removed         = 1 << 1,
        kInvalidate_Reflow          = 1 << 2,
        kInvalidate_ChildAdded      = 1 << 3,
        kInvalidate_ChildRemoved    = 1 << 4,
        kInvalidate_ChildReflow     = 1 << 5,
        kInvalidate_Redraw          = 1 << 7
    } InvalidateReason;

    JSElement(JS::HandleObject obj, JSContext *cx);
    ~JSElement();

    void invalidate(int32_t reason);
    bool isReady();

    NSSValue *getStyle(int index) {
printf("get style @ %d\n", index);
        if (index < m_StyleProperties.capacity()) {
printf("returning index %d\n", index);
if (!m_StyleProperties[index].value) {
printf("no value\n");
    return nullptr;
}
printf("%p/%p/%p\n", &m_StyleProperties[index], m_StyleProperties[index].name , m_StyleProperties[index].value);
            return m_StyleProperties[index].value;
        }

        return nullptr;
    }
    NSSValue *getStyle(const char *name) {
printf("getStyle, loop len=%zu\n", m_StyleProperties.size());
printf("@%p\n", this);
        for (auto &p : m_StyleProperties) {
printf("testing %p/%p\n", p.name, name);
            if (strcmp(p.name, name) == 0) {
                return p.value;
            }
        }

        return nullptr;
    }   

    void setStyle(NSSProperty *property, NSSValue *val);
    void setStyle(NSSProperty *property, JS::HandleValue value);

    void setParent(JSElement *el);
    JSElement *getParent() {
        return m_Parent;
    }

    void setCanvas(JSCanvas *canvas);
    JSCanvas *getCanvas() {
        return m_Canvas;
    }

    Graphics::SkiaContext *getSkiaContext() {
        return m_SkiaContext;
    }

    JSObject *getStylePrivate() {
        return m_StylePrivate;
    }

    Canvas2DContext *getCanvasContext();

    void onFrame();
    void draw();

    static void RegisterObject(JSContext *cx);
  private:
    typedef struct StyleTuple {
        const char *name = nullptr;
        NSSValue *value = nullptr;
        StyleTuple(const char *name, NSSValue *value)  {}
        StyleTuple()  {}
    } _StyleTuple;
    JSCanvas *m_Canvas = nullptr;
    JSElement *m_Parent = nullptr;
    Canvas2DContext *m_CanvasContext = nullptr;
    Graphics::SkiaContext *m_SkiaContext = nullptr;
    Graphics::CanvasHandler *m_CanvasHandler = nullptr;

    std::vector<StyleTuple> m_StyleProperties; 
    std::vector<NSSProperty *> m_StylePreDraw;;
    std::vector<NSSProperty *> m_StylePostDraw;
    std::vector<NSSProperty *> m_StyleReflow;
    JS::Heap<JSObject *> m_StylePrivate;

    bool m_IsReady = false;
    int32_t m_InvalidateReason = kInvalidate_None;
};
// }}}

// {{{ JSFalcon
class JSFalcon
{
  public:
    JSFalcon(JSContext *cx);

    ~JSFalcon();
  
    JSObject *getObject() {
        return m_JSObject;
    }

    JSNSS *getNSS() {
        return m_NSS;
    }

    void onFrame() {
        m_NSS->onFrame();
    }

    static JSFalcon *RegisterObject(JSContext *cx);
  private:
    JS::Heap<JSObject *> m_JSObject;
    JSNSS *m_NSS;
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif

