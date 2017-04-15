/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_canvascontext_h__
#define graphics_canvascontext_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <SkMatrix44.h>
#include <jsapi.h>

#include "Graphics/GLResources.h"
#include "Graphics/GLState.h"
#include "Graphics/GLContext.h"

namespace Nidium {
namespace Binding {
class Canvas2DContext;
}
namespace Graphics {

struct Rect;
class CanvasHandler;

enum ContextType
{
    ContextType_kSkia2D,
    ContextType_kWebGL
};

class CanvasContext
{
public:
    explicit CanvasContext(CanvasHandler *handler);
    virtual ~CanvasContext();

    /* Explicit name used by glBindAttribLocation */
    enum
    {
        SH_ATTR_POSITION = 0,
        SH_ATTR_TEXCOORD = 1,
        SH_ATTR_MODIFIER = 2
    };

    enum mode
    {
        CONTEXT_2D,
        CONTEXT_WEBGL
    } m_Mode;

    enum shaderType
    {
        SHADER_FRAGMENT = 0x8B30,
        SHADER_VERTEX   = 0x8B31
    };

    /*
        Create a CanvasContext of type |type|
    */
    static CanvasContext *Create(ContextType type);

    mode getContextType() const
    {
        return m_Mode;
    }

    /*
        Check if the context has a program installed
    */
    bool hasShader() const
    {
        return (m_GLState->getProgram() != 0);
    }

    /*
        Get the current installed program or 0
    */
    uint32_t getProgram() const
    {
        return m_GLState->getProgram();
    }

    CanvasHandler *getHandler() const
    {
        return m_Handler;
    }

    const SkMatrix44 &getMatrix() const
    {
        return m_Transform;
    }

    void setGLState(GLState *state)
    {
        m_GLState = state;
    }

    inline GLState *getGLState() const
    {
        return m_GLState;
    }

    inline GLContext *getGLContext() const
    {
        if (!m_GLState) {
            ndm_logf(NDM_LOG_ERROR, "CanvasContext", "getGLContext() invalid glstate on %p", this);
            return NULL;
        }

        return m_GLState->getNidiumGLContext();
    }

    bool makeGLCurrent()
    {
        return m_GLState->makeGLCurrent();
    }

    bool validateCurrentFBO();

    /*
        Set the appropriate OpenGL state (bind buffers, ...)
    */
    void resetGLContext();

    static char *ProcessShader(const char *content, shaderType type, int glslversion = 0);
    static char *ProcessMultipleShader(const char *content[], int numcontent, shaderType type, int glslversion = 0);

    static uint32_t CompileShader(const char *data[], int numdata, int type);

    virtual void translate(double x, double y) = 0;
    virtual void setSize(int width, int height, bool redraw = true) = 0;
    virtual void setScale(double x, double y, double px = 1, double py = 1) = 0;
    virtual void clear(uint32_t color = 0x00000000) = 0;
    virtual void flush() = 0;
    virtual uint32_t getTextureID() const = 0;

    /* Returns the size in device pixel */
    virtual void getSize(int *width, int *height) const = 0;

    virtual uint8_t *getPixels()
    {
        return NULL;
    }

    /*
        Create a grid of |resolution^2| points using triangle strip

        Scheme :
       http://dan.lecocq.us/wordpress/wp-content/uploads/2009/12/strip.png
        Details: http://en.wikipedia.org/wiki/Triangle_strip
    */
    static Vertices *BuildVerticesStripe(int resolution);

    static uint32_t CreatePassThroughVertex();
    static uint32_t CreatePassThroughFragment();
    static uint32_t CreatePassThroughProgram(GLResources &resource);


    void preComposeOn(Binding::Canvas2DContext *layer,
                      double left,
                      double top,
                      double opacity,
                      double zoom,
                      const Rect *rclip);

    virtual JSObject *getJSInstance()=0;

protected:
    /* Hold the current matrix (model) sent to the Vertex shader */
    SkMatrix44 m_Transform;
    CanvasHandler *m_Handler;
    GLState *m_GLState;
    GLResources m_Resources;
    void updateMatrix(double left,
                      double top,
                      int layerWidth,
                      int layerHeight,
                      GLState *glstate);

    /*
        Set various uniform values
        to the attached canvas shader
    */
    void setupShader(float opacity,
                     int width,
                     int height,
                     int left,
                     int top,
                     int wWidth,
                     int wHeight);
};

} // namespace Graphics
} // namespace Nidium

#endif
