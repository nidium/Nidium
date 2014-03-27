#include "NativeCanvasContext.h"
#include "GLSLANG/ShaderLang.h"
#include "NativeMacros.h"
#include "NativeCanvasHandler.h"

#include "NativeCanvas2DContext.h"

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
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

    return ocode;
}

uint32_t NativeCanvasContext::compileShader(const char *data, int type)
{
    GLuint shaderHandle = glCreateShader(type);

    int len = strlen(data);
    glShaderSource(shaderHandle, 1, &data, &len);
    glCompileShader(shaderHandle);

    GLint compileSuccess = GL_TRUE;

    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

    if (compileSuccess == GL_FALSE) {
        GLchar messages[512];
        int len;
        glGetShaderInfoLog(shaderHandle, sizeof(messages), &len, messages);
        if (glGetError() != GL_NO_ERROR) {
            return 0;
        }
        NLOG("Shader error %d : %s\n%s", len, messages, data);
        return 0;
    }
    
    return shaderHandle;
}

NativeVertices *NativeCanvasContext::buildVerticesStripe(int resolution)
{
    int x = resolution;
    int y = resolution;
    int t = 0;

    float xstep = 2. / ((float)x-1.);
    float ystep = 2. / ((float)y-1.);
    
    float txstep = 1.  / ((float)x-1.);
    float tystep = 1.  / ((float)y-1.);
    
    NativeVertices *info = (NativeVertices *)malloc(sizeof(NativeVertices));
    
    info->vertices = (NativeVertex *)malloc(sizeof(NativeVertex) * x * y);

    info->nvertices = x*y;
    
    info->indices = (unsigned int *)malloc((sizeof(int) * x * y) * 2);
    info->nindices = 0;
    
    NativeVertex *vert = info->vertices;
    unsigned int *indices = info->indices;
    
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++, t++) {
            /* TODO: Normalize using glAttributeVertex? */
            vert[t].Position[0] = -1. + ((float)j*xstep);
            vert[t].Position[1] = 1. - ((float)i*ystep);
            vert[t].Position[2] = 0.;

            //NLOG("Create vertex: %f %f", vert[t].Position[0], vert[t].Position[1]);
            
            vert[t].TexCoord[0] = ((float)j*txstep);
            vert[t].TexCoord[1] = 1-(((float)i*tystep));

            vert[t].Modifier[0] = 0.;
            vert[t].Modifier[1] = 0.;
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
    if (this->m_GLState) {
        this->m_GLState->setActive();
    }
}

uint32_t NativeCanvasContext::createPassThroughVertex()
{
    /* PassThrough Vertex shader */
    const char *vertex_s = "#version 100\nprecision mediump float;\nattribute vec4 Position;\n"
    "attribute vec2 TexCoordIn;\n"
    "attribute vec2 Modifier;\n"
    "varying vec2 TexCoordOut;\n"
    "uniform mat4 u_projectionMatrix;\n"
    "void main(void) {\n"
    "    gl_Position = u_projectionMatrix * Position + vec4(Modifier, 0., 0.);\n"
    "    TexCoordOut = TexCoordIn;\n"
    "}";

    uint32_t vertexshader = NativeCanvasContext::compileShader(vertex_s, GL_VERTEX_SHADER);

    return vertexshader;
}

uint32_t NativeCanvasContext::createPassThroughFragment()
{
    const char *fragment_s = "#version 100\nprecision mediump float;\n"
    "uniform sampler2D Texture;\n"
    "uniform float u_opacity;\n"
    "varying vec2 TexCoordOut;\n"
    "void main(void) {\n"
    "    gl_FragColor = texture2D(Texture, TexCoordOut.xy) * u_opacity;\n"
    "}";
    
    uint32_t fragmentshader = NativeCanvasContext::compileShader(fragment_s, GL_FRAGMENT_SHADER);

    return fragmentshader;
}

uint32_t NativeCanvasContext::createPassThroughProgram(NativeGLResources &resource)
{   
    uint32_t vertexshader = NativeCanvasContext::createPassThroughVertex();
    uint32_t fragmentshader = NativeCanvasContext::createPassThroughFragment();

    if (vertexshader == 0 || fragmentshader == 0) {
        return 0;
    }

    GLuint programHandle = glCreateProgram();

    resource.add(fragmentshader, NativeGLResources::RSHADER);
    resource.add(vertexshader, NativeGLResources::RSHADER);
    resource.add(programHandle, NativeGLResources::RPROGRAM);

    GLint linkSuccess;

    glAttachShader(programHandle, vertexshader);
    glAttachShader(programHandle, fragmentshader);

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_POSITION, "Position");

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn");

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_MODIFIER, "Modifier");
    
    glLinkProgram(programHandle);

    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);

    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        NLOG("createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

NativeCanvasContext::NativeCanvasContext(NativeCanvasHandler *handler) :
    jsobj(NULL), jscx(NULL), m_Transform(SkMatrix44::kIdentity_Constructor),
    m_Handler(handler), m_GLState(NULL)
{


}

NativeCanvasContext::~NativeCanvasContext()
{
    if (m_GLState) {
        m_GLState->destroy();
        m_GLState = NULL;
    }
}

static void dump_Matrix(float *matrix)
{
    int i = 4;

    printf("==========\n");
    for (i = 0; i < 4; i++) {
        printf("%f,%f,%f,%f\n", matrix[i*4], matrix[i*4+1], matrix[i*4+2], matrix[i*4+3]);
    }
    printf("==========\n");
}

void NativeCanvasContext::updateMatrix(double left, double top,
    int layerWidth, int layerHeight)
{

    if (!m_GLState) {
        return;
    }

    float px = (float)layerWidth, py = (float)layerHeight;
    int w, h;

    /* get the size in device pixels */
    this->getSize(&w, &h);

    m_Transform.reset();

    /*
        X position : -1 ----- 0 ------ 1


        The canvas is scalled to match its size relative to the window ratio.
        Scale to (e.g.) 0.5 will shift the left position from -1 to 0
        and the right position from 1 to 0 -- thus center the canvas.
        We therefore have to translate the canvas back to its original position
    */

    float ratioX = ((float)w)/px, ratioY = ((float)h)/py;
    float offsetX = -1.+ratioX, offsetY = 1-ratioY;

    float ratioL = ((float)left)/px;
    float ratioT = ((float)top)/py;

    m_Transform.preTranslate(
        /*
          We multiply by two because we're using the left-to-windowSize
          percentage whereas the coordinate has a space from -1 to 1 (percentage*2)
        */
        SkFloatToScalar(offsetX) + (SkFloatToScalar(ratioL)*2.f),
        SkFloatToScalar(offsetY) - (SkFloatToScalar(ratioT)*2.f),
        0);

    m_Transform.preScale(SkFloatToScalar(ratioX), SkFloatToScalar(ratioY), 1);

    if (m_GLState->m_GLObjects.uniforms.u_projectionMatrix != -1) {
        GLfloat mat4[16];
        m_Transform.asColMajorf(mat4);
        NATIVE_GL_CALL(m_GLState, UniformMatrix4fv(m_GLState->m_GLObjects.uniforms.u_projectionMatrix, 1, GL_FALSE, mat4));
    } else {
        NLOG("No uniform found");
    }
}

/*
    TODO: implement
*/
NativeCanvasContext *NativeCanvasContext::Create(NativeContextType type)
{
    switch (type) {
        case kWebGL_ContextType:
            return NULL;
        case kSkia2D_ContextType:
            return NULL;
        default:
            NLOG("[Error] Invalid CanvasContext requested");
            return NULL;
    }
}