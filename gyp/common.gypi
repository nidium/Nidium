# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': [
        '../src/libapenetwork/gyp/common.gypi'
    ],

    'target_defaults': {
        'defines': [
            'NIDIUM_VERSION_STR="<(nidium_version)"',
            'NIDIUM_NO_PRIVATE_DIR',
            'NIDIUM_FRAMEWORK_STR="delete me"', # XXX Remove this
            'NIDIUM_BUILD="<!@(git rev-parse HEAD)"',
            'NIDIUM_CRASH_COLLECTOR_HOST="<(nidium_crash_collector_host)"',
            'NIDIUM_CRASH_COLLECTOR_PORT=<(nidium_crash_collector_port)',
            'NIDIUM_CRASH_COLLECTOR_ENDPOINT="<(nidium_crash_collector_endpoint)"',
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
            '-L<(nidium_output_third_party)',
        ],

        'xcode_settings': {
            "OTHER_LDFLAGS": [
                '-stdlib=libc++',
                '-L<(nidium_output_third_party)',
                '-F<(nidium_output_third_party)'
            ],
            'OTHER_CPLUSPLUSFLAGS': [ 
                '-std=c++11',
                '-stdlib=libc++'
            ],
            'ARCHS': [
                'x86_64',
            ],
            'MACOSX_DEPLOYMENT_TARGET': [
                '<(mac_deployment_target)'
            ],
            'SDKROOT': [
                'macosx<(mac_sdk_version)'
            ],
        },

        'msvs_configuration_platform': 'x64',
        'msvs_settings': {
            'VCLinkerTool': {
                'LinkTimeCodeGeneration': 1,
                'SubSystem': '1',  # console app
                "AdditionalLibraryDirectories": ["<(nidium_output_third_party)"]
            }
        },

        'configurations': {
            'Debug': {
                'product_dir': '<(nidium_output)/debug/',
                'defines': ['NIDIUM_DEBUG', 'DEBUG', '_DEBUG'],
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
                'product_dir': '<(nidium_output)/release/',
                'defines': [ 'NDEBUG'],
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
            ['nidium_enable_breakpad==1', {
                'defines': [ 'NIDIUM_ENABLE_CRASHREPORTER' ],
            }],
            # XXX : Remove me once we switched to .nfs file for privates
            ['nidium_embed_private==1', {
                'defines': [
                    'NIDIUM_EMBED_PRIVATE="<(nidium_private_bin_header)"'
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
                'ldflags': [
                    '-lprofiler'
                ],
                'xcode_settings': {
                    "OTHER_LDFLAGS": [
                        '-fsanitize=address'
                    ],
                }
            }]
        ],

        'target_conditions': [
            ['_type=="static_library"', {
                'standalone_static_library': 1, # disable thin archive
            }],
        ],
    }
}
