# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'target_defaults': {
        'default_configuration': 'Release',
        'defines': [
            'NIDIUM_VERSION_STR="<(nidium_version)"',
            'NIDIUM_BUILD="<!@(git rev-parse HEAD)"',
            'NIDIUM_PLATFORM="<(OS)"',
            '<(nidium_product_define)'
            #'UINT32_MAX=4294967295u',
            #'_FILE_OFFSET_BITS=64',
            #'_HAVE_SSL_SUPPORT'
        ],
        'cflags_cc': [
            '-std=c++11'
        ],
        'cflags': [
           #'-fvisibility=hidden',
            '-Wall',
        ],
        'ldflags': [
            '-L<(nidium_output_third_party_path)',
        ],
        'msvs_configuration_platform': '<(platform)',
        'msvs_settings': {
            'VCLinkerTool': {
                'LinkTimeCodeGeneration': 0,
                'SuppressStartupBanner': 'true',
                'LinkIncremental': 0,
                "AdditionalLibraryDirectories": [
                    "<(libapenetwork_output_third_party_path)",
                    "<(nidium_output_third_party_path)"
                ],
                'AdditionalOptions': [
                    '/IGNORE:4049,4099,4217',
                ],
            },
            'VCCLCompilerTool': {
                'ExceptionHandling': 0,
                'AdditionalOptions': [
                    "/EHsc",
                    "-wd4067",
                 ],
            },
        },
        'xcode_settings': {
            "OTHER_LDFLAGS": [
                '-stdlib=libc++',
                # Because of an issue with gyp & xcode we need to hardcode this path : 
                # - On OSX xcodeproj files are generated inside the gyp/ directory of the gyp file called. 
                #     Settings --generator-output flag does not have any effect. XCode will "cd" inside 
                #     the directory of the xcodeproj file (this is problematic when building libapenetwork unit-tests)
                # - On Linux Makefie are generated inside the build/ directory at the root of the repo no mater what.
                '-L<(DEPTH)/build/third-party',
                #'-L<(nidium_output_third_party_path)',
            ],
            'OTHER_CPLUSPLUSFLAGS': [ 
                '-std=c++11',
                '-stdlib=libc++'
            ],
            'ARCHS': [
                '<(platform)',
            ],
            'MACOSX_DEPLOYMENT_TARGET': [
                '<(mac_deployment_target)'
            ],
            'SDKROOT': [
                'macosx<(mac_sdk_version)'
            ],
        },
        'configurations': {
            'Debug': {
                'msvs_settings': {
                    'VCCLCompilerTool': {
                        'DebugInformationFormat': '/ZI',
                     },
                    'VCCLCompilerTool': {
                        'RuntimeLibrary': 3, #Multithreaded using DLL /MDd (msvcrtd.lib)
                    }
                },
                #'defines': ['NIDIUM_DEBUG', 'DEBUG', '_DEBUG'],
                'defines': [],
                'ldflags': [
                    # Skia need to be linked with its own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj/gyp/'
                ],
                'cflags': [
                    '-O0',
                    '-g',
                ],
                'xcode_settings': {
                    'OTHER_CFLAGS': [ 
                        '-g',
                        '-O0'
                    ]
                }
            },
            'Release': {
                'msvs_settings': {
                    'VCCLCompilerTool': {
                        'RuntimeLibrary': 2, #Multithreaded using DLL /MD (msvcrt.lib)
                    }
                },
                'defines': [ 
                    'NDEBUG'
                ],
                'ldflags': [
                    # Skia need to be linked with his own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj/gyp/'
                ],
                'cflags': [
                    '-g',
                    '-O2',
                ],
                'xcode_settings': {
                    'OTHER_CFLAGS': [ 
                        '-g',
                        '-O2',
                    ]
                },
            }
        },

        'conditions': [
            ['target_os=="android"', {
                'defines': [
                    '__ANDROID__',
                    'ANDROID',
                ],
            }],
            ['nidium_enable_breakpad==1', {
                'defines': [ 'NIDIUM_ENABLE_CRASHREPORTER' ],
            }],
            # XXX : Remove me once we switched to .nfs file for privates
            ['nidium_package_embed==1', {
                'defines': [
                    'NIDIUM_EMBED_FILE="<(nidium_embed_bin_header)"',
                    'NIDIUM_PACKAGE_EMBED',
                ]
            }],
            ['asan==1', {
                'cflags': [
                    '-fsanitize=address'
                ],
                'ldflags': [
                    '-fsanitize=address'
                ],
                'xcode_settings': {
                    "OTHER_LDFLAGS": [
                        '-fsanitize=address'
                    ],
                    'OTHER_CFLAGS': [ 
                        '-fsanitize=address'
                    ]
                }
            }],
            ['profiler==1', {
                'defines': [
                    'NIDIUM_PROFILER',
                ],
                'ldflags': [
                    '-lprofiler'
                ],
                'xcode_settings': {
                    "OTHER_LDFLAGS": [
                        '-fsanitize=address'
                    ],
                }
            }],
        ],

        'target_conditions': [
            ['_type=="static_library"', {
                'standalone_static_library': 1, # disable thin archive
            }],
        ],
    }
}
