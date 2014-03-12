{
    'targets': [{
        'target_name': 'nativejscore',
        'type': 'static_library',
        'include_dirs': [
            '<(DEPTH)/<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(DEPTH)/<(third_party_path)/c-ares/',
            '<(DEPTH)/<(third_party_path)/http-parser/',
            '<(DEPTH)/<(third_party_path)/leveldb/include/',
            '../network/',
            '../',
        ],
        'direct_dependent_settings': {
            'include_dirs': [
                '../'
            ]
        },
        'defines': ['_FILE_OFFSET_BITS=64'],
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
        ],
    }],
}
