#ifndef nativecanvas2dcontext_h__
#define nativecanvas2dcontext_h__

/*
    Create a new 2D context using NativeSkia.
    A new JSObject is created with NativeCanvas2DContext as private
    The class is auto destroyed if no reference is retained to the JSObject

    i.e.
    foo = new NativeCanvas2DObject();
    foo->jsobj Must either be JS_AddObjectRoot'ed or given to the userland

    Don't manually delete the instance.
*/

class NativeSkia;

class NativeCanvas2DContext
{
    public:
        NativeCanvas2DContext(struct JSContext *cx, int width, int height);
        ~NativeCanvas2DContext();
        friend class NativeJSCanvas;
        struct JSObject *jsobj;
        struct JSContext *jscx;
        NativeSkia *skia;
    private:
        
};

#endif
