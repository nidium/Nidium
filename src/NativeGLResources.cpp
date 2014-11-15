#include "NativeGLResources.h"

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

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
    
    m_Glid(glid), m_Type(type), m_Name(name)

{

}

NativeGLResources::Resource::~Resource()
{
    switch(this->m_Type) {
        case NativeGLResources::RPROGRAM:
            glDeleteProgram(this->m_Glid);
            break;
        case NativeGLResources::RSHADER:
            glDeleteShader(this->m_Glid);
            break;
        case NativeGLResources::RTEXTURE:
            glDeleteTextures(1, &this->m_Glid);
            break;
        case NativeGLResources::RBUFFER:
            glDeleteBuffers(1, &this->m_Glid);
            break;
        case NativeGLResources::RVERTEX_ARRAY:
            glDeleteVertexArraysAPPLE(1, &this->m_Glid);
            break;
        default:
            break;
    }
}