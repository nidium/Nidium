/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jscanvas2dcontext_h__
#define binding_jscanvas2dcontext_h__

#include <stdint.h>
#include <memory.h>


#include "Graphics/CanvasContext.h"
#include "Graphics/Gradient.h"

#include "Binding/JSImage.h"
#include "Binding/ClassMapper.h"

class SkCanvas;

namespace Nidium {
namespace Interface {
class UIInterface;
}
namespace Graphics {
struct Rect;
class SkiaContext;
class CanvasHandler;
}
namespace Binding {

/*
    Create a new 2D context using SkiaContext.
    A new JSObject is created with Canvas2DContext as private
    The class is auto destroyed if no reference is retained to the JSObject

    i.e.
    foo = new Canvas2Context();
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
    Canvas2DContextState()
        : m_CurrentShader(JS::UndefinedValue()),
          m_CurrentStrokeShader(JS::UndefinedValue()), m_Next(NULL)
    {
    }

    Canvas2DContextState(Canvas2DContextState *other)
        : m_CurrentShader(other->m_CurrentShader),
          m_CurrentStrokeShader(other->m_CurrentStrokeShader), m_Next(other)
    {
    }

    /* either pattern or gradient (mutual exlusive) */
    JS::Heap<JS::Value> m_CurrentShader;
    JS::Heap<JS::Value> m_CurrentStrokeShader;

