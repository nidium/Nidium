#include "NativeGLResources.h"

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

void NativeGLResources::add(uint32_t glid, ResourceType type, const char *name)
{

    NativeGLResources::Resource *res = new NativeGLResources::Resource(glid,
        type, strdup(name));

    m_List.set(name, res);
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
        default:
            break;
    }
}