#include "NativeCanvasContext.h"
#include "GLSLANG/ShaderLang.h"
#include "NativeMacros.h"

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

char *NativeCanvasContext::processShader(const char *content, shaderType type)
{
    ShBuiltInResources resources;
    ShInitBuiltInResources(&resources);
    resources.MaxVertexAttribs = 8;
    resources.MaxVertexUniformVectors = 128;
    resources.MaxVaryingVectors = 8;
    resources.MaxVertexTextureImageUnits = 0;
    resources.MaxCombinedTextureImageUnits = 8;
    resources.MaxTextureImageUnits = 8;
    resources.MaxFragmentUniformVectors = 16;
    resources.MaxDrawBuffers = 1;

    resources.OES_standard_derivatives = 0;
    resources.OES_EGL_image_external = 0;

    ShHandle compiler = NULL;

    compiler = ShConstructCompiler((ShShaderType)type,
        SH_WEBGL_SPEC, SH_GLSL_OUTPUT, &resources);

    if (compiler == NULL) {
        NLOG("Shader : Compiler not supported");
        return NULL;
    }

    if (!ShCompile(compiler, &content, 1, SH_OBJECT_CODE)) {
        size_t logLen;
        ShGetInfo(compiler, SH_INFO_LOG_LENGTH, &logLen);
        char *log = (char *)malloc(logLen);

        ShGetInfoLog(compiler, log);
        NLOG("Shader error : %s", log);

        free(log);
        return NULL;
    }
    size_t bufferLen;
    ShGetInfo(compiler, SH_OBJECT_CODE_LENGTH, &bufferLen);

    char *ocode = (char *)malloc(bufferLen);

    ShGetObjectCode(compiler, ocode);

    NLOG("Shader code : \n=========\n%s======", ocode);

    return ocode;
}

NativeCanvasContext::Vertices *NativeCanvasContext::buildVerticesStripe(int resolution)
{
    int x = resolution;
    int y = resolution;
    int t = 0;

    float xstep = 2. / ((float)x-1.);
    float ystep = 2. / ((float)y-1.);
    
    float txstep = 1.  / ((float)x-1.);
    float tystep = 1  / ((float)y-1);
    
    Vertices *info = (Vertices *)malloc(sizeof(Vertices));
    
    info->vertices = (Vertex *)malloc(sizeof(Vertex) * x * y);

    info->nvertices = x*y;
    
    info->indices = (int *)malloc((sizeof(int) * x * y) * 2);
    info->nindices = 0;
    
    Vertex *vert = info->vertices;
    int *indices = info->indices;
    
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++, t++) {
            /* TODO: Normalize using glAttributeVertex? */
            vert[t].Position[0] = -1. + ((float)j*xstep);
            vert[t].Position[1] = 1. - ((float)i*ystep);
            vert[t].Position[2] = 0.;
            
            vert[t].TexCoord[0] = ((float)j*txstep);
            vert[t].TexCoord[1] = ((float)i*tystep)*-1;
            
        }
    }

    int n   = 0;
    int colSteps = x * 2;
    int rowSteps = y - 1;
    int pos = 0;
    
    for ( int r = 0; r < rowSteps; r++) {
        for ( int c = 0; c < colSteps; c++, pos++ ) {
            int t = c + r * colSteps;
            
            if ( c == colSteps - 1 ) {
                indices[pos] = n;
            }
            else {
                indices[pos] = n;
                
                if ( t%2 == 0 ) {
                    n += x;
                }
                else {
                    if (r%2 == 0) {
                        n -= x-1;
                    } else {
                        n -= x+1;
                    }
                }
            }
        }
    }
    info->nindices = pos;
    return info;
}

void NativeCanvasContext::resetGLContext()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_GLObjects.vbo[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLObjects.vbo[1]);
}

NativeCanvasContext::NativeCanvasContext() :
    jsobj(NULL), jscx(NULL) {

    glGenBuffers(2, m_GLObjects.vbo);
    Vertices *vtx = m_GLObjects.vtx = buildVerticesStripe(128);

    /* Upload the list of vertex */
    glBindBuffer(GL_ARRAY_BUFFER, m_GLObjects.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vtx->nvertices,
        vtx->vertices, GL_STATIC_DRAW);

    /* Upload the indexes for triangle strip */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLObjects.vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * vtx->nindices,
        vtx->indices, GL_STATIC_DRAW);

    if (glGetError() != GL_NO_ERROR) {
        NLOG("Got a GL error :-(");
    }

    NLOG("Vertex buffer object created with ID : %d - %d", m_GLObjects.vbo[0], m_GLObjects.vbo[1]);
}

NativeCanvasContext::~NativeCanvasContext()
{
    glDeleteBuffers(2, m_GLObjects.vbo);
}