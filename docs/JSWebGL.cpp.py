# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc("WebGL", "A W3C compliant implementation of WebGL API inside Nidium", NO_Sees,
    [ExampleDoc("""var c = new Canvas(100, 100);
document.canvas.add(c);
var webglContext = c.getContext("webgl");""")],
    products=["Frontend"]
)

items = [ "WebGLRenderingContext", "WebGLBuffer", "WebGLFramebuffer", "WebGLProgram", "WebGLRenderbuffer", "WebGLShader", "WebGLTexture", "WebGLUniformLocation", "WebGLShaderPrecisionFormat", "WebGLActiveInfo" ]
for i in items:
    NamespaceDoc( i, i + " Class.",
        SeesDocs( "|".join( items ) ),
        NO_Examples,
        section="WebGL"
    )

items = [ "uniform1f", "uniform2f", "uniform3f", "uniform4f" ]
vals = ['x_pos', 'y_pos', 'z_index', 'w_pos']
params = []
for j,i in enumerate( items ):
    params.append( ParamDoc( vals[j], vals[j].title() + " value", "integer", NO_Default, IS_Obligated ) )
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        params,
        NO_Returns
    )

items = [ "uniform1fv", "uniform2fv", "uniform3fv", "uniform4fv" ]
for j,i in enumerate( items ):
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        [ ParamDoc( "location", "Location", "WebGLUniformLocation", NO_Default, IS_Obligated ),
        ParamDoc( "array", "Array", "[float]", NO_Default, IS_Obligated ) ],
        NO_Returns
    )

items = [ "uniform1i", "uniform2i", "uniform3i", "uniform4i" ]
vals = ['x_pos', 'y_pos', 'z_index', 'width']
params = [ParamDoc( "location", "Location", "WebGLUniformLocation", NO_Default, IS_Obligated ) ]
for j,i in enumerate( items ):
    params.append( ParamDoc( vals[j], vals[j].upper() + " value", "integer", NO_Default, IS_Obligated ) )
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        params,
        NO_Returns
    )

items = [ "uniform1iv", "uniform2iv", "uniform3iv", "uniform4iv" ]
for j,i in enumerate( items ):
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        [ ParamDoc( "location", "Location", "WebGLUniformLocation", NO_Default, IS_Obligated ),
        ParamDoc( "array", "Array", "[float]", NO_Default, IS_Obligated ) ],
        NO_Returns
    )

items = [ "uniform1iv", "uniform2iv", "uniform3iv", "uniform4iv" ]
for j,i in enumerate( items ):
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        [ ParamDoc( "location", "Location", "WebGLUniformLocation", NO_Default, IS_Obligated ),
        ParamDoc( "transpose", "Transpose the matrix", "boolean", NO_Default, IS_Obligated ),
        ParamDoc( "array", "Array", "[float]", NO_Default, IS_Obligated ) ],
        NO_Returns
    )

items = [ "vertexAttrib1f", "vertexAttrib2f", "vertexAttrib3f", "vertexAttrib4f"]
vals = ['v0', 'v1', 'v2', 'v3']
params = [ParamDoc( "index", "Index", "integer", NO_Default, IS_Obligated ) ]
for j, i in enumerate( items ):
    params.append( ParamDoc( vals[j], vals[j].upper() + " value", "integer", NO_Default, IS_Obligated ) )
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        params,
        NO_Returns
    )

