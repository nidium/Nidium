#include "Graphics/GLState.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "Graphics/CanvasContext.h"
#include "Graphics/OpenGLHeader.h"

namespace Nidium {
namespace Graphics {

NativeGLState::NativeGLState(Nidium::Interface::NativeUIInterface *ui, bool withProgram, bool webgl) :
    m_Shared(true)
{
    memset(&m_GLObjects, 0, sizeof(m_GLObjects));
    memset(&m_GLObjects.uniforms, -1, sizeof(m_GLObjects.uniforms));

    m_GLContext = new GLContext(ui, webgl ? NULL : ui->getGLContext(), webgl);

    if (!this->initGLBase(withProgram)) {
        NLOG("[OpenGL] Failed to init base GL");
    }
}

NativeGLState::NativeGLState(Nidium::NML::NativeContext *nctx) :
    m_Shared(true)
{
    Nidium::Interface::NativeUIInterface *ui = nctx->getUI();

    memset(&m_GLObjects, 0, sizeof(m_GLObjects));
    memset(&m_GLObjects.uniforms, -1, sizeof(m_GLObjects.uniforms));

    m_GLContext = new GLContext(ui, ui->getGLContext(), false);
}

void NativeGLState::CreateForContext(Nidium::NML::NativeContext *nctx)
{
    Nidium::Interface::NativeUIInterface *ui;
    if ((ui = nctx->getUI()) == NULL || ui->m_NativeCtx->getGLState()) {
        NLOG("Failed to init the first NativeGLState");
        return;
    }

    NativeGLState *_this = new NativeGLState(nctx);

    nctx->setGLState(_this);
    _this->initGLBase(true);
}

void NativeGLState::destroy()
{
    if (!m_Shared) {
        delete this;
    }
}

void NativeGLState::setVertexDeformation(uint32_t vertex, float x, float y)
{
    NATIVE_GL_CALL_MAIN(BindBuffer(GL_ARRAY_BUFFER, m_GLObjects.vbo[0]));

    float mod[2] = {x, y};

    NATIVE_GL_CALL_MAIN(BufferSubData(GL_ARRAY_BUFFER,
        (sizeof(Nidium::NML::Vertex) * vertex) + offsetof(Nidium::NML::Vertex, Modifier),
        sizeof(((Nidium::NML::Vertex *)0)->Modifier),
        &mod));
}

bool NativeGLState::initGLBase(bool withProgram)
{
    //glctx->iface()->fExtensions.print();

    NATIVE_GL_CALL_MAIN(GenBuffers(2, m_GLObjects.vbo));
    NATIVE_GL_CALL_MAIN(GenVertexArrays(1, &m_GLObjects.vao));

    m_Resources.add(m_GLObjects.vbo[0], GLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vbo[1], GLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vao, GLResources::RVERTEX_ARRAY);

    Nidium::NML::Vertices *vtx = m_GLObjects.vtx = CanvasContext::BuildVerticesStripe(4);

    NATIVE_GL_CALL_MAIN(BindVertexArray(m_GLObjects.vao));

    NATIVE_GL_CALL_MAIN(EnableVertexAttribArray(CanvasContext::SH_ATTR_POSITION));
    NATIVE_GL_CALL_MAIN(EnableVertexAttribArray(CanvasContext::SH_ATTR_TEXCOORD));
    NATIVE_GL_CALL_MAIN(EnableVertexAttribArray(CanvasContext::SH_ATTR_MODIFIER));

    /* Upload the list of vertex */
    NATIVE_GL_CALL_MAIN(BindBuffer(GR_GL_ARRAY_BUFFER, m_GLObjects.vbo[0]));
    NATIVE_GL_CALL_MAIN(BufferData(GR_GL_ARRAY_BUFFER, sizeof(Nidium::NML::Vertex) * vtx->nvertices,
        vtx->vertices, GL_DYNAMIC_DRAW));

    /* Upload the indexes for triangle strip */
    NATIVE_GL_CALL_MAIN(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, m_GLObjects.vbo[1]));
    NATIVE_GL_CALL_MAIN(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * vtx->nindices,
        vtx->indices, GL_STATIC_DRAW));

    NATIVE_GL_CALL_MAIN(VertexAttribPointer(CanvasContext::SH_ATTR_POSITION, 3, GR_GL_FLOAT, GR_GL_FALSE,
                          sizeof(Nidium::NML::Vertex), 0));

    NATIVE_GL_CALL_MAIN(VertexAttribPointer(CanvasContext::SH_ATTR_TEXCOORD, 2, GR_GL_FLOAT, GR_GL_FALSE,
                          sizeof(Nidium::NML::Vertex),
                          (GLvoid*) offsetof(Nidium::NML::Vertex, TexCoord)));

    NATIVE_GL_CALL_MAIN(VertexAttribPointer(CanvasContext::SH_ATTR_MODIFIER, 2, GR_GL_FLOAT, GR_GL_FALSE,
                          sizeof(Nidium::NML::Vertex),
                          (GLvoid*) offsetof(Nidium::NML::Vertex, Modifier)));

    if (withProgram) {
        m_GLObjects.program = CanvasContext::CreatePassThroughProgram(m_Resources);

        if (m_GLObjects.program == 0) {
            return false;
        }

        NATIVE_GL_CALL_RET_MAIN(GetUniformLocation(m_GLObjects.program, "u_projectionMatrix"),
            m_GLObjects.uniforms.u_projectionMatrix);

        NATIVE_GL_CALL_RET_MAIN(GetUniformLocation(m_GLObjects.program, "u_opacity"),
            m_GLObjects.uniforms.u_opacity);
    }

    NATIVE_GL_CALL_MAIN(BindVertexArray(0));

    return true;
}

void NativeGLState::setProgram(uint32_t program)
{
    m_GLObjects.program = program;

    NATIVE_GL_CALL_RET_MAIN(GetUniformLocation(m_GLObjects.program, "u_projectionMatrix"),
        m_GLObjects.uniforms.u_projectionMatrix);
    NATIVE_GL_CALL_RET_MAIN(GetUniformLocation(m_GLObjects.program, "u_opacity"),
        m_GLObjects.uniforms.u_opacity);
}

void NativeGLState::setActive()
{
    NATIVE_GL_CALL_MAIN(BindVertexArray(m_GLObjects.vao));
    NATIVE_GL_CALL_MAIN(ActiveTexture(GR_GL_TEXTURE0));
}

NativeGLState::~NativeGLState()
{
    free(m_GLObjects.vtx->indices);
    free(m_GLObjects.vtx->vertices);
    free(m_GLObjects.vtx);

    if (m_GLContext) {
        delete m_GLContext;
    }
}

} // namespace Graphics
} // namespace Nidium

