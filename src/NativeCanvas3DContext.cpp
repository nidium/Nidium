#include "NativeCanvas3DContext.h"
#include "NativeJSCanvas.h"

#include "NativeMacros.h"

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#define CANVASCTX_GETTER(obj) ((class NativeCanvas3DContext *)JS_GetPrivate(obj))

void Canvas3DContext_finalize(JSFreeOp *fop, JSObject *obj);

extern JSClass WebGLRenderingContext_class;
extern JSConstDoubleSpec WebGLRenderingContext_const;


void Canvas3DContext_finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvas3DContext *canvasctx = CANVASCTX_GETTER(obj);
    if (canvasctx != NULL) {
        delete canvasctx;
    }
}

NativeCanvas3DContext::~NativeCanvas3DContext()
{
}

NativeCanvas3DContext::NativeCanvas3DContext(NativeCanvasHandler *handler,
    JSContext *cx, int width, int height, NativeUIInterface *ui) :
    NativeCanvasContext(handler)
{
    m_Mode = CONTEXT_WEBGL;

    jsobj = JS_NewObject(cx, &WebGLRenderingContext_class, NULL, NULL);

    JS_DefineConstDoubles(cx, jsobj, &WebGLRenderingContext_const);

    jscx  = cx;

    JS_SetPrivate(jsobj, this);
}

static JSBool native_Canvas3DContext_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return JS_FALSE;
}


void NativeCanvas3DContext::translate(double x, double y)
{

}

void NativeCanvas3DContext::setSize(int width, int height, bool redraw)
{

}

void NativeCanvas3DContext::setScale(double x, double y, double px, double py)
{

}

void NativeCanvas3DContext::clear(uint32_t color)
{

}

void NativeCanvas3DContext::flush()
{

}

/* Returns the size in device pixel */
void NativeCanvas3DContext::getSize(int *width, int *height) const
{

}

void NativeCanvas3DContext::composeWith(NativeCanvas2DContext *layer,
    double left, double top, double opacity,
    double zoom, const NativeRect *rclip)
{

}