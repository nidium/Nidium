#include "NativeCanvasContext.h"
#include "GLSLANG/ShaderLang.h"
#include "NativeMacros.h"
#include "NativeCanvasHandler.h"

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

NativeCanvasContext::Vertices *NativeCanvasContext::buildVerticesStripe(int resolution)
{
    int x = resolution;
    int y = resolution;
    int t = 0;

    float xstep = 2. / ((float)x-1.);
    float ystep = 2. / ((float)y-1.);
    
    float txstep = 1.  / ((float)x-1.);
    float tystep = 1.  / ((float)y-1.);
    
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

            //NLOG("Create vertex: %f %f", vert[t].Position[0], vert[t].Position[1]);
            
            vert[t].TexCoord[0] = ((float)j*txstep);
            vert[t].TexCoord[1] = 1-(((float)i*tystep));
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
    glBindVertexArray(m_GLObjects.vao);
    glActiveTexture(GL_TEXTURE0);
}

uint32_t NativeCanvasContext::createPassThroughVertex()
{
    /* PassThrough Vertex shader */
    const char *vertex_s = "#version 100\nprecision mediump float;\nattribute vec4 Position;\n"
    "attribute vec2 TexCoordIn;\n"
    "varying vec2 TexCoordOut;\n"
    "uniform mat4 u_projectionMatrix;\n"
    "void main(void) {\n"
    "    gl_Position = u_projectionMatrix * Position;\n"
    "    TexCoordOut = TexCoordIn;\n"
    "}";

    uint32_t vertexshader = NativeCanvasContext::compileShader(vertex_s, GL_VERTEX_SHADER);

    m_Resources.add(vertexshader, NativeGLResources::RSHADER);

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

    m_Resources.add(fragmentshader, NativeGLResources::RSHADER);

    return fragmentshader;
}

uint32_t NativeCanvasContext::createPassThroughProgram()
{    
    uint32_t vertexshader = this->createPassThroughVertex();
    uint32_t fragmentshader = this->createPassThroughFragment();

    if (vertexshader == 0 || fragmentshader == 0) {
        return 0;
    }

    GLuint programHandle = glCreateProgram();

    m_Resources.add(programHandle, NativeGLResources::RPROGRAM);

    GLint linkSuccess;

    glAttachShader(programHandle, vertexshader);
    glAttachShader(programHandle, fragmentshader);

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_POSITION, "Position");

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn");
    
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
    jsobj(NULL), jscx(NULL), m_Handler(handler),
    m_Transform(SkMatrix44::kIdentity_Constructor) {

    memset(&m_GLObjects.uniforms, -1, sizeof(m_GLObjects.uniforms));

    m_GLObjects.program = 0;

    glGenBuffers(2, m_GLObjects.vbo);
    glGenVertexArrays(1, &m_GLObjects.vao);

    m_Resources.add(m_GLObjects.vbo[0], NativeGLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vbo[1], NativeGLResources::RBUFFER);
    m_Resources.add(m_GLObjects.vao, NativeGLResources::RVERTEX_ARRAY);

    Vertices *vtx = m_GLObjects.vtx = buildVerticesStripe(4);
        
    glBindVertexArray(m_GLObjects.vao);

    glEnableVertexAttribArray(NativeCanvasContext::SH_ATTR_POSITION);
    glEnableVertexAttribArray(NativeCanvasContext::SH_ATTR_TEXCOORD);

    /* Upload the list of vertex */
    glBindBuffer(GL_ARRAY_BUFFER, m_GLObjects.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vtx->nvertices,
        vtx->vertices, GL_STATIC_DRAW);

    /* Upload the indexes for triangle strip */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLObjects.vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * vtx->nindices,
        vtx->indices, GL_STATIC_DRAW);

    glVertexAttribPointer(NativeCanvasContext::SH_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(NativeCanvasContext::Vertex), 0);

    glVertexAttribPointer(NativeCanvasContext::SH_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                          sizeof(NativeCanvasContext::Vertex),
                          (GLvoid*) offsetof(NativeCanvasContext::Vertex, TexCoord));

    if ((m_GLObjects.program = this->createPassThroughProgram()) != 0) {
        this->setupUniforms();
    } else {
        NLOG("[OpenGL Error] Failed to create passthrough program");
    }
}

NativeCanvasContext::~NativeCanvasContext()
{
    /* XXX this could be released after glBufferData */
    free(m_GLObjects.vtx->indices);
    free(m_GLObjects.vtx->vertices);
    free(m_GLObjects.vtx);
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

void NativeCanvasContext::setupUniforms()
{
    m_GLObjects.uniforms.u_projectionMatrix = glGetUniformLocation(m_GLObjects.program, "u_projectionMatrix");
    m_GLObjects.uniforms.u_opacity = glGetUniformLocation(m_GLObjects.program, "u_opacity");
}

void NativeCanvasContext::updateMatrix(double left, double top,
    int layerWidth, int layerHeight)
{
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

    if (m_GLObjects.uniforms.u_projectionMatrix != -1) {
        GLfloat mat4[16];
        m_Transform.asColMajorf(mat4);

        glUniformMatrix4fv(m_GLObjects.uniforms.u_projectionMatrix, 1, GL_FALSE, mat4);
    } else {
        //NLOG("No uniform found");
    }
}