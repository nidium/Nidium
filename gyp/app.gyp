{
    'targets': [{
        'target_name': '<(native_exec_name)',
        'type': 'executable',
        'mac_bundle': 1,
        'product_dir': '<(native_exec_path)',
        'dependencies': [
            'native.gyp:*',
            '<(native_network_path)/gyp/network.gyp:*',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:*',
            '<(native_nativejscore_path)/gyp/jsoncpp.gyp:jsoncpp',
            'interface.gyp:nativeinterface',
        ],
        'include_dirs': [
            '<(native_src_path)',
            '<(native_network_path)',
            '<(native_interface_path)',
            '<(third_party_path)/breakpad/src/',
            '<(third_party_path)/SDL2/include/',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/http-parser/',
        ],
        'conditions': [
            ['OS=="linux"', {
                'conditions': [
                    ['native_enable_breakpad==1', {
                        'dependencies': [
                            'crashreporter.gyp:nidium-crash-reporter',
                            'breakpad.gyp:*'
                         ],
                    }],
                ],
                'cflags': [
                    '-fno-rtti',
                    '-ffunction-sections',
                    '-fdata-sections',
                    '-fno-exceptions',
                    '-DTRIMMED',
                    '-freorder-blocks',
                    '-fomit-frame-pointer',
                    '-std=c++11',
                    '-Wno-invalid-offsetof'
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
                        '-lbz2',
                        '-ldl',

                        '-Wl,-Bstatic ',
                        '-lSDL2',
                        '-lzip',
                        '-lportaudio',
                        '-lzita-resampler',
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
                        '-Wl,--end-group',
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
                # XXX : Dono why, but this has no effect
                "xcode_settings": {
                    'STRIP_INSTALLED_PRODUCT': 'YES',
                    'COPY_PHASE_STRIP': 'YES',
                    'DEBUGGING_SYMBOLS': 'NO',
                    'DEAD_CODE_STRIPPING': 'YES'
                },
                'conditions': [
                    ['native_enable_breakpad==1', {
                        'dependencies+': [
                            'breakpad.gyp:*'
                         ],
                        "xcode_settings": {
                            'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
                            'DEPLOYEMENT_POSTPROCESSING': 'YES',
                        }
                    }],
                ],
                "link_settings": {
                    'libraries': [
                        '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
                        '$(SDKROOT)/System/Library/Frameworks/ForceFeedback.framework',
                        '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
                        '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
                        '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
                        '$(SDKROOT)/System/Library/Frameworks/VideoDecodeAcceleration.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreAudioKit.framework',
                        '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
                        '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
                        'libSDL2.a',
                        'libskia_sfnt.a',
                        'libskia_opts_ssse3.a',
                        'libskia_opts.a',
                        'libskia_utils.a',
                        'libskia_ports.a',
                        'libskia_core.a',
                        'libskia_effects.a',
                        'libskia_images.a',
                        'libskia_skgpu.a',
                        'libportaudio.a',
                        'libzita-resampler.a',
                        'libavcodec.a',
                        'libavformat.a',
                        'libavutil.a',
                        'libswscale.a',
                        'libswresample.a',
                        'libzip.a',
                        'libcoroutine.a',
                        #'..//third-party/angle/libpreprocessor.a',
                        #'..//third-party/angle/libtranslator_common.a',
                        #'..//third-party/angle/libtranslator_glsl.a',
                        '/usr/lib/libbz2.dylib',
                        '/usr/lib/libz.dylib',
                        'libiconv.dylib'
                    ],

                },
                "xcode_settings": {
                    'MACOSX_DEPLOYMENT_TARGET': [
                        '10.7'
                    ],
                    'LD_RUNPATH_SEARCH_PATHS': [
                        '@loader_path/../Frameworks'
                    ],
		    'OTHER_LDFLAGS': [
			    '-stdlib=libc++',
		    ],
                    'OTHER_CFLAGS': [ 
                        '-g',
                        '-O2',
                        '-Wall',
                    ],
                    'INFOPLIST_FILE': '<(native_resources_path)/osx/Info.plist',
                },
                'mac_bundle_resources': [
                    '<(native_resources_path)/osx/en.lproj/InfoPlist.strings',
                    '<(native_resources_path)/osx/en.lproj/MainMenu.xib',
                ],
                'include_dirs': [
                    '<(native_interface_path)/osx/',
                    '<(third_party_path)/breakpad/src/client/mac/Framework/',
                    '<(third_party_path)/breakpad/src/client/apple/Framework/'
                ],
                'sources': [
                    '<(native_app_path)/osx/main.mm',
                    '<(native_app_path)/osx/NativeStudioAppDelegate.mm',
                    '<(third_party_path)/portaudio/src/common/pa_ringbuffer.o'
                ],
                'postbuilds': [
                    #{
                    #    'postbuild_name': 'Copy Frameworks',
                    #    'action': [
                    #        'ditto',
                    #        '<(native_output)/third-party-libs/.libs/SDL2.framework/',
                    #        '<(native_exec_path)/<(native_exec_name).app/Contents/Frameworks/SDL2.framework'
                    #    ]
                    #},
                    {
                        'postbuild_name': 'Copy resources',
                        'action': [
                            'cp',
                            '-r',
                            '<(native_resources_path)/osx/',
                            '<(native_exec_path)/<(native_exec_name).app/Contents/Resources/'
                        ]
                    },
                    {
                        'postbuild_name': 'Increment build number',
                        'action': [
                            './osx/incbuild.sh',
                            '<(native_exec_path)/<(native_exec_name).app/Contents/'
                        ]
                    }
                ]
            }],
        ],
    }]
}
