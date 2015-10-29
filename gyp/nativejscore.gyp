{
    'targets': [{
        'target_name': 'nativejscore-includes',
        'type': 'none',
        'direct_dependent_settings': {
            'include_dirs': [
                '<(third_party_path)/mozilla-central/dist/include/',
                '<(third_party_path)/mozilla-central/js/src/',
                '<(third_party_path)/mozilla-central/nsprpub/dist/include/nspr/',
                '<(third_party_path)/http-parser/',
                '<(third_party_path)/leveldb/include/',
                '../',
            ],
            'defines': [
                #'_FILE_OFFSET_BITS=64',
                '__STDC_LIMIT_MACROS',
                'JSGC_USE_EXACT_ROOTING'
            ],
            'cflags': [
                '-Wno-c++0x-extensions',
                # Flags needed to silent some SM warning
                '-Wno-invalid-offsetof',
                '-Wno-mismatched-tags',
                # Include our own js-config.h so it is automatically
                # versioned for our build flavour
                '-include <(native_output_third_party)/js-config.h'
            ],
            'xcode_settings': {
                'OTHER_CFLAGS': [
                    '-Wno-c++0x-extensions',
                    '-Wno-invalid-offsetof',
                    '-Wno-mismatched-tags',
                    '-include <(native_output_third_party)/js-config.h',
                ],
                "OTHER_LDFLAGS": [
                    '-stdlib=libc++'
                ],
                'OTHER_CPLUSPLUSFLAGS': [ 
                    '$inherited',
                ],
            },
        },
    }, {
        'target_name': 'nativejscore-link',
        'type': 'none',
        'direct_dependent_settings': {
            'conditions': [
                ['OS=="mac"', {
                    "link_settings": {
                        'libraries': [
                            'libcares.a',
                            'libhttp_parser.a',
                            'libnspr4.a',
                            'libjs_static.a',
                            'libleveldb.a',
                            '/usr/lib/libz.dylib',
                        ]
                    }
                }],
                ['OS=="linux"', {
                    "link_settings": {
                        'libraries': [
                            '-ljs_static',
                            '-lnspr4',
                            '-lpthread',
                            '-lrt',
                            '-ldl',
                            '-lz',
                            '-lcares',
                            '-lhttp_parser',
                            '-lleveldb',
                        ]
                    }
                }]
            ],
        },
    }, {
        'target_name': 'nativejscore',
        'type': 'static_library',
        'dependencies': [
            '../network/gyp/network.gyp:*',
            'jsoncpp.gyp:jsoncpp',
            'nativejscore.gyp:nativejscore-includes',
        ],
        'conditions': [
            ['OS=="mac"', {
                'defines': [
                    'DSO_EXTENSION=".dylib"'
                ],
            }],
            ['OS=="linux"', {
                'defines': [
                    'DSO_EXTENSION=".so"'
                ]
            }]
        ],
        'sources': [
            '../NativeHTTP.cpp',
            '../NativeJS.cpp',
            '../NativeJSExposer.cpp',
            '../NativeJSFileIO.cpp',
            '../NativeJSHttp.cpp',
            '../NativeJSModules.cpp',
            '../NativeJSSocket.cpp',
            '../NativeJSThread.cpp',
            '../NativeSharedMessages.cpp',
            '../NativeUtils.cpp',
            '../NativeJSStream.cpp',
            '../NativeMessages.cpp',
            '../NativeDB.cpp',
            '../NativeTaskManager.cpp',
            '../NativeFile.cpp',
            '../NativeStreamInterface.cpp',
            '../NativeFileStream.cpp',
            '../NativeHTTPStream.cpp',
            '../NativeNFSStream.cpp',
            '../NativeHTTPListener.cpp',
            '../NativeWebSocket.cpp',
            '../NativeJSWebSocket.cpp',
            '../NativePath.cpp',
            '../NativeJSUtils.cpp',
            '../NativeNFS.cpp',
            '../NativeJSHTTPListener.cpp',
            '../NativeJSDebug.cpp',
            '../NativeJSConsole.cpp',
            '../NativeJSProfiler.cpp',
            '../NativeJSFS.cpp',
            '../NativeJSProcess.cpp',
            '../NativeWebSocketClient.cpp',
            '../NativeHTTPParser.cpp',
        ],
    }],
}
