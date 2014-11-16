#ifndef nativeglstate_h__
#define nativeglstate_h__

#include <stdlib.h>
#include <stdint.h>
#include <NativeTypes.h>
#include <NativeGLResources.h>
#include <NativeMacros.h>

#include "NativeGLContext.h"

#define NATIVE_MAKE_CURRENT_EACH_GL_CALL 1

#if NATIVE_MAKE_CURRENT_EACH_GL_CALL

/*
    Make the context pointed by IFANCE current and make a GL call
    e.g. NATIVE_GL_CALL(this->context, Clear(0, 0, 0, 0));
*/
#define NATIVE_GL_CALL(IFACE, X)            \
    do {                                    \
        (IFACE)->makeGLCurrent();           \
        gl##X;                              \
    } while (false)

#define NATIVE_GL_CALL_RET(IFACE, X, RET)   \
    do {                                    \
        (IFACE)->makeGLCurrent();           \
        (RET) = gl##X;                      \
    } while (false)

#define NATIVE_GL_CALL_MAIN(X)              \
    do {                                    \
        (__NativeUI)->makeMainGLCurrent();  \
        gl##X;                              \
    } while (false)

#define NATIVE_GL_CALL_RET_MAIN(X, RET)     \
    do {                                    \
        (__NativeUI)->makeMainGLCurrent();  \
        (RET) = gl##X;                      \
    } while (false)

#else


#define NATIVE_GL_CALL(IFACE, X)            \
    do {                                    \
        gl##X;                              \
    } while (false)

#define NATIVE_GL_CALL_RET(IFACE, X, RET)   \
    do {                                    \
        (RET) = gl##X;                      \
    } while (false)

#define NATIVE_GL_CALL_MAIN(X)              \
    do {                                    \
        gl##X;                              \
    } while (false)

#define NATIVE_GL_CALL_RET_MAIN(X, RET)     \
    do {                                    \
        (RET) = gl##X;                      \
    } while (false)

#endif    

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

    bool makeGLCurrent() {
        return m_GLContext->makeCurrent();
    }

    void setVertexDeformation(uint32_t vertex, float x, float y);

    NativeGLContext *getNativeGLContext() const {
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

private:
    NativeGLResources m_Resources;

    bool m_Shared;
    NativeGLContext *m_GLContext;
};

#endif