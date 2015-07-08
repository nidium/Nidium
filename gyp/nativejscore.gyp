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
                '__STDC_LIMIT_MACROS'
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'OTHER_CFLAGS': [
                            #'-fvisibility=hidden'
                        ],
                    },
                }],
                ['OS=="linux"', {
                    'cflags': [
                    #    '-fvisibility=hidden',
                        '-Wno-c++0x-extensions',
                        '-Wno-invalid-offsetof'
                    ],
                }]
            ],
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
                            '-lpthread',
                            '-lz',
                            '-ldl',
                            '-lrt',
                            '-ljs_static',
                            '-lnspr4',
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
        'includes': [
            '../network/gyp/config.gypi',
            '../network/gyp/common.gypi',
        ],
        'include_dirs': [
            '<(third_party_path)/mozilla-central/dist/include/',
            '<(third_party_path)/mozilla-central/js/src/',
            '<(third_party_path)/mozilla-central/nsprpub/dist/include/nspr/',
            '<(third_party_path)/leveldb/include/',
            '<(third_party_path)/http-parser/',
            '../',
        ],
        'defines': [
            #'_FILE_OFFSET_BITS=64',
            '__STDC_LIMIT_MACROS'
        ],
        'dependencies': [
            '../network/gyp/network.gyp:*',
            'jsoncpp.gyp:jsoncpp'
        ],
        'conditions': [
            ['OS=="mac"', {
                'defines': [
                    'DSO_EXTENSION=".dylib"'
                ],
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-fvisibility=hidden',
                    	'-std=c++0x'
                    ],
                },
            }],
            ['OS=="linux"', {
                'cflags': [
                    #'-fvisibility=hidden',
                    '-Wno-c++0x-extensions',
                    '-Wno-invalid-offsetof',
                    '-std=c++0x'
                ],
                'defines': [
                    'DSO_EXTENSION=".so"'
                ],
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
            '../NativeJSProcess.cpp'
        ],
    }],
}
