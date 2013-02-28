{
    'targets': [{
        'target_name': 'nativeapp',
        'type': 'executable',
		'mac_bundle': 1,
        'dependencies': [
            'network.gyp:nativenetwork',
            'interface.gyp:nativeinterface',
            'av.gyp:nativeav',
            'native.gyp:nativestudio',
            #'<(third_party_path)/skia/skia.gyp:alltargets',
        ],
        'include_dirs': [
            '<(native_src_path)',
            '<(native_network_path)',
            '<(native_interface_path)',
            '<(third_party_path)/SDL/include/',
            '<(third_party_path)/mozilla/js/src/dist/include/',
        ],
        'conditions': [
            ['OS=="linux"', {
                'cflags': [
                    '-fno-rtti',
                    '-ffunction-sections',
                    '-fdata-sections',
                    '-fno-exceptions',
                    '-DTRIMMED',
                    '-freorder-blocks',
                    '-fomit-frame-pointer'
                ],
                'ldflags': [
                    '-L<(third_party_path)/SDL/build/build/.libs/',
                    '-L<(third_party_path)/skia/out/Release/obj.target/gyp/',
                    '-L<(third_party_path)/mozilla/js/src/',
                    '-L<(third_party_path)/http-parser/',
                    '-L<(third_party_path)/portaudio/lib/.libs/',
                    '-L<(third_party_path)/ffmpeg/libavformat/',
                    '-L<(third_party_path)/ffmpeg/libavcodec/',
                    '-L<(third_party_path)/ffmpeg/libavutil/',
                    '-L<(third_party_path)/ffmpeg/libswscale/',
                    '-L<(third_party_path)/ffmpeg/libswresample/',
                    '-L<(third_party_path)/c-ares/.libs/',
                    '-L<(third_party_path)/libzip/lib/.libs/',
                    '-L<(third_party_path)/zita-resampler/libs/',
                    #-L ../../third-party/angle/out/Debug/obj.target/src/ \
                ],
                'libraries': [
#'-ltranslator_glsl',
#'-ltranslator_common',
#'-lpreprocessor',
                    '-Wl,--start-group',
                    '-lGL',
                    '-lasound',
                    '-lpthread',
                    '-lpng',
                    '-lfreetype',
                    '-lrt',
                    '-lz',
                    '-ldl',

                    '-Wl,-Bstatic '
                    '-lSDL2',
                    '-lzip',
                    '-lcares',
                    '-lhttp_parser',
                    '-ljsoncpp',
                    '-lportaudio',
                    '-lzita-resampler',
                    '-lnspr4',
                    '-Wl,-Bdynamic',

                    '-lswresample',
                    '-lswscale',
                    '-lavformat',
                    '-lavcodec',
                    '-lavutil',
                    '-lskia_sfnt',
                    '-lzlib',
                    '-ljpeg',
                    '-lskia_opts_ssse3',
                    '-lskia_opts',
                    '-lskia_utils',
                    '-lpicture_utils',
                    '-lskia_ports',
                    '-lskia_images',
                    '-lskia_skgr',
                    '-lskia_gr',
                    '-lskia_effects',
                    '-lskia_core',
                    '-ljs_static',
                    '-Wl,--end-group'
                ],
                'include_dirs': [
                    '<(native_interface_path)/linux/',
                ],
                'sources': [
                    '<(native_app_path)/linux/main.cpp',
                    '<(third_party_path)/portaudio/src/common/pa_ringbuffer.o'
                ]
            }],
            ['OS=="mac"', {
                "link_settings": {
                    'libraries': [
                        '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
                        '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreAudioKit.framework',
                        '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
                        '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
                        '<(third_party_path)/SDL/Xcode/SDL/out/SDL2.framework',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_sfnt.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_opts_ssse3.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_opts.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_utils.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_ports.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_core.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_effects.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_gr.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_images.a',
                        '<(third_party_path)/skia/xcodebuild/Release/libskia_skgr.a',
                        '<(third_party_path)/mozilla/js/src/libjs_static.a',
                        '<(third_party_path)/portaudio/lib/.libs/libportaudio.a',
                        '<(third_party_path)/zita-resampler/libs/libzita-resampler.a',
                        '<(third_party_path)/ffmpeg/libavcodec/libavcodec.a',
                        '<(third_party_path)/ffmpeg/libavformat/libavformat.a',
                        '<(third_party_path)/ffmpeg/libavutil/libavutil.a',
                        '<(third_party_path)/ffmpeg/libswscale/libswscale.a',
                        '<(third_party_path)/ffmpeg/libswresample/libswresample.a',
                        '<(third_party_path)/http-parser/libhttp_parser.a',
                        #'..//third-party/angle/libpreprocessor.a',
                        #'..//third-party/angle/libtranslator_common.a',
                        #'..//third-party/angle/libtranslator_glsl.a',
                        '/usr/local/Cellar/nspr/4.9.3/lib/libnspr4.a',
                        '/usr/local/Cellar/c-ares/1.9.1/lib/libcares.a',
                        '/usr/lib/libbz2.dylib',
                        '/usr/lib/libz.dylib',
                        'libjson_linux-gcc-4.2.1_libmt.a',
                        'libzip.a',
                    ],
                    'library_dirs': [
                        '<(third_party_path)/libzip/lib/.libs/',
                    ],
                },
                "xcode_settings": {
                    "OTHER_LDFLAGS": [
                        '-L<(third_party_path)/libzip/lib/.libs/',
                        '-L<(third_party_path)/jsoncpp/libs/linux-gcc-4.2.1/',
                        '-F<(third_party_path)/SDL/Xcode/SDL/out',
                    ],
                    'LD_RUNPATH_SEARCH_PATHS': [
                        '@loader_path/../Frameworks'
                    ],
                    'INFOPLIST_FILE': './osx/Info.plist',
                },
                'mac_bundle_resources': [
                    './osx/en.lproj/InfoPlist.strings',
                    './osx/en.lproj/MainMenu.xib',
                ],
                'include_dirs': [
                    '<(native_interface_path)/osx/',
                ],
                'sources': [
                    '<(native_app_path)/osx/main.mm',
                    '<(native_app_path)/osx/NativeStudioAppDelegate.mm',
                    '<(third_party_path)/portaudio/src/common/pa_ringbuffer.o'
                ],
                'postbuilds': [
                    {
                        'variables': {
                            'install_name_path': './osx/plugin_fix_install_names.sh',
                        },
                        'postbuild_name': 'Fix Framework Paths',
                        'action': ['<(install_name_path)'],
                    },
                    {
                        'variables': {
                            'copy_frameworks_path': './osx/plugin_copy_framworks.sh',
                        },
                        'postbuild_name': 'Copy Frameworks',
                        'action': [
                            'ditto',
                            '<(third_party_path)/SDL/Xcode/SDL/out/SDL2.framework',
                            '${BUILT_PRODUCTS_DIR}/nativeapp.app/Contents/Frameworks/SDL2.framework'
                        ]
                    }
                ]
            }]
        ]
    }]
}
