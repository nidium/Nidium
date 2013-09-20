{
    'targets': [{
        'target_name': 'nativejscore',
        'type': 'static_library',
        'include_dirs': [
            '../network/',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/c-ares/',
            '<(third_party_path)/http-parser/',
            '<(native_nativejscore_path)/',
        ],
        'conditions': [
            ['OS=="mac"', {
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-fvisibility=hidden',
                        '-Wno-invalid-offsetof'
                    ],
                },
            }],
            ['OS=="linux"', {
                'cflags': [
                    '-fvisibility=hidden',
                ],
            }]
        ],
        'sources': [
            '<(native_nativejscore_path)/NativeFileIO.cpp',
            '<(native_nativejscore_path)/NativeHTTP.cpp',
            '<(native_nativejscore_path)/NativeJS.cpp',
            '<(native_nativejscore_path)/NativeJSExposer.cpp',
            '<(native_nativejscore_path)/NativeJSFileIO.cpp',
            '<(native_nativejscore_path)/NativeJSHttp.cpp',
            '<(native_nativejscore_path)/NativeJSModules.cpp',
            '<(native_nativejscore_path)/NativeJSSocket.cpp',
            '<(native_nativejscore_path)/NativeJSThread.cpp',
            '<(native_nativejscore_path)/NativeSharedMessages.cpp',
            '<(native_nativejscore_path)/NativeStream.cpp',
            '<(native_nativejscore_path)/NativeUtils.cpp',
        ],
    }],
}
