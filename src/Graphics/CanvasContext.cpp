/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/CanvasContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SkCanvas.h>

#include "Interface/SystemInterface.h"

#include "Graphics/GLHeader.h"
#include "Graphics/SkiaContext.h"
#include "Binding/JSCanvas2DContext.h"

using Nidium::Interface::SystemInterface;
using Nidium::Frontend::Context;
using Nidium::Binding::Canvas2DContext;
using Nidium::Binding::NidiumJS;

namespace Nidium {
namespace Graphics {

char *CanvasContext::ProcessMultipleShader(const char *content[],
    int numcontent, shaderType type, int glslversion)
{
    ShHandle compiler = NULL;

    Frontend::Context *frontendContext
        = Context::GetObject<Frontend::Context>(NidiumJS::GetObject());

    compiler = ShConstructCompiler((sh::GLenum)type, SH_WEBGL_SPEC,
                                   (!glslversion)
                                   ? frontendContext->getShaderOutputVersion()
                                   : (ShShaderOutput)glslversion,
                                   frontendContext->getShaderResources());

    if (compiler == NULL) {
        ndm_logf(NDM_LOG_ERROR, "CanvasContext", "Shader : Compiler not supported");
        return NULL;
    }

    if (!ShCompile(compiler, content, numcontent,
                   SH_VARIABLES | SH_ENFORCE_PACKING_RESTRICTIONS 
                   | SH_OBJECT_CODE
#ifdef SH_INIT_VARYINGS_WITHOUT_STATIC_USE
                   | SH_INIT_VARYINGS_WITHOUT_STATIC_USE
#endif
                   | SH_LIMIT_CALL_STACK_DEPTH | SH_INIT_GL_POSITION)) {

        std::string log = ShGetInfoLog(compiler);
        ndm_logf(NDM_LOG_ERROR, "CanvasContext", "Shader error : %s", log.c_str());

        return NULL;
    }

    std::string buffer = ShGetObjectCode(compiler);

    ShDestruct(compiler);

    return strdup(buffer.c_str());
}

char *CanvasContext::ProcessShader(const char *content, shaderType type, int glslversion)
{
    return ProcessMultipleShader(&content, 1, type, glslversion);
}

uint32_t CanvasContext::CompileShader(const char *data[], int numdata, int type)
{
    GLuint shaderHandle = glCreateShader(type);

    std::vector<int> lens;
    lens.resize(numdata);

    for (int i = 0; i < numdata; i++) {
        lens[i] = strlen(data[i]);
    }
    

    glShaderSource(shaderHandle, numdata, data, &lens[0]);
    glCompileShader(shaderHandle);

    GLint compileSuccess = GL_TRUE;

    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

    if (compileSuccess == GL_FALSE) {
        GLchar messages[4096];
        int elen;

        glGetShaderInfoLog(shaderHandle, sizeof(messages), &elen, messages);
        if (glGetError() != GL_NO_ERROR) {
            return 0;
        }

        ndm_logf(NDM_LOG_ERROR, "CanvasContext", "Shader error %d : %s\n%s", elen, messages, data[0]);
        return 0;
    }

    return shaderHandle;
}

Vertices *CanvasContext::BuildVerticesStripe(int resolution)
{
    int x = resolution;
    int y = resolution;
    int t = 0;

    float xstep = 2. / (static_cast<float>(x) - 1.);
    float ystep = 2. / (static_cast<float>(y) - 1.);

    float txstep = 1. / (static_cast<float>(x) - 1.);
    float tystep = 1. / (static_cast<float>(y) - 1.);

    Vertices *info = static_cast<Vertices *>(malloc(sizeof(Vertices)));

    info->vertices = static_cast<Vertex *>(malloc(sizeof(Vertex) * x * y));

    info->nvertices = x * y;

    info->indices
        = static_cast<unsigned int *>(malloc((sizeof(int) * x * y) * 2));
    info->nindices = 0;

    Vertex *vert          = info->vertices;
    unsigned int *indices = info->indices;

    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++, t++) {
            /* TODO: Normalize using glAttributeVertex? */
            vert[t].Position[0] = -1. + (static_cast<float>(j) * xstep);
            vert[t].Position[1] = 1. - (static_cast<float>(i) * ystep);
            vert[t].Position[2] = 0.;

            // ndm_printf("Create vertex: %f %f", vert[t].Position[0],
            // vert[t].Position[1]);

            vert[t].TexCoord[0] = (static_cast<float>(j) * txstep);
            vert[t].TexCoord[1] = 1 - ((static_cast<float>(i) * tystep));

            vert[t].Modifier[0] = 0.;
            vert[t].Modifier[1] = 0.;
        }
    }

