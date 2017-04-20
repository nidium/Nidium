# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'libnidium',
        'type': 'static_library',
        'dependencies': [
            '<(nidium_network_path)/gyp/network.gyp:network-includes',
            'libnidiumcore.gyp:libnidiumcore-includes',
        ],
        'include_dirs': [
            '<(nidium_src_path)',
            '<(nidium_interface_path)/',
            '<(nidium_network_path)/',
            '<(nidium_av_path)',
            '<(third_party_path)/skia/',
            '<(third_party_path)/skia/include/core',
            '<(third_party_path)/skia/include/private',
            '<(third_party_path)/skia/include/pipe',
            '<(third_party_path)/skia/include/gpu',
            '<(third_party_path)/skia/include/utils',
            '<(third_party_path)/skia/include/device',
            '<(third_party_path)/skia/include/config',
            '<(third_party_path)/skia/include/images',
            '<(third_party_path)/skia/include/test',
            '<(third_party_path)/skia/include/pdf',
            '<(third_party_path)/skia/src/gpu/gl',
            '<(third_party_path)/skia/src/gpu',
            '<(third_party_path)/skia/src/effects',
            '<(third_party_path)/skia/src/core',
            '<(third_party_path)/skia/src/utils',
            '<(third_party_path)/skia/include/effects',
            '<(third_party_path)/skia/include/utils/mac',
            '<(third_party_path)/skia/include/lazy',
            '<(third_party_path)/libzip/lib',
            '<(third_party_path)/libzip',
            '<(third_party_path)/rapidxml',
            '<(third_party_path)/ffmpeg/',
            '<(third_party_path)/libcoroutine/source/',
            '<(third_party_path)/basekit/source/',
            '<(third_party_path)/angle/include/',
            '<(third_party_path)/yoga/',
        ],
        'defines': [
            'EXPORT_JS_API',
            'GR_GL_CUSTOM_SETUP_HEADER=<../patch/skia_gl_config.h>'
            'IMPL_MFBT',
            'JS_THREADSAFE',
            'SK_GAMMA_APPLY_TO_A8',
            'SK_RELEASE',
            'SK_SUPPORT_PDF',
            'TRACING',
        ],
        'msvs_settings': {
            'VCCLCompilerTool': {
                'AdditionalOptions': [
                ]
            },
        },
        'conditions': [
            ['OS=="win"', {
                'include_dirs': [
                    '<(third_party_path)/opengl/api/',
                 ],
                'link_settings': {
                    'libraries': [
                    ]
                },
                'defines': [
                    'WIN32',
                    '_CONSOLE',
                    '_LIB',
                    'SK_INTERNAL',
                    'SK_GAMMA_SRGB',
                    'SK_GAMMA_APPLY_TO_A8',
                    'SK_SCALAR_TO_FLOAT_EXCLUDED',
                    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1',
                    'SK_SUPPORT_GPU=1',
                    'SK_SUPPORT_OPENCL=0',
                    'SK_FORCE_DISTANCE_FIELD_TEXT=0',
                    'SK_BUILD_FOR_WIN32',
                    'GR_GL_FUNCTION_TYPE=__stdcall',
                    'SK_DEVELOPER=1',

                    '__MINGW32__',
                    'NOMINMAX', 
                    'SK_USE_POSIX_THREADS'
                    '__STDC_CONSTANT_MACROS',
                 ]
            }],
            ['OS=="mac"', {
                'defines': [
                    'ENABLE_TYPEDARRAY_MOVE',
                    'ENABLE_YARR_JIT=1',
                    'GR_MAC_BUILD=1',
                    'GR_RELEASE=1',
                    'MOZILLA_CLIENT',
                    'NO_NSPR_10_SUPPORT',
                    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1',
                    'SK_BUILD_FOR_MAC',
                    'SK_CAN_USE_FLOAT',
                    'SK_FONTHOST_USES_FONTMGR',
                    'SK_GAMMA_SRGB',
                    'SK_SCALAR_IS_FLOAT',
                    'SK_SUPPORT_GPU=1',
                    'SK_USE_POSIX_THREADS'
                ],
            }],
            ['OS=="linux"', {
                'defines': [
                    '__STDC_CONSTANT_MACROS',
                    'ENABLE_ASSEMBLER=1',
                    'ENABLE_JIT=1',
                    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=0',
                    'SK_CODEC_DECODES_RAW',
                    'SK_ENABLE_DISCRETE_GPU',
                    'SK_HAS_JPEG_LIBRARY',
                    'SK_HAS_PNG_LIBRARY',
                    'SK_HAS_WEBP_LIBRARY',
                    'SK_INTERNAL',
                    'SK_PDF_USE_SFNTLY',
                    'SK_RASTER_PIPELINE_HAS_JIT',
                    'SK_SAMPLES_FOR_X',
                    'SK_XML',
                    'SKIA_IMPLEMENTATION=1',
                    'UINT32_MAX=4294967295u',
                    'USE_SYSTEM_MALLOC=1',
                ],
            }],
            ['nidium_audio==1', {
                'sources': [
                    '<(nidium_src_path)/Binding/JSAV.cpp',
                    '<(nidium_src_path)/Binding/JSAudio.cpp',
                    '<(nidium_src_path)/Binding/JSAudioContext.cpp',
                    '<(nidium_src_path)/Binding/JSAudioNode.cpp',
                    '<(nidium_src_path)/Binding/JSVideo.cpp',
                 ],
                 'defines': [ 'NIDIUM_AUDIO_ENABLED' ],
                     'includes': [
                        'av.gypi'
                     ]
            }],
            ['nidium_webgl==1', {
                'sources': [
                    '<(nidium_src_path)/Binding/JSWebGL.cpp',
                 ],
                 'defines': [ 'NIDIUM_WEBGL_ENABLED' ],
            }],
            ['nidium_gl_debug==1', {
                 'defines': [ 'NIDIUM_ENABLE_GL_ERROR' ],
            }],
        ],
        'sources': [
            '<(nidium_src_path)/Frontend/App.cpp',
            '<(nidium_src_path)/Frontend/NML.cpp',
            '<(nidium_src_path)/Frontend/Assets.cpp',
            '<(nidium_src_path)/Frontend/Context.cpp',
            '<(nidium_src_path)/Frontend/InputHandler.cpp',
            '<(nidium_src_path)/Graphics/Gradient.cpp',
            '<(nidium_src_path)/Graphics/Image.cpp',
            '<(nidium_src_path)/Graphics/ShadowLooper.cpp',
            '<(nidium_src_path)/Graphics/GLResources.cpp',
            '<(nidium_src_path)/Graphics/GLState.cpp',
            '<(nidium_src_path)/Graphics/CanvasContext.cpp',
            '<(nidium_src_path)/Graphics/CanvasHandler.cpp',
            '<(nidium_src_path)/Graphics/Canvas3DContext.cpp',
            '<(nidium_src_path)/Graphics/SkiaContext.cpp',
            '<(nidium_src_path)/Binding/JSImage.cpp',
            '<(nidium_src_path)/Binding/JSWindow.cpp',
            '<(nidium_src_path)/Binding/JSDocument.cpp',
            '<(nidium_src_path)/Binding/JSCanvas.cpp',
            '<(nidium_src_path)/Binding/JSCanvas2DContext.cpp',
            '<(nidium_src_path)/Binding/JSNML.cpp',
            '<(nidium_src_path)/IO/SystemStream.cpp',
            '<(third_party_path)/yoga/Yoga.c',
            '<(third_party_path)/yoga/YGNodeList.c',
            '<(third_party_path)/yoga/YGNodeList.c',
            '<(third_party_path)/yoga/YGEnums.c',
            '<(third_party_path)/yoga/YGStringEnums.c',
        ],
    }],
}

