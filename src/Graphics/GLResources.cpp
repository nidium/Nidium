/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/GLResources.h"

#include "Graphics/GLHeader.h"

namespace Nidium {
namespace Graphics {

GLResources::Resource::Resource(uint32_t glid,
                                GLResources::ResourceType type,
                                char *name)
    :

      m_Glid(glid),
      m_Type(type)
{
}

void GLResources::add(uint32_t glid, ResourceType type, const char *name)
{
    if (glid == 0) {
        return;
    }

    GLResources::Resource *res = new GLResources::Resource(glid, type, NULL);

    m_List.set(this->genId(glid, type), res);
}


GLResources::Resource::~Resource()
{
    switch (m_Type) {
        case GLResources::RPROGRAM:
            glDeleteProgram(m_Glid);
            break;
        case GLResources::RSHADER:
            glDeleteShader(m_Glid);
            break;
        case GLResources::RTEXTURE:
            glDeleteTextures(1, &m_Glid);
            break;
        case GLResources::RBUFFER:
            glDeleteBuffers(1, &m_Glid);
            break;
        case GLResources::RVERTEX_ARRAY:
#if NIDIUM_OPENGLES2
            glDeleteVertexArraysOES(1, &m_Glid);
#elif __APPLE__
            glDeleteVertexArraysAPPLE(1, &m_Glid);
#else
            glDeleteVertexArrays(1, &m_Glid);
#endif
            break;
        default:
            break;
    }
}

} // namespace Graphics
} // namespace Nidium
