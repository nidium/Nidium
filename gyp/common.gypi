{
    'target_defaults': {
        'include_dirs': [
            '<(third_party_path)/c-ares/',
        ],
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'product_dir': '<(native_output)/debug/',
                'defines': [ 'NATIVE_DEBUG', 'DEBUG', '_DEBUG' ],
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
                    ],
                    'ARCHS': [
                        'x86_64',
                    ],
                    'MACOSX_DEPLOYMENT_TARGET': [
                        '10.7'
                    ],
                    'SDKROOT': [
                        'macosx10.7'
                    ],
                    'OTHER_CFLAGS': [ 
                        '-g'
                    ]
                },
                'ldflags': [
                    '-L<(native_output)/third-party-libs/debug/',
                    # Skia need to be linked with his own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj.target/gyp/',
                ],
            },
            'Release': {
                'product_dir': '<(native_output)/release/',
                'defines': [ 'NDEBUG' ],
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
                    'MACOSX_DEPLOYMENT_TARGET': [
                        '10.7'
                    ],
                    'SDKROOT': [
                        'macosx10.7'
                    ],
                    'OTHER_CFLAGS': [ 
                        '-g',
                        '-O0'
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
                    '-O0',
                    '-g',
                ],
                'conditions': [
                    ['native_strip_exec==1', {
                        'xcode_settings': {
                            'DEAD_CODE_STRIPPING': 'NO',
                            'DEPLOYMENT_POSTPROCESSING': 'NO',
                            'STRIP_INSTALLED_PRODUCT': 'NO'
                        },
                    }],
                ],
            }
        }  
    },
}
