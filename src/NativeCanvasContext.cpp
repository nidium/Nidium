#include "NativeCanvasContext.h"
#include "GLSLANG/ShaderLang.h"
#include "NativeMacros.h"

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

