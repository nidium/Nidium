{
    'targets': [{
        'target_name': 'nativejscore',
        'type': 'static_library',
        'include_dirs': [
            '<(DEPTH)/<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(DEPTH)/<(third_party_path)/mozilla-central/js/src/',
            '<(DEPTH)/<(third_party_path)/mozilla-central/nsprpub/dist/include/nspr/',
            '<(DEPTH)/<(third_party_path)/c-ares/',
            '<(DEPTH)/<(third_party_path)/http-parser/',
            '<(DEPTH)/<(third_party_path)/leveldb/include/',
            '../network/',
            '../',
        ],
        'direct_dependent_settings': {
            'include_dirs': [
                '../'
                '<(DEPTH)/<(third_party_path)/openssl/include/',
            ],
            'conditions': [
                ['OS=="linux"', {
                    "link_settings": {
                        'libraries': [
                            '-lssl',
                            '-lcrypto'
                        ]
                    }
                }],
                ['OS=="mac"', {
                    "link_settings": {
                        'libraries': [
                            'libssl.a',
                            'libcrypto.a'
                        ]
                    }
                }]
            ]
        },
        'defines': [
            '_FILE_OFFSET_BITS=64',
            '__STDC_LIMIT_MACROS',
            '_HAVE_SSL_SUPPORT'
        ],
        'dependencies': [
            '../network/gyp/network.gyp:nativenetwork',
            'jsoncpp.gyp:jsoncpp'
        ],
        'conditions': [
            ['OS=="mac"', {
                'defines': [
                    'DSO_EXTENSION=".so"'
                ],
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-fvisibility=hidden'
                    ],
                },
            }],
            ['OS=="linux"', {
                'cflags': [
                    '-fvisibility=hidden',
                    '-Wno-c++0x-extensions',
                    '-Wno-invalid-offsetof'
                ],
                'defines': [
                    'DSO_EXTENSION=".so"'
                ],
            }]
        ],
        'sources': [
            '../NativeFileIO.cpp',
            '../NativeHTTP.cpp',
            '../NativeJS.cpp',
            '../NativeJSExposer.cpp',
            '../NativeJSFileIO.cpp',
            '../NativeJSHttp.cpp',
            '../NativeJSModules.cpp',
            '../NativeJSSocket.cpp',
            '../NativeJSThread.cpp',
            '../NativeSharedMessages.cpp',
            '../NativeStream.cpp',
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
        ],
    }],
}
