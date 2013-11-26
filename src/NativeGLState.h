#ifndef nativeglstate_h__
#define nativeglstate_h__

#include <stdlib.h>
#include <stdint.h>
#include <NativeTypes.h>
#include <NativeGLResources.h>

class NativeGLState
{

public:

    NativeGLState();
    ~NativeGLState();

    bool initGLBase();
    void setActive();

    uint32_t getProgram() const {
        return m_GLObjects.program;
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
};

#endif