    int n        = 0;
    int colSteps = x * 2;
    int rowSteps = y - 1;
    int pos      = 0;
    int r, c;

    for (r = 0; r < rowSteps; r++) {
        for (c = 0; c < colSteps; c++, pos++) {
            int tc = c + r * colSteps;

            if (c == colSteps - 1) {
                indices[pos] = n;
            } else {
                indices[pos] = n;

                if (tc % 2 == 0) {
                    n += x;
                } else {
                    if (r % 2 == 0) {
                        n -= x - 1;
                    } else {
                        n -= x + 1;
                    }
                }
            }
        }
    }
    info->nindices = pos;
    return info;
}

void CanvasContext::resetGLContext()
{
    if (m_GLState) {
        m_GLState->setActive();
    }
}

uint32_t CanvasContext::CreatePassThroughVertex()
{
    /* PassThrough Vertex shader */
    const char *vertex_s
        = "attribute vec4 Position;\n"
          "attribute vec2 TexCoordIn;\n"
          "attribute vec2 Modifier;\n"
          "varying vec2 TexCoordOut;\n"
          "uniform mat4 u_projectionMatrix;\n"
          "void main(void) {\n"
          "    gl_Position = u_projectionMatrix * Position + vec4(Modifier, "
          "0., 0.);\n"
          "    TexCoordOut = TexCoordIn;\n"
          "}";

    uint32_t vertexshader
        = CanvasContext::CompileShader(&vertex_s, 1, GL_VERTEX_SHADER);

    return vertexshader;
}

uint32_t CanvasContext::CreatePassThroughFragment()
{
    const char *fragment_s
        = "\n"
          "uniform sampler2D Texture;\n"
          "uniform float u_opacity;\n"
          "varying vec2 TexCoordOut;\n"
          "void main(void) {\n"
          "    gl_FragColor = texture2D(Texture, TexCoordOut.xy) * u_opacity;\n"
          "}";

    uint32_t fragmentshader = CanvasContext::CompileShader(&fragment_s, 1, GL_FRAGMENT_SHADER);

    return fragmentshader;
}

