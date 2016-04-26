#ifndef graphics_glstate_h__
#define graphics_glstate_h__

#include <stdlib.h>
#include <stdint.h>

#include "NML/Macros.h"
#include "Graphics/GLResources.h"
#include "Graphics/GLContext.h"

namespace Nidium {
    namespace Interface {
        class NativeUIInterface;
    }
namespace Graphics {

typedef struct _Vertex {
    float Position[3];
    float TexCoord[2];
    float Modifier[2];
} Vertex;

typedef struct _Vertices {
    Vertex *vertices;
    unsigned int *indices;
    unsigned int nvertices;
    unsigned int nindices;
} Vertices;


class GLState
{

public:

    GLState(Nidium::Interface::NativeUIInterface *ui,
        bool withProgram = true, bool webgl = false);
    ~GLState();

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

    inline GLContext *getNativeGLContext() const {
        return m_GLContext;
    }

    struct {
        uint32_t vbo[2];
        uint32_t vao;
        Vertices *vtx;
        uint32_t program;
        struct {
            int32_t u_projectionMatrix;
            int32_t u_opacity;
            int32_t u_resolution;
            int32_t u_position;
            int32_t u_padding;
        } uniforms;
    } m_GLObjects;

    static void CreateForContext(Nidium::NML::NativeContext *nctx);

private:
    GLState(Nidium::NML::NativeContext *nctx);

    GLResources m_Resources;

    bool m_Shared;
    GLContext *m_GLContext;
};

} // namespace Graphics
} // namespace Nidium

#endif

