# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

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
        'cflags': [
            '-std=c++11',
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

