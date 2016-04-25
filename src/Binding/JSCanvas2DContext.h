#ifndef binding_jscanvas2dcontext_h__
#define binding_jscanvas2dcontext_h__

#include <stdint.h>

#include <Binding/JSExposer.h>

#include "Graphics/CanvasContext.h"
#include "Binding/JSImage.h"

class SkCanvas;

namespace Nidium {
    namespace Interface {
        class NativeUIInterface;
    }
    namespace Graphics {
        struct NativeRect;
        class NativeSkia;
        class CanvasHandler;
    }
namespace Binding {

/*
    Create a new 2D context using Graphics::NativeSkia.
    A new JSObject is created with Canvas2DContext as private
    The class is auto destroyed if no reference is retained to the JSObject

    i.e.
    foo = new NativeCanvas2DObject();
    foo->m_JsObj Must either be JS_AddObjectRoot'ed or given to the userland

    Don't manually delete the instance.
*/
class JSCanvas;
class JSImage;

/*
    JSAPI tracer is told to trace JS::Heap stored in this chain of state
*/
// {{{ Canvas2DContextState
struct Canvas2DContextState
{
    Canvas2DContextState() :
        m_CurrentShader(JS::UndefinedValue()),
        m_CurrentStrokeShader(JS::UndefinedValue()),
        m_Next(NULL) {
    }

    Canvas2DContextState(Canvas2DContextState *other) :
        m_CurrentShader(other->m_CurrentShader),
        m_CurrentStrokeShader(other->m_CurrentStrokeShader),
        m_Next(other) {
    }

    /* either pattern or gradient (mutual exlusive) */
    JS::Heap<JS::Value> m_CurrentShader;
    JS::Heap<JS::Value> m_CurrentStrokeShader;

    Canvas2DContextState *m_Next;
};
// }}}

// {{{ Canvas2DContext
class Canvas2DContext : public Graphics::CanvasContext
{
    public:

        static JSClass *jsclass;

        friend class JSCanvas;

        bool m_SetterDisabled;

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

        Graphics::NativeSkia *getSurface() const {
            return m_Skia;
        }

        void setScale(double x, double y, double px=1, double py=1);

        uint32_t createProgram(const char *data);

        void drawTexture(uint32_t textureID, uint32_t width,
            uint32_t height, uint32_t left, uint32_t top);


        Canvas2DContextState *getCurrentState() const {
            return m_CurrentState;
        }
        void pushNewState() {
            Canvas2DContextState *state =
                m_CurrentState ? new Canvas2DContextState(m_CurrentState)
                               : new Canvas2DContextState();

            m_CurrentState = state;
        }

        void popState() {
            /*
                can't be stateless
            */
            if (!m_CurrentState->m_Next) {
                return;
            }

            Canvas2DContextState *tmp = m_CurrentState->m_Next;

            delete m_CurrentState;

            m_CurrentState = tmp;
        }

        static void RegisterObject(JSContext *cx);

        Canvas2DContext(Graphics::CanvasHandler *handler,
            int width, int height, Interface::NativeUIInterface *ui, bool isGL = true);

        Canvas2DContext(Graphics::CanvasHandler *handler,
            struct JSContext *cx, int width, int height, Interface::NativeUIInterface *ui);

        virtual ~Canvas2DContext();
    private:
        Graphics::NativeSkia *m_Skia;
        Canvas2DContextState *m_CurrentState;


        void initCopyTex();
        uint32_t compileCoopFragmentShader();
        char *genModifiedFragmentShader(const char *data);
        void drawTexToFBO(uint32_t textureID);
        void drawTexIDToFBO(uint32_t textureID, uint32_t width,
            uint32_t height, uint32_t left, uint32_t top, uint32_t fbo);

        uint32_t getSkiaTextureID(int *width = NULL, int *height = NULL);
        uint32_t getMainFBO();
};
// }}}

// {{{ CanvasPattern
class CanvasPattern
{
    public:
        JSImage *m_JsImg;

        enum PATTERN_MODE {
            PATTERN_REPEAT,
            PATTERN_NOREPEAT,
            PATTERN_REPEAT_X,
            PATTERN_REPEAT_Y,
            PATTERN_REPEAT_MIRROR
        } m_Mode;

        CanvasPattern(JSImage *img, PATTERN_MODE repeat) :
            m_JsImg(img), m_Mode(repeat) {
        };
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif

