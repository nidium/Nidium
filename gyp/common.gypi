{
    'target_defaults': {
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
                'defines': ['NATIVE_DEBUG', 'DEBUG', '_DEBUG', 'JS_DEBUG', 'js_config_h'],
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
                'cxxflags': [
                    '-std=c++11'
                ],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS': [ 
                        '-stdlib=libc++',
                        '-std=c++11',
                        '-Wno-mismatched-tags'
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
            },
            'Release': {
                'product_dir': '<(native_output)/release/',
                'defines': [ 'NDEBUG', 'js_config_h', 'JS_THREADSAFE', 'JS_HAS_CTYPES', 'JS_HAVE_MACHINE_ENDIAN_H'],
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
                    'ARCHS': [
                        'x86_64',
                    ],
                    'OTHER_CPLUSPLUSFLAGS': [ 
                        '-stdlib=libc++',
                        '-g',
                        '-O2',
                        '-Wall',
                        '-Wno-invalid-offsetof',
                        '-std=c++11',
                        '-Wno-mismatched-tags'
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
                'cflags': [
                    '-O2',
                    '-g',
                    '-Wall',
                ],
                'cxxflags': [
                    '-std=c++11'
                ],
            }
        },
        'conditions': [
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
        ],
    },
}