    Canvas2DContextState *m_Next;
};
// }}}

// {{{ JSCanvasGLProgram
class AutoGLProgram
{
public:
    AutoGLProgram(int32_t program);
    ~AutoGLProgram();

private:
    int32_t m_Program;
    int32_t m_PreviousProgram;
};

class JSCanvasGLProgram : public ClassMapper<JSCanvasGLProgram>
{
public:
    JSCanvasGLProgram(JSContext *cx, int32_t program);
    static JSFunctionSpec *ListMethods();

protected:
    NIDIUM_DECL_JSCALL(getUniformLocation);
    NIDIUM_DECL_JSCALL(getActiveUniforms);
    NIDIUM_DECL_JSCALL(uniform1i);
    NIDIUM_DECL_JSCALL(uniform1f);
    NIDIUM_DECL_JSCALL(uniform1iv);
    NIDIUM_DECL_JSCALL(uniform2iv);
    NIDIUM_DECL_JSCALL(uniform3iv);
    NIDIUM_DECL_JSCALL(uniform4iv);
    NIDIUM_DECL_JSCALL(uniform1fv);
    NIDIUM_DECL_JSCALL(uniform2fv);
    NIDIUM_DECL_JSCALL(uniform3fv);
    NIDIUM_DECL_JSCALL(uniform4fv);
private:
    bool uniformXiv(JSContext *cx, JS::CallArgs &args, int nb);
    bool uniformXfv(JSContext *cx, JS::CallArgs &args, int nb);
    size_t m_Program;
};
// }}}

// {{{ Canvas2DContext
class Canvas2DContext : public ClassMapper<Canvas2DContext>,
                        public Graphics::CanvasContext
{
public:
    friend class JSCanvas;

    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();
    static Canvas2DContext *UnWrap(void *ptr);
    static void *Wrap(Canvas2DContext *obj);
    static void Delete(void *ptr);

    NIDIUM_DECL_JSTRACER();

    bool m_SetterDisabled;

    void clear(uint32_t color = 0x00000000) override;

    uint8_t *getPixels() override;
    uint32_t getTextureID() const override;
    void flush() override;
    virtual void setSize(float width, float height, bool redraw = true) override;

    void translate(double x, double y) override;

    void getSize(int *width, int *height) const override;

    uint32_t attachShader(const char *string);
    void detachShader();

    void setVertexDeformation(uint32_t vertex, float x, float y);

    Graphics::SkiaContext *getSkiaContext() const
    {
        return m_Skia;
    }

    void setScale(double x, double y, double px = 1, double py = 1) override;

    uint32_t createProgram(const char *data);

    void drawTexture(uint32_t textureID);


    Canvas2DContextState *getCurrentState() const
    {
        return m_CurrentState;
    }
    void pushNewState()
    {
        Canvas2DContextState *state
            = m_CurrentState ? new Canvas2DContextState(m_CurrentState)
                             : new Canvas2DContextState();

        m_CurrentState = state;
    }

    void popState()
    {
        /*
            Can't be stateless
        */
        if (!m_CurrentState->m_Next) {
            return;
        }

        Canvas2DContextState *tmp = m_CurrentState->m_Next;

        delete m_CurrentState;

        m_CurrentState = tmp;
    }

    JSObject *getJSInstance() override {
        return this->getJSObject();
    }

    static void RegisterObject(JSContext *cx);

    Canvas2DContext(Graphics::CanvasHandler *handler,
                    int width,
                    int height,
                    Interface::UIInterface *ui,
                    bool isGL = true);

    Canvas2DContext(Graphics::CanvasHandler *handler,
                    struct JSContext *cx,
                    int width,
                    int height,
                    Interface::UIInterface *ui);

    virtual ~Canvas2DContext();
protected:
    NIDIUM_DECL_JSCALL(breakText);
    NIDIUM_DECL_JSCALL(shadow);
    NIDIUM_DECL_JSCALL(onerror);
    NIDIUM_DECL_JSCALL(fillRect);
    NIDIUM_DECL_JSCALL(fillText);
    NIDIUM_DECL_JSCALL(strokeText);
    NIDIUM_DECL_JSCALL(strokeRect);
    NIDIUM_DECL_JSCALL(clearRect);
    NIDIUM_DECL_JSCALL(beginPath);
    NIDIUM_DECL_JSCALL(moveTo);
    NIDIUM_DECL_JSCALL(lineTo);
    NIDIUM_DECL_JSCALL(fill);
    NIDIUM_DECL_JSCALL(stroke);
    NIDIUM_DECL_JSCALL(closePath);
    NIDIUM_DECL_JSCALL(clip);
    NIDIUM_DECL_JSCALL(arc);
    NIDIUM_DECL_JSCALL(arcTo);
    NIDIUM_DECL_JSCALL(rect);
    NIDIUM_DECL_JSCALL(quadraticCurveTo);
    NIDIUM_DECL_JSCALL(bezierCurveTo);
    NIDIUM_DECL_JSCALL(rotate);
    NIDIUM_DECL_JSCALL(scale);
    NIDIUM_DECL_JSCALL(save);
    NIDIUM_DECL_JSCALL(restore);
    NIDIUM_DECL_JSCALL(translate);
    NIDIUM_DECL_JSCALL(transform);
    NIDIUM_DECL_JSCALL(iTransform);
    NIDIUM_DECL_JSCALL(setTransform);
    NIDIUM_DECL_JSCALL(createLinearGradient);
    NIDIUM_DECL_JSCALL(createRadialGradient);
    NIDIUM_DECL_JSCALL(createImageData);
    NIDIUM_DECL_JSCALL(createPattern);
    NIDIUM_DECL_JSCALL(putImageData);
    NIDIUM_DECL_JSCALL(getImageData);
    NIDIUM_DECL_JSCALL(drawImage);
    NIDIUM_DECL_JSCALL(measureText);
    NIDIUM_DECL_JSCALL(isPointInPath);
    NIDIUM_DECL_JSCALL(getPathBounds);
    NIDIUM_DECL_JSCALL(attachFragmentShader);
    NIDIUM_DECL_JSCALL(detachFragmentShader);
    NIDIUM_DECL_JSCALL(setVertexOffset);

    NIDIUM_DECL_JSGETTERSETTER(fillStyle);
    NIDIUM_DECL_JSGETTERSETTER(strokeStyle);
    NIDIUM_DECL_JSGETTERSETTER(lineWidth);
    NIDIUM_DECL_JSGETTERSETTER(miterLimit);
    NIDIUM_DECL_JSGETTERSETTER(globalAlpha);
    NIDIUM_DECL_JSGETTERSETTER(globalCompositeOperation);
    NIDIUM_DECL_JSGETTERSETTER(fontSize);
    NIDIUM_DECL_JSGETTERSETTER(textAlign);
    NIDIUM_DECL_JSGETTERSETTER(textBaseline);
    NIDIUM_DECL_JSGETTERSETTER(fontFamily);
    NIDIUM_DECL_JSGETTERSETTER(fontStyle);
    NIDIUM_DECL_JSGETTERSETTER(fontSkew);
    NIDIUM_DECL_JSGETTERSETTER(fontFile);
    NIDIUM_DECL_JSGETTERSETTER(lineCap);
    NIDIUM_DECL_JSGETTERSETTER(lineJoin);
    NIDIUM_DECL_JSGETTERSETTER(shadowOffsetX);
    NIDIUM_DECL_JSGETTERSETTER(shadowOffsetY);
    NIDIUM_DECL_JSGETTERSETTER(shadowBlur);
    NIDIUM_DECL_JSGETTERSETTER(shadowColor);
    NIDIUM_DECL_JSGETTERSETTER(imageSmoothingEnabled);
    NIDIUM_DECL_JSGETTER(width);
    NIDIUM_DECL_JSGETTER(height);

private:
    Graphics::SkiaContext *m_Skia;
    Canvas2DContextState *m_CurrentState;
    bool m_CanBeRecycled = true;

    void initCopyTex();
    uint32_t compileCoopFragmentShader(const char *glslversion);
    char *genModifiedFragmentShader(const char *data, const char *glslversion);
};
// }}}

// {{{ JSGradient
class JSGradient : public ClassMapper<JSGradient>
{
public:
    JSGradient(Graphics::Gradient *gradient) :
        m_Gradient(gradient){}

    virtual ~JSGradient() {
        delete m_Gradient;
    }

    static JSFunctionSpec *ListMethods() {
        static JSFunctionSpec funcs[] = {
            CLASSMAPPER_FN(JSGradient, addColorStop, 2),
            JS_FS_END
        };

        return funcs;
    }

    inline Graphics::Gradient *getGradient() const {
        return m_Gradient;
    }
protected:
    NIDIUM_DECL_JSCALL(addColorStop);
private:
    Graphics::Gradient *m_Gradient;
};
// }}}

// {{{ CanvasPattern
class JSCanvasPattern : public ClassMapper<JSCanvasPattern>
{
public:
    JSImage *m_JsImg;

    enum PATTERN_MODE
    {
        PATTERN_REPEAT,
        PATTERN_NOREPEAT,
        PATTERN_REPEAT_X,
        PATTERN_REPEAT_Y,
        PATTERN_REPEAT_MIRROR
    } m_Mode;

    JSCanvasPattern(JSImage *img, PATTERN_MODE repeat)
        : m_JsImg(img), m_Mode(repeat){};

    virtual ~JSCanvasPattern(){};

};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
