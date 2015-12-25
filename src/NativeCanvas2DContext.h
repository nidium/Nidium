#ifndef nativecanvas2dcontext_h__
#define nativecanvas2dcontext_h__

#include <stdint.h>

#include <NativeJSExposer.h>

#include "NativeJSImage.h"
#include "NativeCanvasContext.h"

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

class NativeCanvas2DContext : public NativeCanvasContext
{
    public:

        static JSClass *ImageData_jsclass;

        friend class NativeJSCanvas;

        bool setterDisabled;

        void clear(uint32_t color = 0x00000000) override;

        /*
            draw layer on top of "this"
        */

        void resetSkiaContext(uint32_t flags = 0);

        uint8_t *getPixels() override;
        uint32_t getTextureID() const;
        void flush();
        virtual void setSize(int width, int height, bool redraw = true) override;
        void translate(double x, double y);

        void getSize(int *width, int *height) const;

        uint32_t attachShader(const char *string);
        void detachShader();

        void setVertexDeformation(uint32_t vertex, float x, float y);

        NativeSkia *getSurface() const {
            return this->m_Skia;
        }

        void setScale(double x, double y, double px=1, double py=1);

        uint32_t createProgram(const char *data);

        void drawTexture(uint32_t textureID, uint32_t width,
            uint32_t height, uint32_t left, uint32_t top);

        static void registerObject(JSContext *cx);

        NativeCanvas2DContext(NativeCanvasHandler *handler,
            int width, int height, NativeUIInterface *ui, bool isGL = true);

        NativeCanvas2DContext(NativeCanvasHandler *handler,
            struct JSContext *cx, int width, int height, NativeUIInterface *ui);

        virtual ~NativeCanvas2DContext();
    private:
        NativeSkia *m_Skia;

        void initCopyTex();
        uint32_t compileCoopFragmentShader();
        char *genModifiedFragmentShader(const char *data);
        void drawTexToFBO(uint32_t textureID);
        void drawTexIDToFBO(uint32_t textureID, uint32_t width,
            uint32_t height, uint32_t left, uint32_t top, uint32_t fbo);

        uint32_t getSkiaTextureID(int *width = NULL, int *height = NULL);
        uint32_t getMainFBO();
};

class NativeCanvasPattern
{
    public:
        NativeJSImage *jsimg;

        enum PATTERN_MODE {
            PATTERN_REPEAT,
            PATTERN_NOREPEAT,
            PATTERN_REPEAT_X,
            PATTERN_REPEAT_Y,
            PATTERN_REPEAT_MIRROR
        } mode;

        NativeCanvasPattern(NativeJSImage *img, PATTERN_MODE repeat) :
            jsimg(img), mode(repeat) {
        };
};

#endif

