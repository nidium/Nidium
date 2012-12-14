#ifndef nativecanvas2dcontext_h__
#define nativecanvas2dcontext_h__

class NativeSkia;

class NativeCanvas2DContext
{
    public:
        NativeCanvas2DContext(struct JSContext *cx, NativeSkia *skia);
        ~NativeCanvas2DContext();
        friend class NativeJSCanvas;
        struct JSObject *jsobj;
    private:
        struct JSContext *jscx;
};

#endif
