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
            '<(third_party_path)/rapidxml',
            '<(third_party_path)/ffmpeg/',
            '<(third_party_path)/angle/include/'
        ],

        'conditions': [
            ['OS=="mac"', {
                'defines': [
                    'ENABLE_TYPEDARRAY_MOVE',
                    'ENABLE_YARR_JIT=1',
                    'NO_NSPR_10_SUPPORT',
                    'IMPL_MFBT EXPORT_JS_API',
                    'MOZILLA_CLIENT',
                    'SK_GAMMA_SRGB',
                    'SK_GAMMA_APPLY_TO_A8',
                    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1',
                    'SK_SCALAR_IS_FLOAT',
                    'SK_CAN_USE_FLOAT',
                    'SK_SUPPORT_GPU=1',
                    'SK_BUILD_FOR_MAC',
                    'SK_FONTHOST_USES_FONTMGR',
                    'SK_USE_POSIX_THREADS',
                    'GR_MAC_BUILD=1',
                    'SK_SUPPORT_PDF',
                    'SK_RELEASE',
                    'GR_RELEASE=1',
                    'TRACING',
                    'JS_THREADSAFE',
                    'GR_GL_CUSTOM_SETUP_HEADER=<../patch/skia_gl_config.h>'
                ],
            }],
            ['OS=="linux"', {
                'defines': [
                    'EXPORT_JS_API',
                    'IMPL_MFBT',
                    'USE_SYSTEM_MALLOC=1',
                    'ENABLE_ASSEMBLER=1',
                    'ENABLE_JIT=1',
                    'EXPORT_JS_API',
                    'IMPL_MFBT',
                    'SK_RELEASE',
                    'SK_SAMPLES_FOR_X',
                    'SK_GAMMA_APPLY_TO_A8',
                    'SK_INTERNAL',
                    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=0',
                    'SK_ENABLE_DISCRETE_GPU',
                    'SKIA_IMPLEMENTATION=1',
                    'SK_HAS_JPEG_LIBRARY',
                    'SK_SUPPORT_PDF',
                    'SK_PDF_USE_SFNTLY',
                    'SK_HAS_PNG_LIBRARY',
                    'SK_CODEC_DECODES_RAW',
                    'SK_RASTER_PIPELINE_HAS_JIT',
                    'SK_HAS_WEBP_LIBRARY',
                    'SK_XML',

                    'UINT32_MAX=4294967295u',
                    '__STDC_CONSTANT_MACROS',
                    'TRACING',
                    'JS_THREADSAFE',
                    'GR_GL_CUSTOM_SETUP_HEADER=<../patch/skia_gl_config.h>'
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
            '<(nidium_src_path)/IO/SystemStream.cpp'
        ],
    }],
}

