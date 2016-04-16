# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

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
                '../src/',
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
                            'libleveldb.a'
                        ]
                    }
                }],
                ['OS=="linux"', {
                    "link_settings": {
                        'libraries': [
                            '-rdynamic',
                            '-ljs_static',
                            '-lnspr4',
                            '-lpthread',
                            '-lrt',
                            '-ldl',
                            '-lcares',
                            '-lhttp_parser',
                            '-lleveldb'
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
            '../src/Net/NativeHTTP.cpp',
            '../src/Net/NativeWebSocketClient.cpp',
            '../src/Net/NativeHTTPParser.cpp',
            '../src/Net/NativeHTTPListener.cpp',
            '../src/Net/NativeWebSocket.cpp',
            '../src/Net/NativeHTTPStream.cpp',

            '../src/Binding/NativeJS.cpp',
            '../src/Binding/JSExposer.cpp',
            '../src/Binding/JSFileIO.cpp',
            '../src/Binding/JSHttp.cpp',
            '../src/Binding/NativeJSModules.cpp',
            '../src/Binding/NativeJSDebugger.cpp',
            '../src/Binding/JSSocket.cpp',
            '../src/Binding/JSThread.cpp',
            '../src/Binding/NativeJSHTTPListener.cpp',
            '../src/Binding/NativeJSDebug.cpp',
            '../src/Binding/JSConsole.cpp',
            '../src/Binding/JSFS.cpp',
            '../src/Binding/JSProcess.cpp',
            '../src/Binding/NativeJSWebSocket.cpp',
            '../src/Binding/NativeJSUtils.cpp',
            '../src/Binding/JSStream.cpp',
            '../src/Binding/NativeJSWebSocketClient.cpp',
            '../src/Binding/JSDB.cpp',

            '../src/Core/SharedMessages.cpp',
            '../src/Core/NativeUtils.cpp',
            '../src/Core/NativeMessages.cpp',
            '../src/Core/DB.cpp',
            '../src/Core/NativeTaskManager.cpp',
            '../src/Core/NativePath.cpp',

            '../src/IO/File.cpp',
            '../src/IO/Stream.cpp',
            '../src/IO/FileStream.cpp',
            '../src/IO/NFSStream.cpp',
            '../src/IO/NFS.cpp',
        ],
    }],
}
