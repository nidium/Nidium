#include "Graphics/GLResources.h"

#include "Graphics/OpenGLHeader.h"

namespace Nidium {
namespace Graphics {

void NativeGLResources::add(uint32_t glid, ResourceType type, const char *name)
{
    if (glid == 0) {
        return;
    }

    NativeGLResources::Resource *res = new NativeGLResources::Resource(glid,
        type, NULL);

    m_List.set(this->genId(glid, type), res);
}


NativeGLResources::Resource::Resource(uint32_t glid,
    NativeGLResources::ResourceType type, char *name) :

    m_Glid(glid), m_Type(type)

{

}

NativeGLResources::Resource::~Resource()
{
    switch(m_Type) {
        case NativeGLResources::RPROGRAM:
            glDeleteProgram(m_Glid);
            break;
        case NativeGLResources::RSHADER:
            glDeleteShader(m_Glid);
            break;
        case NativeGLResources::RTEXTURE:
            glDeleteTextures(1, &m_Glid);
            break;
        case NativeGLResources::RBUFFER:
            glDeleteBuffers(1, &m_Glid);
            break;
        case NativeGLResources::RVERTEX_ARRAY:
#ifdef __APPLE__
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

