/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/GLState.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "Graphics/CanvasContext.h"
#include "Graphics/GLHeader.h"

using Nidium::Frontend::Context;
using Nidium::Interface::UIInterface;

#define SURFACE_VERTICE_RESOLUTION 4

namespace Nidium {
namespace Graphics {

GLState::GLState(UIInterface *ui, bool withProgram, bool webgl) : m_Shared(true)
{
    memset(&m_GLObjects, 0, sizeof(m_GLObjects));
    memset(&m_GLObjects.uniforms, -1, sizeof(m_GLObjects.uniforms));

    m_GLContext = new GLContext(ui, webgl ? NULL : ui->getGLContext(), webgl);

    if (!this->initGLBase(withProgram)) {
        NUI_LOG("[OpenGL] Failed to init base GL");
    }
}

GLState::GLState(Context *nctx) : m_Shared(true)
{
    UIInterface *ui = nctx->getUI();

    memset(&m_GLObjects, 0, sizeof(m_GLObjects));
    memset(&m_GLObjects.uniforms, -1, sizeof(m_GLObjects.uniforms));

    m_GLContext = new GLContext(ui, ui->getGLContext(), false);
}

void GLState::CreateForContext(Context *nctx)
{
    UIInterface *ui;
    if ((ui = nctx->getUI()) == NULL || ui->m_NidiumCtx->getGLState()) {
        NUI_LOG("Failed to init the first GLState");
        return;
    }

    GLState *_this = new GLState(nctx);

    nctx->setGLState(_this);
    _this->initGLBase(true);
}

void GLState::destroy()
{
    if (!m_Shared) {
        delete this;
    }
}

void GLState::setVertexDeformation(uint32_t vertex, float x, float y)
{
    NIDIUM_GL_CALL_MAIN(BindBuffer(GL_ARRAY_BUFFER, m_GLObjects.vbo[0]));

    float mod[2] = { x, y };

    NIDIUM_GL_CALL_MAIN(BufferSubData(
        GL_ARRAY_BUFFER, (sizeof(Vertex) * vertex) + offsetof(Vertex, Modifier),
        sizeof(((Vertex *)0)->Modifier), &mod));
}

bool GLState::initGLBase(bool withProgram)
{
    // glctx->iface()->fExtensions.print();

    NIDIUM_GL_CALL_MAIN(GenBuffers(2, m_GLObjects.vbo));
    NIDIUM_GL_CALL_MAIN(GenVertexArrays(1, &m_GLObjects.vao));

    m_Resources.add(m_GLObjects.vbo[0], GLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vbo[1], GLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vao, GLResources::RVERTEX_ARRAY);

    Vertices *vtx = m_GLObjects.vtx = CanvasContext::BuildVerticesStripe(SURFACE_VERTICE_RESOLUTION);

    NIDIUM_GL_CALL_MAIN(BindVertexArray(m_GLObjects.vao));

    NIDIUM_GL_CALL_MAIN(
        EnableVertexAttribArray(CanvasContext::SH_ATTR_POSITION));
    NIDIUM_GL_CALL_MAIN(
        EnableVertexAttribArray(CanvasContext::SH_ATTR_TEXCOORD));
    NIDIUM_GL_CALL_MAIN(
        EnableVertexAttribArray(CanvasContext::SH_ATTR_MODIFIER));

    /* Upload the list of vertex */
    NIDIUM_GL_CALL_MAIN(BindBuffer(GR_GL_ARRAY_BUFFER, m_GLObjects.vbo[0]));
    NIDIUM_GL_CALL_MAIN(BufferData(GR_GL_ARRAY_BUFFER,
                                   sizeof(Vertex) * vtx->nvertices,
                                   vtx->vertices, GL_DYNAMIC_DRAW));

    /* Upload the indexes for triangle strip */
    NIDIUM_GL_CALL_MAIN(
        BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, m_GLObjects.vbo[1]));
    NIDIUM_GL_CALL_MAIN(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                                   sizeof(int) * vtx->nindices, vtx->indices,
                                   GL_STATIC_DRAW));

    NIDIUM_GL_CALL_MAIN(VertexAttribPointer(CanvasContext::SH_ATTR_POSITION, 3,
                                            GR_GL_FLOAT, GR_GL_FALSE,
                                            sizeof(Vertex), 0));

    NIDIUM_GL_CALL_MAIN(VertexAttribPointer(
        CanvasContext::SH_ATTR_TEXCOORD, 2, GR_GL_FLOAT, GR_GL_FALSE,
        sizeof(Vertex), (GLvoid *)offsetof(Vertex, TexCoord)));

    NIDIUM_GL_CALL_MAIN(VertexAttribPointer(
        CanvasContext::SH_ATTR_MODIFIER, 2, GR_GL_FLOAT, GR_GL_FALSE,
        sizeof(Vertex), (GLvoid *)offsetof(Vertex, Modifier)));

    if (withProgram) {
        m_GLObjects.program
            = CanvasContext::CreatePassThroughProgram(m_Resources);

        if (m_GLObjects.program == 0) {
            return false;
        }

        NIDIUM_GL_CALL_RET_MAIN(
            GetUniformLocation(m_GLObjects.program, "u_projectionMatrix"),
            m_GLObjects.uniforms.u_projectionMatrix);

        NIDIUM_GL_CALL_RET_MAIN(
            GetUniformLocation(m_GLObjects.program, "u_opacity"),
            m_GLObjects.uniforms.u_opacity);
    }

    NIDIUM_GL_CALL_MAIN(BindVertexArray(0));

    return true;
}

void GLState::setProgram(uint32_t program)
{
    m_GLObjects.program = program;

    NIDIUM_GL_CALL_RET_MAIN(
        GetUniformLocation(m_GLObjects.program, "u_projectionMatrix"),
        m_GLObjects.uniforms.u_projectionMatrix);
    NIDIUM_GL_CALL_RET_MAIN(
        GetUniformLocation(m_GLObjects.program, "u_opacity"),
        m_GLObjects.uniforms.u_opacity);
}

void GLState::setActive()
{
    NIDIUM_GL_CALL_MAIN(BindVertexArray(m_GLObjects.vao));
    NIDIUM_GL_CALL_MAIN(ActiveTexture(GR_GL_TEXTURE0));
}

GLState::~GLState()
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
