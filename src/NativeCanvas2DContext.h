#ifndef nativecanvas2dcontext_h__
#define nativecanvas2dcontext_h__

#include <stdint.h>
#include "NativeJSExposer.h"
#include "NativeJSImage.h"

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
struct NativeRect;
class SkCanvas;

class NativeCanvas2DContext : public NativeJSExposer
{
    public:

        friend class NativeJSCanvas;

        class JSObject *jsobj;
        struct JSContext *jscx;
        NativeSkia *skia;
        bool setterDisabled;

        struct {
            uint32_t program;
            uint32_t texture;
            uint32_t fbo;
            SkCanvas *copy;
        } gl;

        void clear(uint32_t color);

        /*
            draw layer on top of "this"
        */

        void resetGLContext();
        void composeWith(NativeCanvas2DContext *layer, double left,
            double top, double opacity, const NativeRect *clip);
        void flush();
        void setSize(int width, int height);
        void translate(double x, double y);
        
        uint32_t attachShader(const char *string);
        bool hasShader() const {
            return (gl.program != 0);
        }
        uint32_t getProgram() const {
            return gl.program;
        }

        uint32_t createProgram(const char *data);
        uint32_t compileShader(const char *data, int type);

        static void registerObject(JSContext *cx);
        NativeCanvas2DContext(int width, int height);
        NativeCanvas2DContext(struct JSContext *cx, int width, int height);
        ~NativeCanvas2DContext();
    private:
        void initCopyTex(uint32_t textureID);
        void drawTexToFBO(uint32_t textureID);
        uint32_t getSkiaTextureID();
};

class NativeCanvasPattern
{
    public:
        NativeJSImage *jsimg;

        enum PATTERN_MODE {
            PATTERN_REPEAT,
            PATTERN_NOREPEAT
        } mode;

        NativeCanvasPattern(NativeJSImage *img, PATTERN_MODE repeat) :
            jsimg(img), mode(repeat) {
        };

};

#endif