items = [ "vertexAttrib1fv", "vertexAttrib2fv", "vertexAttrib3fv", "vertexAttrib4fv"]
for j,i in enumerate( items ):
    FunctionDoc( "WebGLRenderingContext." + i, "Performs a " + i,
        SeesDocs( "WebGLRenderingContext|" + "|WebGLRenderingContext.".join( items ) ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        [ParamDoc( "index", "Index", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "array", "Array", "[float]", NO_Default, IS_Obligated ) ],
        NO_Returns
    )

items = ['DEPTH_BUFFER_BIT', 'STENCIL_BUFFER_BIT', 'COLOR_BUFFER_BIT', 'POINTS', 'LINES', 'LINE_LOOP', 'LINE_STRIP', 'TRIANGLES', 'TRIANGLE_STRIP', 'TRIANGLE_FAN', 'ZERO', 'ONE', 'SRC_COLOR', 'ONE_MINUS_SRC_COLOR', 'SRC_ALPHA', 'ONE_MINUS_SRC_ALPHA', 'DST_ALPHA', 'ONE_MINUS_DST_ALPHA', 'DST_COLOR', 'ONE_MINUS_DST_COLOR', 'SRC_ALPHA_SATURATE', 'FUNC_ADD', 'BLEND_EQUATION', 'BLEND_EQUATION_RGB', 'BLEND_EQUATION_ALPHA', 'FUNC_SUBTRACT', 'FUNC_REVERSE_SUBTRACT', 'BLEND_DST_RGB', 'BLEND_SRC_RGB', 'BLEND_DST_ALPHA', 'BLEND_SRC_ALPHA', 'CONSTANT_COLOR', 'ONE_MINUS_CONSTANT_COLOR', 'CONSTANT_ALPHA', 'ONE_MINUS_CONSTANT_ALPHA', 'BLEND_COLOR', 'ARRAY_BUFFER', 'ELEMENT_ARRAY_BUFFER', 'ARRAY_BUFFER_BINDING', 'ELEMENT_ARRAY_BUFFER_BINDING', 'STREAM_DRAW', 'STATIC_DRAW', 'DYNAMIC_DRAW', 'BUFFER_SIZE', 'BUFFER_USAGE', 'CURRENT_VERTEX_ATTRIB', 'FRONT', 'BACK', 'FRONT_AND_BACK', 'TEXTURE_2D', 'CULL_FACE', 'BLEND', 'DITHER', 'STENCIL_TEST', 'DEPTH_TEST', 'SCISSOR_TEST', 'POLYGON_OFFSET_FILL', 'SAMPLE_ALPHA_TO_COVERAGE', 'SAMPLE_COVERAGE', 'NO_ERROR', 'INVALID_ENUM', 'INVALID_VALUE', 'INVALID_OPERATION', 'OUT_OF_MEMORY', 'CW', 'CCW', 'LINE_WIDTH', 'ALIASED_POINT_SIZE_RANGE', 'ALIASED_LINE_WIDTH_RANGE', 'CULL_FACE_MODE', 'FRONT_FACE', 'DEPTH_RANGE', 'DEPTH_WRITEMASK', 'DEPTH_CLEAR_VALUE', 'DEPTH_FUNC', 'STENCIL_CLEAR_VALUE', 'STENCIL_FUNC', 'STENCIL_FAIL', 'STENCIL_PASS_DEPTH_FAIL', 'STENCIL_PASS_DEPTH_PASS', 'STENCIL_REF', 'STENCIL_VALUE_MASK', 'STENCIL_WRITEMASK', 'STENCIL_BACK_FUNC', 'STENCIL_BACK_FAIL', 'STENCIL_BACK_PASS_DEPTH_FAIL', 'STENCIL_BACK_PASS_DEPTH_PASS', 'STENCIL_BACK_REF', 'STENCIL_BACK_VALUE_MASK', 'STENCIL_BACK_WRITEMASK', 'VIEWPORT', 'SCISSOR_BOX', 'COLOR_CLEAR_VALUE', 'COLOR_WRITEMASK', 'UNPACK_ALIGNMENT', 'PACK_ALIGNMENT', 'MAX_TEXTURE_SIZE', 'MAX_VIEWPORT_DIMS', 'SUBPIXEL_BITS', 'RED_BITS', 'GREEN_BITS', 'BLUE_BITS', 'ALPHA_BITS', 'DEPTH_BITS', 'STENCIL_BITS', 'POLYGON_OFFSET_UNITS', 'POLYGON_OFFSET_FACTOR', 'TEXTURE_BINDING_2D', 'SAMPLE_BUFFERS', 'SAMPLES', 'SAMPLE_COVERAGE_VALUE', 'SAMPLE_COVERAGE_INVERT', 'COMPRESSED_TEXTURE_FORMATS', 'DONT_CARE', 'FASTEST', 'NICEST', 'GENERATE_MIPMAP_HINT', 'BYTE', 'UNSIGNED_BYTE', 'SHORT', 'UNSIGNED_SHORT', 'INT', 'UNSIGNED_INT', 'FLOAT', 'DEPTH_COMPONENT', 'ALPHA', 'RGB', 'RGBA', 'LUMINANCE', 'LUMINANCE_ALPHA', 'UNSIGNED_SHORT_4_4_4_4', 'UNSIGNED_SHORT_5_5_5_1', 'UNSIGNED_SHORT_5_6_5', 'FRAGMENT_SHADER', 'VERTEX_SHADER', 'MAX_VERTEX_ATTRIBS', 'MAX_VERTEX_UNIFORM_VECTORS', 'MAX_VARYING_VECTORS', 'MAX_COMBINED_TEXTURE_IMAGE_UNITS', 'MAX_VERTEX_TEXTURE_IMAGE_UNITS', 'MAX_TEXTURE_IMAGE_UNITS', 'MAX_FRAGMENT_UNIFORM_VECTORS', 'SHADER_TYPE', 'DELETE_STATUS', 'LINK_STATUS', 'VALIDATE_STATUS', 'ATTACHED_SHADERS', 'ACTIVE_UNIFORMS', 'ACTIVE_ATTRIBUTES', 'SHADING_LANGUAGE_VERSION', 'CURRENT_PROGRAM', 'NEVER', 'LESS', 'EQUAL', 'LEQUAL', 'GREATER', 'NOTEQUAL', 'GEQUAL', 'ALWAYS', 'KEEP', 'REPLACE', 'INCR', 'DECR', 'INVERT', 'INCR_WRAP', 'DECR_WRAP', 'VENDOR', 'RENDERER', 'VERSION', 'NEAREST', 'LINEAR', 'NEAREST_MIPMAP_NEAREST', 'LINEAR_MIPMAP_NEAREST', 'NEAREST_MIPMAP_LINEAR', 'LINEAR_MIPMAP_LINEAR', 'TEXTURE_MAG_FILTER', 'TEXTURE_MIN_FILTER', 'TEXTURE_WRAP_S', 'TEXTURE_WRAP_T', 'TEXTURE', 'TEXTURE_CUBE_MAP', 'TEXTURE_BINDING_CUBE_MAP', 'TEXTURE_CUBE_MAP_POSITIVE_X', 'TEXTURE_CUBE_MAP_NEGATIVE_X', 'TEXTURE_CUBE_MAP_POSITIVE_Y', 'TEXTURE_CUBE_MAP_NEGATIVE_Y', 'TEXTURE_CUBE_MAP_POSITIVE_Z', 'TEXTURE_CUBE_MAP_NEGATIVE_Z', 'MAX_CUBE_MAP_TEXTURE_SIZE', 'TEXTURE0', 'TEXTURE1', 'TEXTURE2', 'TEXTURE3', 'TEXTURE4', 'TEXTURE5', 'TEXTURE6', 'TEXTURE7', 'TEXTURE8', 'TEXTURE9', 'TEXTURE10', 'TEXTURE11', 'TEXTURE12', 'TEXTURE13', 'TEXTURE14', 'TEXTURE15', 'TEXTURE16', 'TEXTURE17', 'TEXTURE18', 'TEXTURE19', 'TEXTURE20', 'TEXTURE21', 'TEXTURE22', 'TEXTURE23', 'TEXTURE24', 'TEXTURE25', 'TEXTURE26', 'TEXTURE27', 'TEXTURE28', 'TEXTURE29', 'TEXTURE30', 'TEXTURE31', 'ACTIVE_TEXTURE', 'REPEAT', 'CLAMP_TO_EDGE', 'MIRRORED_REPEAT', 'FLOAT_VEC2', 'FLOAT_VEC3', 'FLOAT_VEC4', 'INT_VEC2', 'INT_VEC3', 'INT_VEC4', 'BOOL', 'BOOL_VEC2', 'BOOL_VEC3', 'BOOL_VEC4', 'FLOAT_MAT2', 'FLOAT_MAT3', 'FLOAT_MAT4', 'SAMPLER_2D', 'SAMPLER_CUBE', 'VERTEX_ATTRIB_ARRAY_ENABLED', 'VERTEX_ATTRIB_ARRAY_SIZE', 'VERTEX_ATTRIB_ARRAY_STRIDE', 'VERTEX_ATTRIB_ARRAY_TYPE', 'VERTEX_ATTRIB_ARRAY_NORMALIZED', 'VERTEX_ATTRIB_ARRAY_POINTER', 'VERTEX_ATTRIB_ARRAY_BUFFER_BINDING', 'COMPILE_STATUS', 'LOW_FLOAT', 'MEDIUM_FLOAT', 'HIGH_FLOAT', 'LOW_INT', 'MEDIUM_INT', 'HIGH_INT', 'FRAMEBUFFER', 'RENDERBUFFER', 'RGBA4', 'RGB5_A1', 'RGB565', 'DEPTH_COMPONENT16', 'STENCIL_INDEX', 'STENCIL_INDEX8', 'DEPTH_STENCIL', 'RENDERBUFFER_WIDTH', 'RENDERBUFFER_HEIGHT', 'RENDERBUFFER_INTERNAL_FORMAT', 'RENDERBUFFER_RED_SIZE', 'RENDERBUFFER_GREEN_SIZE', 'RENDERBUFFER_BLUE_SIZE', 'RENDERBUFFER_ALPHA_SIZE', 'RENDERBUFFER_DEPTH_SIZE', 'RENDERBUFFER_STENCIL_SIZE', 'FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE', 'FRAMEBUFFER_ATTACHMENT_OBJECT_NAME', 'FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL', 'FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE', 'COLOR_ATTACHMENT0', 'DEPTH_ATTACHMENT', 'STENCIL_ATTACHMENT', 'DEPTH_STENCIL_ATTACHMENT', 'NONE', 'FRAMEBUFFER_COMPLETE', 'FRAMEBUFFER_INCOMPLETE_ATTACHMENT', 'FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT', 'FRAMEBUFFER_UNSUPPORTED', 'FRAMEBUFFER_BINDING', 'RENDERBUFFER_BINDING', 'MAX_RENDERBUFFER_SIZE', 'INVALID_FRAMEBUFFER_OPERATION', 'UNPACK_FLIP_Y_WEBGL', 'UNPACK_PREMULTIPLY_ALPHA_WEBGL', 'CONTEXT_LOST_WEBGL', 'UNPACK_COLORSPACE_CONVERSION_WEBGL', 'BROWSER_DEFAULT_WEBGL']
items +=['ES_VERSION_2_0', 'NUM_COMPRESSED_TEXTURE_FORMATS', 'FIXED', 'ACTIVE_UNIFORM_MAX_LENGTH', 'ACTIVE_ATTRIBUTE_MAX_LENGTH', 'EXTENSIONS', 'IMPLEMENTATION_COLOR_READ_TYPE', 'IMPLEMENTATION_COLOR_READ_FORMAT', 'INFO_LOG_LENGTH', 'SHADER_SOURCE_LENGTH', 'SHADER_COMPILER', 'FRAMEBUFFER_INCOMPLETE_DIMENSIONS']


for i in items:
    FieldDoc( "WebGLRenderingContext." + i, "Constant name for " + i,
        [SeeDoc( "WebGLRenderingContext" ) ],
        NO_Examples,
        IS_Static, IS_Public, IS_Readonly,
        "integer",
        NO_Default
    )

FunctionDoc( "WebGLRenderingContext.isContextLost", "Check if the context has been lost.",
    SeesDocs( "WebGLRenderingContext.isContextLost|WebGLRenderingContext.useProgram|WebGLRenderingContext.getError|WebGLRenderingContext.viewport"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "true", "boolean" )
)

FunctionDoc( "WebGLRenderingContext.getExtension", "Get an WebGL extension.",
    SeesDocs( "WebGLRenderingContext.isContextLost|WebGLRenderingContext.useProgram|WebGLRenderingContext.getError|WebGLRenderingContext.viewport"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "null", "void" )
)

FunctionDoc( "WebGLRenderingContext.activeTexture", "Call an texture.",
    SeesDocs( "WebGLRenderingContext.activeTexture|WebGLRenderingContext.createTexture|WebGLRenderingContext.deleteTexture"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "texture", "texture settings", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.attachShader", "Attach a shader.",
    SeesDocs( "WebGLRenderingContext.detachShader|WebGLRenderingContext.attachShader"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "program", "Program settings", "WebGLProgram", NO_Default, IS_Obligated ),
     ParamDoc( "shader", "Shader settings", "WebGLShader", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bindAttribLocation", "Set attributes.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "program", "Program settings", "WebGLProgram", NO_Default, IS_Obligated ),
     ParamDoc( "vertex", "Vertex", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "name", "Name", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bindBuffer", "Set a buffer.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "buffer", "Buffer", "WebGLBuffer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bindFramebuffer", "Set a framebuffer.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "buffer", "Buffer", "WebGLFramebuffer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bindRenderbuffer", "Set a renderbuffer.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "buffer", "Buffer", "WebGLRenderbuffer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bindTexture", "Set a texture.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "texture", "Texture", "WebGLTexture", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.copyTexImage2D", "Copy an texture.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "level", "Level", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "internal", "Internal format", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "x_pos", "X position", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "y_pos", "Y position", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "height", "Height", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "border", "Border", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.copyTexSubImage2D", "Copy an texture.",
    SeesDocs( "WebGLRenderingContext.bindAttribLocation|WebGLRenderingContext.bindBuffer|WebGLRenderingContext.bindFrameBuffer|WebGLRenderingContext.bindRenderBuffer|WebGLRenderingContext.bindTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "level", "Level", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "internal", "Internal format", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "x_pos", "X position", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "y_pos", "Y position", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "height", "Height", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "border", "Border", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.blendEquation", "Blend an equation.",
    SeesDocs( "WebGLRenderingContext.blendEquation|WebGLRenderingContext.blendEquationSeperate|WebGLRenderingContext.blendFunc|WebGLRenderingContext.blendFuncSeperate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "mode", "Mode", "integer", NO_Default, IS_Obligated ) ] ,
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.blendEquationSeparate", "Blend a separate equation.",
    SeesDocs( "WebGLRenderingContext.blendEquation|WebGLRenderingContext.blendEquationSeperate|WebGLRenderingContext.blendFunc|WebGLRenderingContext.blendFuncSeperate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "rgb", "Mode RGB", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "alpha", "Mode Alpha", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.blendFunc", "Blend a function.",
    SeesDocs( "WebGLRenderingContext.blendEquation|WebGLRenderingContext.blendEquationSeperate|WebGLRenderingContext.blendFunc|WebGLRenderingContext.blendFuncSeperate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "sfactor", "S-factor", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "dfactor", "D-factor", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.blendFuncSeparate", "Blend a separate function.",
    SeesDocs( "WebGLRenderingContext.blendEquation|WebGLRenderingContext.blendEquationSeperate|WebGLRenderingContext.blendFunc|WebGLRenderingContext.blendFuncSeperate" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "srcRgb", "RGB Source", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "dstRgb", "RGB Destination", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "srcAlpha", "Alpha Source", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "dstAlpha", "Alpha Destination", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bufferData", "Buffer data.",
    SeesDocs( "WebGLRenderingContext.bufferData|WebGLRenderingContext.bufferSubData" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "array", "Buffer", "WebGLBuffer", NO_Default, IS_Obligated ),
        ParamDoc( "usage", "Usage", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.bufferSubData", "Buffer sub data.",
    SeesDocs( "WebGLRenderingContext.bufferData|WebGLRenderingContext.bufferSubData" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "offset", "Offset", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "array", "Buffer", "WebGLBuffer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.clear", "Clears a context.",
    SeesDocs( "WebGLRenderingContext.clean|WebGLRenderingContext.clearColor|WebGLRenderingContext.clearDepth|clearColor|WebGLRenderingContext.clearStencil" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.clearColor", "Clears a color in a context.",
    SeesDocs( "WebGLRenderingContext.clean|WebGLRenderingContext.clearColor|WebGLRenderingContext.clearDepth|clearColor|WebGLRenderingContext.clearStencil" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "red", "Red value", "float", NO_Default, IS_Obligated ),
        ParamDoc( "green", "Green value", "float", NO_Default, IS_Obligated ),
        ParamDoc( "blue", "Blue value", "float", NO_Default, IS_Obligated ),
        ParamDoc( "alpha", "Alpha value", "float", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.clearDepth", "Clears a depth in a context.",
    SeesDocs( "WebGLRenderingContext.clean|WebGLRenderingContext.clearColor|WebGLRenderingContext.clearDepth|clearColor|WebGLRenderingContext.clearStencil" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "depth", "depth size", "float", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.clearStencil", "Clears a stencil in a context.",
    SeesDocs( "WebGLRenderingContext.clean|WebGLRenderingContext.clearColor|WebGLRenderingContext.clearDepth|clearColor|WebGLRenderingContext.clearStencil" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "stencil", "Stencil", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.colorMask", "Mask a color in a context.",
    SeesDocs( "WebGLRenderingContext.clearColor" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "red", "Red value", "float", NO_Default, IS_Optional ),
        ParamDoc( "green", "Green value", "float", NO_Default, IS_Optional ),
        ParamDoc( "blue", "Blue value", "float", NO_Default, IS_Optional ),
        ParamDoc( "alpha", "Alpha value", "float", NO_Default, IS_Optional )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.compileShader", "Compile a shader.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "shader", "Shader", "WebGLShader", NO_Default, IS_Optional ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.createBuffer", "Create a buffer.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "buffer", "WebGLBuffer" )
)

FunctionDoc( "WebGLRenderingContext.createFrameBuffer", "Create a Framebuffer.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "buffer", "WebGLFramebuffer" )
)

FunctionDoc( "WebGLRenderingContext.createRenderBuffer", "Create a Renderbuffer.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "buffer", "WebGLRenderbuffer" )
)

FunctionDoc( "WebGLRenderingContext.createProgram", "Create a Program.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "program", "WebGLProgram" )
)

FunctionDoc( "WebGLRenderingContext.createShader", "Create a Shader.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "type", "shader type", "integer", NO_Default, IS_Obligated ) ],
    ReturnDoc( "shader", "WebGLShader" )
)

FunctionDoc( "WebGLRenderingContext.createTexture", "Create a Texture.",
    SeesDocs( "WebGLRenderingContext.compileShader|WebGLRenderingContext.texImage2D|WebGLRenderingContext.createBuffer|WebGLRenderingContext.createFramebuffer|WebGLRenderingContext.createRenderbuffer|WebGLRenderingContext.createProgram|WebGLRenderingContext.createShader|WebGLRenderingContext.createTexture" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "texture", "WebGLTexture" )
)

FunctionDoc( "WebGLRenderingContext.cullFace", "Cullface.",
    SeesDocs( "WebGLRenderingContext.cullFace|WebGLRenderingContext.frontFace" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "mode", "CullFace mode", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

items = { "WebGLRenderingContext.deleteBuffer": "WebGLBuffer", "WebGLRenderingContext.deleteFramebuffer": "WebGLFramebuffer", "WebGLRenderingContext.deleteRenderbuffer": "WebGLRenderbuffer", "WebGLRenderingContext.deleteProgram": "WebGLProgram", "WebGLRenderingContext.deleteShader": "WebGLShader", "WebGLRenderingContext.deleteTexture": "WebGLTexture" }
for i, typed in items.items():
    FunctionDoc( i, "Delete a " + i[22: ]+ " from a context.",
        SeesDocs( "WebGLRenderingContext.deleteBuffer|WebGLRenderingContext.deleteFramebuffer|WebGLRenderingContext.deleteRenderbuffer|WebGLRenderingContext.deleteProgram|WebGLRenderingContext.deleteShader|WebGLRenderingContext.deleteTexture" ),
        NO_Examples,
        IS_Dynamic, IS_Public, IS_Fast,
        [ ParamDoc( "Object", typed + " instance", typed, NO_Default, IS_Obligated ) ],
        NO_Returns
    )

FunctionDoc( "WebGLRenderingContext.depthFunc", "Start a function on a depth.",
    SeesDocs( "WebGLRenderingContext.depthFunc|WebGLRenderingContext.depthMask|WebGLRenderingContext.depthShader" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "func", "Function", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.depthMask", "Start a mask on a depth.",
    SeesDocs( "WebGLRenderingContext.depthFunc|WebGLRenderingContext.depthMask|WebGLRenderingContext.depthShader" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "flag", "Flag", "boolean", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.depthRange", "Start a range on a depth.",
    SeesDocs( "WebGLRenderingContext.depthFunc|WebGLRenderingContext.depthMask|WebGLRenderingContext.depthShader" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "near", "Near", "float", NO_Default, IS_Obligated ),
      ParamDoc( "far", "Far", "float", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.detachShader", "Remove a shader.",
    SeesDocs( "WebGLRenderingContext.detachShader|WebGLRenderingContext.attachShader" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ),
      ParamDoc( "shader", "Shader", "WebGLShader", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.disable", "Disable a context.",
    SeesDocs( "WebGLRenderingContext.disable|WebGLRenderingContext.enable|WebGLRenderingContext.disableVertexAttribArray|WebGLRenderingContext.enableVertexAttribArray" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "cap", "Cap", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.disableVertexAttribArray", "Disable a vertex array.",
    SeesDocs( "WebGLRenderingContext.disable|WebGLRenderingContext.enable|WebGLRenderingContext.disableVertexAttribArray|WebGLRenderingContext.enableVertexAttribArray" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "cap", "Cap", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.drawArrays", "Draw Arrays.",
    SeesDocs( "WebGLRenderingContext.drawArrays|WebGLRenderingContext.drawElements" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "mode", "Mode", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "first", "First", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "count", "Count", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.drawElements", "Draw Elements.",
    SeesDocs( "WebGLRenderingContext.drawArrays|WebGLRenderingContext.drawElements" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "mode", "Mode", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "first", "First", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "count", "Count", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "offset", "Offset", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.enable", "Enable a context.",
    SeesDocs( "WebGLRenderingContext.disable|WebGLRenderingContext.enable|WebGLRenderingContext.disableVertexAttribArray|WebGLRenderingContext.enableVertexAttribArray" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "bits", "Bits", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.enableVertexAttribArray", "Enable a vertex array.",
    SeesDocs( "WebGLRenderingContext.disable|WebGLRenderingContext.enable|WebGLRenderingContext.disableVertexAttribArray|WebGLRenderingContext.enableVertexAttribArray" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "attr", "Attributes", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.finish", "Finish a Context.",
    SeesDocs( "WebGLRenderingContext.finish|WebGLRenderingContext.flush" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.flush", "Flush a Context.",
    SeesDocs( "WebGLRenderingContext.finish|WebGLRenderingContext.flush" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.getUniformLocation", "Get a location in a Context.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ) ,
        ParamDoc( "name", "Name", "string", NO_Default, IS_Obligated )
    ],
    ReturnDoc( "location", "WebGLUniformLocation" )
)

FunctionDoc( "WebGLRenderingContext.getShaderPrecisionFormat", "Get the precision of a shader.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "shader", "Shadertype", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "precision", "Precisiontype", "integer", NO_Default, IS_Obligated )
    ],
    ReturnDoc( "format", "WebGLShaderPrecisionFormat" )
)

items = [ "rangeMin", "rangeMax", "precision"]
for i in items:
    FieldDoc( "WebGLShaderPrecisionFormat." + i, "The " + i + " value of the ShaderPrecisionFormat.",
        SeesDocs( "WebGLShaderPrecisionFormat." + "|WebGLShaderPrecisionFormat." .join( items ) ),
        NO_Examples,
        IS_Static, IS_Public, IS_ReadWrite,
        "integer",
        NO_Default
    )

FunctionDoc( "WebGLRenderingContext.framebufferRenderbuffer", "Foo's the bar's?",
    SeesDocs( "WebGLRenderingContext.framebufferRenderbuffer|WebGlRenderingContext.framebufferTexture2D" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "attach", "Attachment", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "renderbuffer_target", "Renderbuffer target", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "renderbuffer", "Renderbuffer target", "WebGLRenderbuffer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.framebufferTexture2D", "Bar's the Foo's?",
    SeesDocs( "WebGLRenderingContext.framebufferRenderbuffer|WebGlRenderingContext.framebufferTexture2D" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "attach", "Attachment", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "texture_target", "Texture target", "integer", NO_Default, IS_Obligated ) ,
        ParamDoc( "texture", "Texture target", "WebGLTexture", NO_Default, IS_Obligated ),
        ParamDoc( "level", "level", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.frontFace", "Frontface.",
    SeesDocs( "WebGLRenderingContext.cullFace|WebGLRenderingContext.frontFace" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "mode", "FrontFace mode", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.generateMipmap", "Generate a Mipmap.",
    SeesDocs( "WebGLRenderingContext.generateMipamp" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

items = { "size": "integer", "type": "string", "name": "string" }
for i, typed in items.items():
    FieldDoc( "WebGLActiveInfo." + i, "The " + i + " of the Object instance.",
        SeesDocs( "WebGLActiveInfo.size|WebGLActiveInfo.type|WebGLActiveInfo.name" ),
        NO_Examples,
        IS_Static, IS_Public, IS_ReadWrite,
        typed,
        NO_Default
    )

FunctionDoc( "WebGLRenderingContext.getActiveAttrib", "Get the active attributes of this context.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ) ,
        ParamDoc( "index", "Attribute index", "integer", NO_Default, IS_Obligated )
    ],
    ReturnDoc( "Information about the attributes", "WebGLActiveInfo" )
)

FunctionDoc( "WebGLRenderingContext.getActiveUniform", "Get the active uniform location of this context.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.getAttribLocation", "Get the location of a attribute.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ) ,
        ParamDoc( "attr", "Attribute", "string", NO_Default, IS_Obligated )
    ],
    ReturnDoc( "location", "WebGLUniformLocation" )
)

FunctionDoc( "WebGLRenderingContext.getParameter", "Get parameter of a context.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "attr", "Attribute", "integer", NO_Default, IS_Obligated ) ],
    ReturnDoc( "value", "any" )
)

FunctionDoc( "WebGLRenderingContext.getParameter", "Get parameter of a program.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ),
        ParamDoc( "attr", "Attribute", "integer", NO_Default, IS_Obligated )
    ],
    ReturnDoc( "value", "any" )
)

FunctionDoc( "WebGLRenderingContext.getProgramInfoLog", "Get information about a program.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ) ],
    ReturnDoc( "log", "string" )
)

FunctionDoc( "WebGLRenderingContext.getShaderParameter", "Get parameter from a shader.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "shader", "Shader", "WebGLShader", NO_Default, IS_Obligated ),
        ParamDoc( "param", "Parameter name", "integer", NO_Default, IS_Obligated )
    ],
    ReturnDoc( "value", "integer" )
)

FunctionDoc( "WebGLRenderingContext.getShaderInfoLog", "Get information about a shader.",
    SeesDocs( "WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getActiveAttrib|WebGLRenderingContext.getActiveUniform|WebGLRenderingContext.getAttribLocation|WebGLRenderingContext.getParameter|WebGLRenderingContext.getProgramParameter|WebGLRenderingContext.getProgramInfoLog|WebGLRenderingContext.getShaderParameter|WebGLRenderingContext.getShaderInfoLog|WebGLRenderingContext.getUniformLocation|WebGLRenderingContext.getShaderPrecisionFormat" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "shader", "Shader", "WebGLShader", NO_Default, IS_Obligated ) ],
    ReturnDoc( "log", "string" )
)

FunctionDoc( "WebGLRenderingContext.lineWidth", "Set the width of a line.",
    SeesDocs( "WebGLRenderingContext.lineWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "width", "Width size", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.linkProgram", "Link a program to the context.",
    SeesDocs( "WebGLRenderingContext.linkProgram" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.pixelStorei", "Store a parameter.",
    SeesDocs( "WebGLRenderingContext.pixelStorei" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "param", "parameter", "integer", NO_Default, IS_Obligated ),
    ParamDoc( "value", "The value to store", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.renderBufferStorage", "Render a buffer.",
    SeesDocs( "WebGLRenderingContext.renderBufferStorage" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "internalFormat", "Internal format", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "height", "Height", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.shaderSource", "Set the source on a shader.",
    SeesDocs( "WebGLRenderingContext.shaderSource" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "shader", "Shader", "WebGLShader", NO_Default, IS_Obligated ),
        ParamDoc( "source", "Source", "string", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

# TODO: investigate how to describe the 2nd parameter list
FunctionDoc( "WebGLRenderingContext.texImage2D", "Texture of an image.",
    SeesDocs( "WebGLRenderingContext.texImage2D" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "level", "Level", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "internalFormat", "Internal format", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "height", "Height", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "border", "Border", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "format", "Format", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "type", "Type", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.texParameteri", "Set a parameter value on a Texture.",
    SeesDocs( "texImage2D|WebGLRenderingContext.texParameteri" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "target", "Target", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "name", "Parameter name", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "value", "Value", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.vertexAttribPointer", "Create a pointer to an attribute.",
    SeesDocs( "WebGLRenderingContext.vertextAttribPointer" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "attr", "Attribute", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "size", "Size", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "type", "Type", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "normalized", "Normalized", "boolean", NO_Default, IS_Obligated ),
        ParamDoc( "stride", "Stride", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "offset", "Offset", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.useProgram", "Use a certain program.",
    SeesDocs( "WebGLRenderingContext.isContextLost|WebGLRenderingContext.useProgram|WebGLRenderingContext.getError|WebGLRenderingContext.viewport"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "program", "Program", "WebGLProgram", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.viewPort", "Set the viewport to a certain position.",
    SeesDocs( "WebGLRenderingContext.isContextLost|WebGLRenderingContext.useProgram|WebGLRenderingContext.getError|WebGLRenderingContext.viewport"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "x_pos", "X position", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "y_pos", "Y position", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "width", "width size", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "height", "Height size", "integer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc( "WebGLRenderingContext.getError", "Retrieve the most recent error.",
    SeesDocs( "WebGLRenderingContext.isContextLost|WebGLRenderingContext.useProgram|WebGLRenderingContext.getError|WebGLRenderingContext.viewport"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "Errorcode", "integer")
)

ConstructorDoc( "WebGLRenderingContext", "Constructs an WebGLRenderingContext.",
    NO_Sees,
    NO_Examples,
    NO_Params,
    ReturnDoc( "Context", "WebGLRenderingContext" )
)

