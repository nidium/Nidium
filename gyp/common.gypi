{
    'target_defaults': {
        'include_dirs': [
            '<(third_party_path)/c-ares/',
        ],
        'defines': [
            'NATIVE_VERSION_STR="<(native_version)"',
            'NATIVE_BUILD="<!@(git rev-parse HEAD)"',
            'NATIVE_CRASH_COLLECTOR_HOST="<(native_crash_collector_host)"',
            'NATIVE_CRASH_COLLECTOR_PORT=<(native_crash_collector_port)',
            'NATIVE_CRASH_COLLECTOR_ENDPOINT="<(native_crash_collector_endpoint)"',
            'UINT32_MAX=4294967295u',
            '_FILE_OFFSET_BITS=64'
        ],
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'product_dir': '<(native_output)/debug/',
                'defines': ['NATIVE_DEBUG', 'DEBUG', '_DEBUG'],
                'msvs_settings': {
                    'VCCLCompilerTool': {
                        'RuntimeLibrary': 1, 
                    },
                    'VCLinkerTool': {
                        'LinkTimeCodeGeneration': 1,
                        'OptimizeReferences': 2,
                        'EnableCOMDATFolding': 2,
                        'LinkIncremental': 1,
                        'GenerateDebugInformation': 'true',
                        #'AdditionalLibraryDirectories': [
                        #    '../external/thelibrary/lib/debug'
                        #]
                    }          
                },
                'cflags': [
                    '-g',
                ],
                'xcode_settings': {
                    "OTHER_LDFLAGS": [
                        '-L<(native_output)/third-party-libs/debug/',
                        '-F<(native_output)/third-party-libs/debug/',
                        '-stdlib=libc++'
                    ],
                    'OTHER_CPLUSPLUSFLAGS': [ 
                        '-stdlib=libc++'
                    ],
                    'ARCHS': [
                        'x86_64',
                    ],
                    'MACOSX_DEPLOYMENT_TARGET': [
                        '10.7'
                    ],
                    'SDKROOT': [
                        'macosx10.9'
                    ],
                    'OTHER_CFLAGS': [ 
                        '-g'
                    ]
                },
                'ldflags': [
                    '-L<(native_output)/third-party-libs/debug/',
                    # Skia need to be linked with its own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj.target/gyp/',
                ],
            },
            'Release': {
                'product_dir': '<(native_output)/release/',
                'defines': [ 'NDEBUG'],
                'msvs_settings': {
                    'VCCLCompilerTool': {
                        'RuntimeLibrary': 0,
                        'Optimization': 3,
                        'FavorSizeOrSpeed': 1,
                        'InlineFunctionExpansion': 2,
                        'WholeProgramOptimization': 'true',
                        'OmitFramePointers': 'true',
                        'EnableFunctionLevelLinking': 'true',
                        'EnableIntrinsicFunctions': 'true'            
                    },
                    'VCLinkerTool': {
                        'LinkTimeCodeGeneration': 1,
                        'OptimizeReferences': 2,
                        'EnableCOMDATFolding': 2,
                        'LinkIncremental': 1,
                        #'AdditionalLibraryDirectories': [
                        #    '../external/thelibrary/lib/debug'
                        #]            
                    }          
                },
                'xcode_settings': {
                    "OTHER_LDFLAGS": [
                        '-L<(native_output)/third-party-libs/release/',
                        '-F<(native_output)/third-party-libs/release/',
                    ],
                    'ARCHS': [
                        'x86_64',
                    ],
                    'OTHER_CPLUSPLUSFLAGS': [ 
                        '-stdlib=libc++'
                    ],
                    'MACOSX_DEPLOYMENT_TARGET': [
                        '10.7'
                    ],
                    'SDKROOT': [
                        'macosx10.9'
                    ],
                    'OTHER_CFLAGS': [ 
                        '-g',
                        '-O2',
                        '-Wall',
                    ]
                },
                'ldflags': [
                    '-L<(native_output)/third-party-libs/release/',
                    # Skia need to be linked with his own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj.target/gyp/',
                ],
                'cflags': [
                    '-O2',
                    '-g',
                ],
            }
        },
        'conditions': [
            ['native_enable_breakpad==1', {
                'defines': [ 'NATIVE_ENABLE_BREAKPAD' ],
            }],
            ['addresse_sanitizer==1', {
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
        ],
    },
}
