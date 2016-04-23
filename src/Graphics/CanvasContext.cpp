#include "Graphics/CanvasContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SkCanvas.h>

#include <SystemInterface.h>

#include "Graphics/Skia.h"
#include "Graphics/OpenGLHeader.h"
#include "Binding/JSCanvas2DContext.h"

namespace Nidium {
namespace Graphics {

char *NativeCanvasContext::processShader(const char *content, shaderType type)
{
    ShHandle compiler = NULL;

    compiler = ShConstructCompiler((ShShaderType)type,
        SH_WEBGL_SPEC, SH_GLSL_OUTPUT,
        NativeContext::GetObject(Nidium::Binding::NidiumJS::GetObject())->getShaderResources());

    if (compiler == NULL) {
        NLOG("Shader : Compiler not supported");
        return NULL;
    }

    if (!ShCompile(compiler, &content, 1, SH_OBJECT_CODE | SH_ATTRIBUTES_UNIFORMS |
            SH_ENFORCE_PACKING_RESTRICTIONS | SH_MAP_LONG_VARIABLE_NAMES)) {
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

    ShDestruct(compiler);

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
    int r, c;

    for (r = 0; r < rowSteps; r++) {
        for (c = 0; c < colSteps; c++, pos++) {
            int t = c + r * colSteps;

            if (c == colSteps - 1) {
                indices[pos] = n;
            }
            else {
                indices[pos] = n;

                if (t % 2 == 0) {
                    n += x;
                } else {
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
    if (m_GLState) {
        m_GLState->setActive();
    }
}

uint32_t NativeCanvasContext::createPassThroughVertex()
{
    /* PassThrough Vertex shader */
    const char *vertex_s = "attribute vec4 Position;\n"
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
    const char *fragment_s = "\n"
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

    NATIVE_GL_CALL_MAIN(AttachShader(programHandle, vertexshader));

    NATIVE_GL_CALL_MAIN(AttachShader(programHandle, fragmentshader));

    NATIVE_GL_CALL_MAIN(BindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_POSITION, "Position"));

    NATIVE_GL_CALL_MAIN(BindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn"));

    NATIVE_GL_CALL_MAIN(BindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_MODIFIER, "Modifier"));

    NATIVE_GL_CALL_MAIN(LinkProgram(programHandle));

    NATIVE_GL_CALL_MAIN(GetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess));

    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        NLOG("createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

NativeCanvasContext::NativeCanvasContext(NativeCanvasHandler *handler) :
    m_JsCx(handler->m_JsCx), m_Mode(CONTEXT_2D),
    m_Transform(SkMatrix44::kIdentity_Constructor),
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

#if DEBUG && 0
static void dump_Matrix(float *matrix)
{
    int i = 4;

    printf(" = = = = = \n");
    for (i = 0; i < 4; i++) {
        printf("%f, %f, %f, %f\n", matrix[i*4], matrix[i*4+1], matrix[i*4+2], matrix[i*4+3]);
    }
    printf(" = = = = = \n");
}
#endif

void NativeCanvasContext::updateMatrix(double left, double top,
    int layerWidth, int layerHeight, NativeGLState *glstate)
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

        /*
            Execute the call on the specified (should be main) OpenGL context
        */
        NATIVE_GL_CALL(glstate->getNativeGLContext(),
            UniformMatrix4fv(m_GLState->m_GLObjects.uniforms.u_projectionMatrix,
                1, GL_FALSE, mat4));
    } else {
        NLOG("No uniform found");
    }
}


void NativeCanvasContext::setupShader(float opacity, int width, int height,
    int left, int top, int wWidth, int wHeight)
{
    uint32_t program = this->getProgram();
    NATIVE_GL_CALL_MAIN(UseProgram(program));

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    if (program > 0) {
        if (m_GLState->m_GLObjects.uniforms.u_opacity != -1) {
            NATIVE_GL_CALL_MAIN(Uniform1f(m_GLState->m_GLObjects.uniforms.u_opacity, opacity));
        }
        float padding = this->getHandler()->m_Padding.global * ratio;

        if (m_GLState->m_GLObjects.uniforms.u_resolution != -1)
            NATIVE_GL_CALL_MAIN(Uniform2f(m_GLState->m_GLObjects.uniforms.u_resolution,
               (width) - (padding * 2), (height) - (padding * 2)));
        if (m_GLState->m_GLObjects.uniforms.u_position  != -1)
            NATIVE_GL_CALL_MAIN(Uniform2f(m_GLState->m_GLObjects.uniforms.u_position,
            ratio * left, ratio * wHeight - (height + ratio * top)));
        if (m_GLState->m_GLObjects.uniforms.u_padding != -1)
            NATIVE_GL_CALL_MAIN(Uniform1f(m_GLState->m_GLObjects.uniforms.u_padding, padding));
    }

}

void NativeCanvasContext::preComposeOn(Nidium::Binding::NativeCanvas2DContext *layer,
    double left, double top, double opacity,
    double zoom, const NativeRect *rclip)
{
    bool revertScissor = false;
    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    NativeSkia *skia = layer->getSurface();
    SkISize layerSize = skia->getCanvas()->getDeviceSize();

    /*
        Activate alpha blending
    */
    NATIVE_GL_CALL(layer->m_GLState->getNativeGLContext(), Enable(GL_BLEND));
    NATIVE_GL_CALL(layer->m_GLState->getNativeGLContext(), BlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    /*
        Setup clipping
    */
    if (rclip != NULL) {
        SkRect r;
        r.set(SkDoubleToScalar(rclip->m_fLeft*(double)ratio),
            SkDoubleToScalar(rclip->m_fTop*(double)ratio),
            SkDoubleToScalar(rclip->m_fRight*(double)ratio),
            SkDoubleToScalar(rclip->m_fBottom*(double)ratio));
        NATIVE_GL_CALL(layer->m_GLState->getNativeGLContext(), Enable(GL_SCISSOR_TEST));
        NATIVE_GL_CALL(layer->m_GLState->getNativeGLContext(), Scissor(r.left(),
           layerSize.height() - (r.top() + r.height()), r.width(), r.height()));
        revertScissor = true;
    }

    this->flush();
    this->resetGLContext();

    int width, height;

    this->getSize(&width, &height);

    this->setupShader((float)opacity, width, height,
        left, top,
        (int)layer->getHandler()->getWidth(),
        (int)layer->getHandler()->getHeight());

    this->updateMatrix(left*ratio, top*ratio, layerSize.width(),
        layerSize.height(), layer->m_GLState);

    layer->drawTexture(this->getTextureID(), width, height, left*ratio, top*ratio);

    if (revertScissor) {
        NATIVE_GL_CALL(layer->m_GLState->getNativeGLContext(), Disable(GR_GL_SCISSOR_TEST));
    }
}

bool NativeCanvasContext::validateCurrentFBO()
{
    GrGLenum status;
    NATIVE_GL_CALL_RET(m_GLState->getNativeGLContext(),
        CheckFramebufferStatus(GR_GL_FRAMEBUFFER), status);

    switch(status) {
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            printf("fbo %x (incomplete multisample)\n", status);
            break;
        case GR_GL_FRAMEBUFFER_COMPLETE:
            break;
        case GR_GL_FRAMEBUFFER_UNSUPPORTED:
            printf("fbo unsupported\n");
            return false;
        default:
            printf("fbo fatal error %x\n", status);
            exit(1);
            return false;
    }

    return true;
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

} // namespace Graphics
} // namespace Nidium

