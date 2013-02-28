{
    'defines': ['ENABLE_WEBGL=0'], # tmp hack
    'conditions': [
        ['OS=="mac"', {
            'xcode_settings': {
                'ARCHS': [
                    'x86_64',
                ],
                'OTHER_CFLAGS': [
                    '-v',
                    '-stdlib=libc++'
                ],
                'MACOSX_DEPLOYMENT_TARGET': [
                    '10.7'
                ],
                'SDKROOT': [
                    'macosx10.7'
                ]
            },
        }],
    ],
    'target_defaults': {
        
        'include_dirs': [
            '<(third_party_path)/c-ares/',
        ],
        'default_configuration': 'Release',
            'configurations': {
                'Debug': {
                    'defines': [ 'DEBUG', '_DEBUG' ],
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
                    'xcode_settings': {
                        #'OTHER_LDFLAGS': [
                        #    '-Lexternal/thelibrary/lib/debug'
                        #]
                    }
                },
                'Release': {
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
                    #'xcode_settings': {
                    #    'OTHER_LDFLAGS': [
                    #        '-Lexternal/thelibrary/lib/release'
                    #        ]
                    #}
                }
            }  
    }
}
