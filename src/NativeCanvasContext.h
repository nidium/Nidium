#ifndef nativecanvascontext_h__
#define nativecanvascontext_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <SkMatrix44.h>
#include "NativeGLResources.h"
#include "NativeGLState.h"
#include "NativeTypes.h"
#include "NativeGLContext.h"

class NativeCanvas2DContext;
class NativeCanvasHandler;
struct NativeRect;

class NativeCanvasContext
{
public:

    explicit NativeCanvasContext(NativeCanvasHandler *handler);
    virtual ~NativeCanvasContext();
    
    /* Explicit name used by glBindAttribLocation */
    enum {
        SH_ATTR_POSITION = 0,
        SH_ATTR_TEXCOORD = 1,
        SH_ATTR_MODIFIER = 2
    };

    class JSObject *jsobj;
    struct JSContext *jscx;

    enum mode {
        CONTEXT_2D,
        CONTEXT_WEBGL
    } m_Mode;

    enum shaderType {
        SHADER_FRAGMENT = 0x8B30,
        SHADER_VERTEX = 0x8B31
    };

    /*
        Create a CanvasContext of type |type|
    */
    static NativeCanvasContext *Create(NativeContextType type);

    /*
        Check if the context has a program installed
    */
    bool hasShader() const {
        return (m_GLState->getProgram() != 0);
    }

    /*
        Get the current installed program or 0
    */
    uint32_t getProgram() const {
        return m_GLState->getProgram();
    }

    NativeCanvasHandler *getHandler() const {
        return this->m_Handler;
    }

    const SkMatrix44 &getMatrix() const {
        return m_Transform;
    }

    void setGLState(NativeGLState *state) {
        m_GLState = state;
    }

    NativeGLState *getGLState() const {
        return m_GLState;
    }

    bool makeGLCurrent() {
        if (!m_GLState) return false;

        return m_GLState->makeGLCurrent();
    }
    /*
        Set the appropriate OpenGL state (bind buffers, ...)
    */
    void resetGLContext();

    static char *processShader(const char *content, shaderType type);
    static uint32_t compileShader(const char *data, int type);
    
    virtual void translate(double x, double y)=0;
    virtual void setSize(int width, int height)=0;
    virtual void setScale(double x, double y, double px=1, double py=1)=0;
    virtual void clear(uint32_t color)=0;
    virtual void flush()=0;

    /* Returns the size in device pixel */
    virtual void getSize(int *width, int *height) const=0;

    virtual void composeWith(NativeCanvas2DContext *layer,
        double left, double top, double opacity,
        double zoom, const NativeRect *rclip)=0;

    /*
        Create a grid of |resolution^2| points using triangle strip

        Scheme : http://dan.lecocq.us/wordpress/wp-content/uploads/2009/12/strip.png
        Details: http://en.wikipedia.org/wiki/Triangle_strip
    */
    static NativeVertices *buildVerticesStripe(int resolution);

    static uint32_t createPassThroughVertex();
    static uint32_t createPassThroughFragment();
    static uint32_t createPassThroughProgram(NativeGLResources &resource);
protected:
    /* Hold the current matrix (model) sent to the Vertex shader */
    SkMatrix44 m_Transform;
    NativeCanvasHandler *m_Handler;
    NativeGLState *m_GLState;
    NativeGLResources m_Resources;
    void updateMatrix(double left, double top, int layerWidth, int layerHeight);

};

#endif
