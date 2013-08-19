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
class NativeCanvasHandler;

class NativeCanvas2DContext : public NativeJSExposer<NativeCanvas2DContext>
{
    public:

        friend class NativeJSCanvas;

        class JSObject *jsobj;
        struct JSContext *jscx;
        NativeSkia *skia;
        bool setterDisabled;
        bool commonDraw;

        struct {
            uint32_t program;
            uint32_t texture;
            uint32_t fbo;
            uint32_t textureWidth;
            uint32_t textureHeight;
            uint32_t vertexBuffer;
            uint32_t indexBuffer;
        } gl;

        struct {
            uint32_t uniformOpacity;
            uint32_t uniformResolution;
        } shader;

        void clear(uint32_t color);

        /*
            draw layer on top of "this"
        */

        void resetGLContext();
        void composeWith(NativeCanvas2DContext *layer, double left,
            double top, double opacity, double zoom,
            const NativeRect *clip);

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

        NativeCanvasHandler *getHandler() const {
            return this->handler;
        }

        NativeSkia *getSurface() const {
            return this->skia;
        }

        uint32_t createProgram(const char *data);
        uint32_t compileShader(const char *data, int type);

        static void registerObject(JSContext *cx);
        NativeCanvas2DContext(NativeCanvasHandler *handler, int width, int height, bool isGL = true);
        NativeCanvas2DContext(NativeCanvasHandler *handler, struct JSContext *cx, int width, int height);
        ~NativeCanvas2DContext();
    private:
        void initCopyTex();
        void drawTexToFBO(uint32_t textureID);
        void drawTexIDToFBO(uint32_t textureID, uint32_t width,
            uint32_t height, uint32_t left, uint32_t top, uint32_t fbo);
        uint32_t getSkiaTextureID(int *width = NULL, int *height = NULL);
        uint32_t getMainFBO();
        void setupShader(float opacity);
        NativeCanvasHandler *handler;
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