uint32_t CanvasContext::CreatePassThroughProgram(GLResources &resource)
{
    uint32_t vertexshader   = CanvasContext::CreatePassThroughVertex();
    uint32_t fragmentshader = CanvasContext::CreatePassThroughFragment();

    if (vertexshader == 0 || fragmentshader == 0) {
        return 0;
    }

    GLuint programHandle = glCreateProgram();

    resource.add(fragmentshader, GLResources::RSHADER);
    resource.add(vertexshader, GLResources::RSHADER);
    resource.add(programHandle, GLResources::RPROGRAM);

    GLint linkSuccess;

    NIDIUM_GL_CALL_MAIN(AttachShader(programHandle, vertexshader));

    NIDIUM_GL_CALL_MAIN(AttachShader(programHandle, fragmentshader));

    NIDIUM_GL_CALL_MAIN(BindAttribLocation(
        programHandle, CanvasContext::SH_ATTR_POSITION, "Position"));

    NIDIUM_GL_CALL_MAIN(BindAttribLocation(
        programHandle, CanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn"));

    NIDIUM_GL_CALL_MAIN(BindAttribLocation(
        programHandle, CanvasContext::SH_ATTR_MODIFIER, "Modifier"));

    NIDIUM_GL_CALL_MAIN(LinkProgram(programHandle));

    NIDIUM_GL_CALL_MAIN(
        GetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess));

    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        ndm_logf(NDM_LOG_ERROR, "CanvasContext", "createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

CanvasContext::CanvasContext(CanvasHandler *handler)
    : m_Mode(CONTEXT_2D), m_Transform(SkMatrix44::kIdentity_Constructor),
      m_Handler(handler), m_GLState(NULL)
{
}

CanvasContext::~CanvasContext()
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

    ndm_log(NDM_LOG_DEBUG, "CanvasContext", " = = = = = ");
    for (i = 0; i < 4; i++) {
        ndm_logf(NDM_LOG_DEBUG, "CanvasContext", "%f, %f, %f, %f", matrix[i * 4], matrix[i * 4 + 1],
               matrix[i * 4 + 2], matrix[i * 4 + 3]);
    }
    ndm_log(NDM_LOG_DEBUG, "CanvasContext", " = = = = =");
}
#endif

void CanvasContext::updateMatrix(
    double left, double top, int layerWidth, int layerHeight, GLState *glstate)
{

    if (!m_GLState) {
        return;
    }

    float px = static_cast<float>(layerWidth),
          py = static_cast<float>(layerHeight);
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

    float ratioX  = (static_cast<float>(w)) / px,
          ratioY  = (static_cast<float>(h)) / py;
    float offsetX = -1. + ratioX, offsetY = 1 - ratioY;

    float ratioL = (static_cast<float>(left)) / px;
    float ratioT = (static_cast<float>(top)) / py;

    m_Transform.preTranslate(
        /*
          We multiply by two because we're using the left-to-windowSize
          percentage whereas the coordinate has a space from -1 to 1
          (percentage*2)
        */
        SkFloatToScalar(offsetX) + (SkFloatToScalar(ratioL) * 2.f),
        SkFloatToScalar(offsetY) - (SkFloatToScalar(ratioT) * 2.f), 0);

    m_Transform.preScale(SkFloatToScalar(ratioX), SkFloatToScalar(ratioY), 1);

    if (m_GLState->m_GLObjects.uniforms.u_projectionMatrix != -1) {
        GLfloat mat4[16];
        m_Transform.asColMajorf(mat4);

        /*
            Execute the call on the specified (should be main) OpenGL context
        */
        NIDIUM_GL_CALL(
            glstate->getNidiumGLContext(),
            UniformMatrix4fv(m_GLState->m_GLObjects.uniforms.u_projectionMatrix,
                             1, GL_FALSE, mat4));
    } else {
        ndm_logf(NDM_LOG_ERROR, "CanvasContext", "No uniform found");
    }
}


void CanvasContext::setupShader(float opacity,
                                int width,
                                int height,
                                int left,
                                int top,
                                int wWidth,
                                int wHeight)
{
    uint32_t program = this->getProgram();
    NIDIUM_GL_CALL_MAIN(UseProgram(program));

    float ratio = SystemInterface::GetInstance()->backingStorePixelRatio();

    if (program > 0) {
        if (m_GLState->m_GLObjects.uniforms.u_opacity != -1) {
            NIDIUM_GL_CALL_MAIN(
                Uniform1f(m_GLState->m_GLObjects.uniforms.u_opacity, opacity));
        }
        float padding = this->getHandler()->m_Padding.global * ratio;

        if (m_GLState->m_GLObjects.uniforms.u_resolution != -1)
            NIDIUM_GL_CALL_MAIN(
                Uniform2f(m_GLState->m_GLObjects.uniforms.u_resolution,
                          (width) - (padding * 2), (height) - (padding * 2)));
        if (m_GLState->m_GLObjects.uniforms.u_position != -1)
            NIDIUM_GL_CALL_MAIN(Uniform2f(
                m_GLState->m_GLObjects.uniforms.u_position, ratio * left,
                ratio * wHeight - (height + ratio * top)));
        if (m_GLState->m_GLObjects.uniforms.u_padding != -1)
            NIDIUM_GL_CALL_MAIN(
                Uniform1f(m_GLState->m_GLObjects.uniforms.u_padding, padding));
    }
}

void CanvasContext::preComposeOn(Canvas2DContext *layer,
                                 double left,
                                 double top,
                                 double opacity,
                                 double zoom,
                                 const Rect *rclip)
{
    bool revertScissor = false;
    float ratio        = SystemInterface::GetInstance()->backingStorePixelRatio();

    SkiaContext *skia = layer->getSkiaContext();
    SkISize layerSize = skia->getCanvas()->getDeviceSize();

    /*
        Activate alpha blending
    */
    NIDIUM_GL_CALL(layer->m_GLState->getNidiumGLContext(), Enable(GL_BLEND));
    NIDIUM_GL_CALL(layer->m_GLState->getNidiumGLContext(),
                   BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    /*
        Setup clipping
    */
    if (rclip != NULL) {
        SkRect r;
        r.set(SkDoubleToScalar(rclip->m_fLeft * static_cast<double>(ratio)),
              SkDoubleToScalar(rclip->m_fTop * static_cast<double>(ratio)),
              SkDoubleToScalar(rclip->m_fRight * static_cast<double>(ratio)),
              SkDoubleToScalar(rclip->m_fBottom * static_cast<double>(ratio)));
        NIDIUM_GL_CALL(layer->m_GLState->getNidiumGLContext(),
                       Enable(GL_SCISSOR_TEST));
        NIDIUM_GL_CALL(layer->m_GLState->getNidiumGLContext(),
                       Scissor(r.left(),
                               layerSize.height() - (r.top() + r.height()),
                               r.width(), r.height()));
        revertScissor = true;
    }

    this->flush();
    this->resetGLContext();

    int width, height;

    this->getSize(&width, &height);

    this->setupShader(static_cast<float>(opacity), width, height, left, top,
                      static_cast<int>(layer->getHandler()->getWidth()),
                      static_cast<int>(layer->getHandler()->getHeight()));

    this->updateMatrix(left * ratio, top * ratio, layerSize.width(),
                       layerSize.height(), layer->m_GLState);

    layer->drawTexture(this->getTextureID());

    if (revertScissor) {
        NIDIUM_GL_CALL(layer->m_GLState->getNidiumGLContext(),
                       Disable(GR_GL_SCISSOR_TEST));
    }
}

bool CanvasContext::validateCurrentFBO()
{
    GrGLenum status;
    NIDIUM_GL_CALL_RET(m_GLState->getNidiumGLContext(),
                       CheckFramebufferStatus(GR_GL_FRAMEBUFFER), status);

    switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            ndm_logf(NDM_LOG_DEBUG, "CanvasContext", "fbo %x (incomplete multisample)", status);
            break;
        case GR_GL_FRAMEBUFFER_COMPLETE:
            break;
        case GR_GL_FRAMEBUFFER_UNSUPPORTED:
            ndm_log(NDM_LOG_WARN, "CanvasContext", "fbo unsupported");
            return false;
        default:
            ndm_logf(NDM_LOG_ERROR, "CanvasContext", "fbo fatal error %x", status);
            exit(1);
            return false;
    }

    return true;
}

/*
    TODO: implement
*/
CanvasContext *CanvasContext::Create(ContextType type)
{
    switch (type) {
        case ContextType_kWebGL:
            return NULL;
        case ContextType_kSkia2D:
            return NULL;
        default:
            ndm_logf(NDM_LOG_ERROR, "CanvasContext", "Invalid CanvasContext requested");
            return NULL;
    }
}

} // namespace Graphics
} // namespace Nidium
