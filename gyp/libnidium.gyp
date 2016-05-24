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
            'third-party/angle.gyp:*',
        ],
        'include_dirs': [
            '<(nidium_src_path)',
            '<(nidium_interface_path)/',
            '<(nidium_network_path)/',
            '<(nidium_av_path)',
            '<(third_party_path)/skia/',
            '<(third_party_path)/skia/include/core',
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
            '<(third_party_path)/skia/include/effects',
            '<(third_party_path)/skia/include/utils/mac',
            '<(third_party_path)/skia/include/lazy',
            '<(third_party_path)/libzip/lib',
            '<(third_party_path)/rapidxml',
            '<(third_party_path)/ffmpeg/',
            '<(third_party_path)/libcoroutine/source/',
            '<(third_party_path)/basekit/source/',
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
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-Qunused-arguments',
                        '-fvisibility=hidden',
                        '-fvisibility-inlines-hidden',
                        '-Wno-c++0x-extensions',
                        '-Wno-unused-function',
                        '-Wno-invalid-offsetof'
                    ],
                },
            }],
            ['OS=="linux"', {
                'defines+': [
                    'EXPORT_JS_API',
                    'IMPL_MFBT',
                    'USE_SYSTEM_MALLOC=1',
                    'ENABLE_ASSEMBLER=1',
                    'ENABLE_JIT=1',
                    'EXPORT_JS_API',
                    'IMPL_MFBT',
                    'SK_GAMMA_SRGB',
                    'SK_GAMMA_APPLY_TO_A8',
                    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1',
                    'SK_SCALAR_IS_FLOAT',
                    'SK_CAN_USE_FLOAT',
                    'SK_SUPPORT_GPU=1',
                    'SK_SAMPLES_FOR_X',
                    'SK_BUILD_FOR_UNIX',
                    'SK_USE_POSIX_THREADS',
                    'SK_SUPPORT_PDF',
                    'GR_LINUX_BUILD=1',
                    'SK_RELEASE',
                    'UINT32_MAX=4294967295u',
                    'GR_RELEASE=1',
                    '__STDC_CONSTANT_MACROS',
                    'TRACING',
                    'JS_THREADSAFE',
                    'GR_GL_CUSTOM_SETUP_HEADER=<../patch/skia_gl_config.h>'
                ],
                'cflags+': [
                    '-fno-rtti',
                    '-fvisibility=hidden',
                    '-fvisibility-inlines-hidden',
                    '-Wno-c++0x-extensions',
                    '-std=c++11',
                    '-Wno-invalid-offsetof'
                ],
            }],
            ['nidium_audio==1', {
                'sources': [
                    '<(nidium_src_path)/Binding/JSAV.cpp',
                 ],
                 'defines': [ 'NIDIUM_AUDIO_ENABLED' ],
                 'includes': [
                    'av.gyp'
                 ]
            }],
            ['nidium_webgl==1', {
                'sources': [
                    '<(nidium_src_path)/Binding/JSWebGL.cpp',
                 ],
                 'defines+': [ 'NIDIUM_WEBGL_ENABLED' ],
            }],
        ],
        'sources': [
            '<(nidium_src_path)/Frontend/App.cpp',
            '<(nidium_src_path)/Frontend/NML.cpp',
            '<(nidium_src_path)/Frontend/Assets.cpp',
            '<(nidium_src_path)/Frontend/Context.cpp',
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
        ],
    }],
}

