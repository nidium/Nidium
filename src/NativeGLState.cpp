#include "NativeGLState.h"
#include "NativeCanvasContext.h"
#include "NativeTypes.h"
#include <stdlib.h>
#include <stddef.h>

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif


NativeGLState::NativeGLState()
{
    memset(&this->m_GLObjects, 0, sizeof(this->m_GLObjects));
    memset(&this->m_GLObjects.uniforms, -1, sizeof(this->m_GLObjects.uniforms));

    this->initGLBase();
}

bool NativeGLState::initGLBase()
{
    glGenBuffers(2, m_GLObjects.vbo);
    glGenVertexArrays(1, &m_GLObjects.vao);

    m_Resources.add(m_GLObjects.vbo[0], NativeGLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vbo[1], NativeGLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vao, NativeGLResources::RVERTEX_ARRAY);

    NativeVertices *vtx = m_GLObjects.vtx = NativeCanvasContext::buildVerticesStripe(4);

    glBindVertexArray(m_GLObjects.vao);

    glEnableVertexAttribArray(NativeCanvasContext::SH_ATTR_POSITION);
    glEnableVertexAttribArray(NativeCanvasContext::SH_ATTR_TEXCOORD);

    /* Upload the list of vertex */
    glBindBuffer(GL_ARRAY_BUFFER, m_GLObjects.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NativeVertex) * vtx->nvertices,
        vtx->vertices, GL_STATIC_DRAW);

    /* Upload the indexes for triangle strip */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLObjects.vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * vtx->nindices,
        vtx->indices, GL_STATIC_DRAW);

    glVertexAttribPointer(NativeCanvasContext::SH_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(NativeVertex), 0);

    glVertexAttribPointer(NativeCanvasContext::SH_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                          sizeof(NativeVertex),
                          (GLvoid*) offsetof(NativeVertex, TexCoord));

    this->m_GLObjects.program = NativeCanvasContext::createPassThroughProgram(this->m_Resources);

    m_GLObjects.uniforms.u_projectionMatrix = glGetUniformLocation(m_GLObjects.program, "u_projectionMatrix");
    m_GLObjects.uniforms.u_opacity = glGetUniformLocation(m_GLObjects.program, "u_opacity");    

    glBindVertexArray(0);

    return true;
}

void NativeGLState::setActive()
{
    glBindVertexArray(m_GLObjects.vao);
    glActiveTexture(GL_TEXTURE0);
}

NativeGLState::~NativeGLState()
{
    free(m_GLObjects.vtx->indices);
    free(m_GLObjects.vtx->vertices);
    free(m_GLObjects.vtx);
}