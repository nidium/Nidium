{
    'targets': [{
        'target_name': 'nativestudio',
        'type': 'static_library',
        'dependencies': [
            '<(native_network_path)/gyp/network.gyp:nativenetwork-includes',
            '<(native_nativejscore_path)/gyp/nidiumcore.gyp:nativejscore-includes',
            '<(native_nativejscore_path)/gyp/jsoncpp.gyp:jsoncpp',
            'angle.gyp:*',
        ],
        'include_dirs': [
            '<(native_src_path)',
            '<(native_interface_path)/',
            '<(native_nativejscore_path)/src/',
            '<(native_network_path)/',
            '<(native_av_path)',
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
                    'GR_GL_CUSTOM_SETUP_HEADER=<../gyp/skia_gl_config.h>'
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
                    'GR_GL_CUSTOM_SETUP_HEADER=<../gyp/skia_gl_config.h>'
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
            ['native_audio==1', {
                'sources': [
                    '<(native_src_path)/JSAV.cpp',
                 ],
                 'defines+': [ 'NATIVE_AUDIO_ENABLED' ],
                 'dependencies': [
                    'av.gyp:nativeav'
                 ]
            }],
            ['native_webgl==1', {
                'sources': [
                    '<(native_src_path)/JSWebGL.cpp',
                 ],
                 'defines+': [ 'NATIVE_WEBGL_ENABLED' ],
            }],
        ],
        'sources': [
            '<(native_src_path)/Skia.cpp',
            '<(native_src_path)/SkGradient.cpp',
            '<(native_src_path)/SkImage.cpp',
            '<(native_src_path)/ShadowLooper.cpp',
            '<(native_src_path)/JSImage.cpp',
            '<(native_src_path)/JSNidium.cpp',
            '<(native_src_path)/JSWindow.cpp',
            '<(native_src_path)/JSDocument.cpp',
            '<(native_src_path)/JSCanvas.cpp',
            '<(native_src_path)/CanvasHandler.cpp',
            '<(native_src_path)/Canvas2DContext.cpp',
            '<(native_src_path)/Canvas3DContext.cpp',
            '<(native_src_path)/App.cpp',
            '<(native_src_path)/NML.cpp',
            '<(native_src_path)/Assets.cpp',
            '<(native_src_path)/Context.cpp',
            '<(native_src_path)/CanvasContext.cpp',
            '<(native_src_path)/GLResources.cpp',
            '<(native_src_path)/GLState.cpp',
        ],
    }],
}
