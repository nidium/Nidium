#ifndef nidium_glstate_h__
#define nidium_glstate_h__

#include <stdlib.h>
#include <stdint.h>

#include "NML/Types.h"
#include "NML/Macros.h"
#include "Graphics/GLResources.h"
#include "Graphics/GLContext.h"

class NativeUIInterface;

class NativeGLState
{

public:

    NativeGLState(NativeUIInterface *ui,
        bool withProgram = true, bool webgl = false);
    ~NativeGLState();

    bool initGLBase(bool withProgram = true);
    void setActive();
    void destroy();

    void setShared(bool val) {
        m_Shared = val;
    }

    bool isShared() const {
        return m_Shared;
    }

    uint32_t getProgram() const {
        return m_GLObjects.program;
    }

    void setProgram(uint32_t program);

    inline bool makeGLCurrent() {
        return m_GLContext->makeCurrent();
    }

    void setVertexDeformation(uint32_t vertex, float x, float y);

    inline NativeGLContext *getNativeGLContext() const {
        return m_GLContext;
    }

    struct {
        uint32_t vbo[2];
        uint32_t vao;
        NativeVertices *vtx;
        uint32_t program;
        struct {
            int32_t u_projectionMatrix;
            int32_t u_opacity;
            int32_t u_resolution;
            int32_t u_position;
            int32_t u_padding;
        } uniforms;
    } m_GLObjects;

    static void CreateForContext(NativeContext *nctx);

private:
    NativeGLState(NativeContext *nctx);

    NativeGLResources m_Resources;

    bool m_Shared;
    NativeGLContext *m_GLContext;
};

#endif

