{
    'targets': [{
        'target_name': '<(native_exec_name)',
        'type': 'executable',
		'mac_bundle': 1,
        'product_dir': '../framework/dist/',
'dependencies': [
            '<(native_network_path)/gyp/network.gyp:nativenetwork',
            'interface.gyp:nativeinterface',
            'native.gyp:nativestudio',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore',
#            #'<(third_party_path)/skia/skia.gyp:alltargets',
        ],

        'include_dirs': [
            '<(native_src_path)',
            '<(native_network_path)',
            '<(native_interface_path)',
            '<(third_party_path)/SDL2/include/',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/http-parser/',
        ],
        'conditions': [
            ['OS=="linux"', {
                'conditions': [
                    ['native_use_gtk==1', {
                        'libraries': [
                            '<!@(pkg-config --libs gtk+-2.0)',
                        ]
                    }],
                    ['native_use_qt==1', {
                        'libraries': [
                            '<!@(pkg-config --libs QtCore QtGui)'
                        ]
                    }]
                ],
                'cflags': [
                    '-fno-rtti',
                    '-ffunction-sections',
                    '-fdata-sections',
                    '-fno-exceptions',
                    '-DTRIMMED',
                    '-freorder-blocks',
                    '-fomit-frame-pointer'
                ],
                'link_settings': {
                    'libraries': [
                        '-rdynamic',
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
                        '-lcoroutine',
                        '-Wl,-Bdynamic',

                        '-lswresample',
                        '-lswscale',
                        '-lavformat',
                        '-lavcodec',
                        '-lavutil',
                        '-ljpeg',
                        '-lskia_sfnt',
                        '-lskia_opts_ssse3',
                        '-lskia_opts',
                        '-lskia_utils',
                        '-lskia_ports',
                        '-lskia_images',
                        '-lskia_skgpu',
                        '-lskia_effects',
                        '-lskia_core',
                        '-ljs_static',
                        '-lleveldb',
                        '-Wl,--end-group'
                    ],
                },
                'include_dirs': [
                    '<(native_interface_path)/linux/',
                ],
                'sources': [
                    '<(native_app_path)/linux/main.cpp',
                    '<(third_party_path)/portaudio/src/common/pa_ringbuffer.o'
                ],
                #'actions': [{
                #    'action_name': 'strip',
                #    'inputs': '$(PRODUCT_DIR)/<(native_exec_name)',
                #    'outputs': '$(PRODUCT_DIR)/<(native_exec_name)',
                #    'action': ['strip', '$(PRODUCT_DIR)/<(native_exec_name)']
                #}]
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
                        'SDL2.framework',
                        'libskia_sfnt.a',
                        'libskia_opts_ssse3.a',
                        'libskia_opts.a',
                        'libskia_utils.a',
                        'libskia_ports.a',
                        'libskia_core.a',
                        'libskia_effects.a',
                        'libskia_images.a',
                        'libskia_skgpu.a',
                        'libjs_static.a',
                        'libportaudio.a',
                        'libzita-resampler.a',
                        'libavcodec.a',
                        'libavformat.a',
                        'libavutil.a',
                        'libswscale.a',
                        'libswresample.a',
                        'libhttp_parser.a',
                        'libjsoncpp.a',
                        'libzip.a',
                        'libnspr4.a',
                        'libcares.a',
                        'libcoroutine.a',
                        'libleveldb.a',
                        #'..//third-party/angle/libpreprocessor.a',
                        #'..//third-party/angle/libtranslator_common.a',
                        #'..//third-party/angle/libtranslator_glsl.a',
                        '/usr/lib/libbz2.dylib',
                        '/usr/lib/libz.dylib',
                        'libiconv.dylib'
                    ],
                },
                "xcode_settings": {
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
                        'postbuild_name': 'Increment build number',
                        'action': ['./osx/incbuild.sh']
                    },
                    {
                        'postbuild_name': 'Copy Frameworks',
                        'action': [
                            'ditto',
                            '<(native_output)/third-party-libs/.libs/SDL2.framework/',
                            '../framework/dist/nidium.app/Contents/Frameworks/SDL2.framework'
                        ]
                    },
					{
						'postbuild_name': 'Copy resources',
						'action': [
							'ditto',
							'<(native_output)/../resources/',
							'../framework/dist/nidium.app/Contents/Resources/'
						]
					}
                ]
            }],
        ],
    }]
}
