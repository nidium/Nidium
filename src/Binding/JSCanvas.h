/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jscanvas_h__
#define binding_jscanvas_h__

#include "Binding/ClassMapper.h"
#include "Core/Messages.h"

namespace Nidium {
namespace Interface {
class UIInterface;
}
namespace Graphics {
class CanvasHandler;
}
namespace Binding {


#define JSCANVAS_GENERIC_PERCENT_PROP_GETTER(name, prop) \
    bool JSCanvas::JSGetter_##prop(JSContext *cx, JS::MutableHandleValue vp) \
    { \
        float name = m_CanvasHandler->getProp##name(); \
        if (isnan(name)) { \
            vp.setUndefined(); \
            return true; \
        } \
        if (m_CanvasHandler->p_##name.isPercentageValue()) { \
            char ret[8]; \
            int len = sprintf(ret, "%.2f%%", name); \
            vp.setString(JS_NewStringCopyN(cx, ret, len)); \
            return true; \
        }   \
        vp.setNumber(m_CanvasHandler->getProp##name()); \
        return true; \
    }

#define JSCANVAS_GENERIC_PERCENT_PROP_SETTER(name, prop) \
    bool JSCanvas::JSSetter_##prop(JSContext *cx, JS::MutableHandleValue vp) \
    { \
        double dval; \
        if (vp.isNullOrUndefined()) { \
            m_CanvasHandler->setProp##name(NAN); \
            return true; \
        } \
        m_CanvasHandler->p_##name.setIsPercentageValue(JSUtils::ValuePercent(cx, vp, &dval)); \
        m_CanvasHandler->setProp##name((float)dval); \
        return true; \
    }

class JSCanvas : public ClassMapperWithEvents<JSCanvas>, public Core::Messages
{
public:
    virtual void onMessage(const Core::SharedMessages::Message &msg) override;
    virtual void onMessageLost(const Core::SharedMessages::Message &msg) override;
    static void RegisterObject(JSContext *cx);
    static JSObject *GenerateJSObject(JSContext *cx,
                                      int width,
                                      int height,
                                      Graphics::CanvasHandler **out);

    JSCanvas(Graphics::CanvasHandler *handler);
    virtual ~JSCanvas();

    Graphics::CanvasHandler *getHandler() const
    {
        return m_CanvasHandler;
    }


    static JSCanvas *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);

    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();

    NIDIUM_DECL_JSTRACER();
protected:

    NIDIUM_DECL_JSCALL(requestPaint);
    NIDIUM_DECL_JSCALL(getContext);
    NIDIUM_DECL_JSCALL(setContext);
    NIDIUM_DECL_JSCALL(addSubCanvas);
    NIDIUM_DECL_JSCALL(insertBefore);
    NIDIUM_DECL_JSCALL(insertAfter);
    NIDIUM_DECL_JSCALL(removeFromParent);
    NIDIUM_DECL_JSCALL(bringToFront);
    NIDIUM_DECL_JSCALL(sendToBack);
    NIDIUM_DECL_JSCALL(getParent);
    NIDIUM_DECL_JSCALL(getFirstChild);
    NIDIUM_DECL_JSCALL(getLastChild);
    NIDIUM_DECL_JSCALL(getNextSibling);
    NIDIUM_DECL_JSCALL(getPrevSibling);
    NIDIUM_DECL_JSCALL(getChildren);
    NIDIUM_DECL_JSCALL(getVisibleRect);
    NIDIUM_DECL_JSCALL(setCoordinates);
    NIDIUM_DECL_JSCALL(show);
    NIDIUM_DECL_JSCALL(hide);
    NIDIUM_DECL_JSCALL(setSize);
    NIDIUM_DECL_JSCALL(clear);
    NIDIUM_DECL_JSCALL(setZoom);
    NIDIUM_DECL_JSCALL(setScale);
    NIDIUM_DECL_JSCALL(getDimensions);

    NIDIUM_DECL_JSGETTERSETTER(opacity);
    NIDIUM_DECL_JSGETTERSETTER(overflow);
    NIDIUM_DECL_JSGETTERSETTER(scrollable);
    NIDIUM_DECL_JSGETTERSETTER(scrollableX);
    NIDIUM_DECL_JSGETTERSETTER(scrollableY);
    NIDIUM_DECL_JSGETTERSETTER(scrollLeft);
    NIDIUM_DECL_JSGETTERSETTER(scrollTop);
    NIDIUM_DECL_JSGETTERSETTER(allowNegativeScroll);
    NIDIUM_DECL_JSGETTERSETTER(width);
    NIDIUM_DECL_JSGETTERSETTER(height);
    NIDIUM_DECL_JSGETTERSETTER(maxWidth);
    NIDIUM_DECL_JSGETTERSETTER(maxHeight);
    NIDIUM_DECL_JSGETTERSETTER(minWidth);
    NIDIUM_DECL_JSGETTERSETTER(minHeight);
    NIDIUM_DECL_JSGETTERSETTER(position);
    NIDIUM_DECL_JSGETTERSETTER(display);
    NIDIUM_DECL_JSGETTERSETTER(top);
    NIDIUM_DECL_JSGETTERSETTER(left);
    NIDIUM_DECL_JSGETTERSETTER(right);
    NIDIUM_DECL_JSGETTERSETTER(bottom);
    NIDIUM_DECL_JSGETTERSETTER(visible);
    NIDIUM_DECL_JSGETTERSETTER(id);
    NIDIUM_DECL_JSGETTERSETTER(marginLeft);
    NIDIUM_DECL_JSGETTERSETTER(marginRight);
    NIDIUM_DECL_JSGETTERSETTER(marginTop);
    NIDIUM_DECL_JSGETTERSETTER(marginBottom);

    NIDIUM_DECL_JSGETTERSETTER(paddingLeft);
    NIDIUM_DECL_JSGETTERSETTER(paddingRight);
    NIDIUM_DECL_JSGETTERSETTER(paddingTop);
    NIDIUM_DECL_JSGETTERSETTER(paddingBottom);

    NIDIUM_DECL_JSGETTERSETTER(coating);
    NIDIUM_DECL_JSGETTERSETTER(cursor);

    /* Flexbox container */
    NIDIUM_DECL_JSGETTERSETTER(flexDirection);
    NIDIUM_DECL_JSGETTERSETTER(flexWrap);
    NIDIUM_DECL_JSGETTERSETTER(justifyContent);
    NIDIUM_DECL_JSGETTERSETTER(alignItems);
    NIDIUM_DECL_JSGETTERSETTER(alignContent);

    /* Flexbox items */
    NIDIUM_DECL_JSGETTERSETTER(flexGrow);
    NIDIUM_DECL_JSGETTERSETTER(flexShrink);
    NIDIUM_DECL_JSGETTERSETTER(flexBasis);
    NIDIUM_DECL_JSGETTERSETTER(alignSelf);
    NIDIUM_DECL_JSGETTERSETTER(aspectRatio);

    NIDIUM_DECL_JSGETTER(idx);
    NIDIUM_DECL_JSGETTER(clientWidth);
    NIDIUM_DECL_JSGETTER(clientHeight);
    NIDIUM_DECL_JSGETTER(clientTop);
    NIDIUM_DECL_JSGETTER(clientLeft);
    NIDIUM_DECL_JSGETTER(innerWidth);
    NIDIUM_DECL_JSGETTER(innerHeight);
    NIDIUM_DECL_JSGETTER(__visible);
    NIDIUM_DECL_JSGETTER(__top);
    NIDIUM_DECL_JSGETTER(__left);
    NIDIUM_DECL_JSGETTER(__fixed);
    NIDIUM_DECL_JSGETTER(__outofbound);
    NIDIUM_DECL_JSGETTER(ctx);


private:
    Graphics::CanvasHandler *m_CanvasHandler;

    bool fireInputEvent(int event, JS::HandleObject ev, const Core::SharedMessages::Message &msg);
};

} // namespace Binding
} // namespace Nidium

#endif
