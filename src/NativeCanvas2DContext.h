#ifndef nativecanvas2dcontext_h__
#define nativecanvas2dcontext_h__

#include <stdint.h>

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

        friend class NativeJSCanvas;

        struct JSObject *jsobj;
        struct JSContext *jscx;
        NativeSkia *skia;

        void clear(uint32_t color);

        /*
            draw layer on top of "this"
        */
        void composeWith(NativeCanvas2DContext *layer, double left, double top);
        void flush();

        NativeCanvas2DContext(int width, int height);
        NativeCanvas2DContext(struct JSContext *cx, int width, int height);
        ~NativeCanvas2DContext();
    private:
        
};

#endif
