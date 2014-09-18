{
    'target_defaults': {
        'include_dirs': [
            '<(third_party_path)/c-ares/',
        ],
        'defines': [
            'NATIVE_VERSION_STR="<(native_version)"',
            'NATIVE_BUILD="<!@(git rev-parse HEAD)"',
            'UINT32_MAX=4294967295u',
            #'_FILE_OFFSET_BITS=64',
            'NATIVE_NO_PRIVATE_DIR'
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
                    '-O0',
                    '-g',
                ],
                'xcode_settings': {
                    "OTHER_LDFLAGS": [
                        '-L<(native_output)/third-party-libs/debug/',
                        '-F<(native_output)/third-party-libs/debug/',
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
                        '-g',
                        '-O0'
                    ]
                },
                'ldflags': [
                    '-L<(native_output)/third-party-libs/debug/',
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
                        '-F<(native_output)/third-party-libs/release/'
                    ],
                    'ARCHS': [
                        'x86_64',
                    ],
                    'OTHER_CPLUSPLUSFLAGS': [ 
                        '-stdlib=libc++',
                        '-g',
                        '-O2',
                        '-Wall',
                        '-Wno-invalid-offsetof'
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
                        '-stdlib=libc++',
                        '-Wno-invalid-offsetof'
                    ]
                },
                'ldflags': [
                    '-L<(native_output)/third-party-libs/release/',
                ],
                'cflags': [
                    '-O2',
                    '-g',
                    '-Wall',
                ],
            }
        }
    },
}
