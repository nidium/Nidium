# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nidiumcore-includes',
        'type': 'none',
        'direct_dependent_settings': {
            'include_dirs': [
                '<(third_party_path)/mozilla-central/dist/include/',
                '<(third_party_path)/mozilla-central/js/src/',
                '<(third_party_path)/mozilla-central/nsprpub/dist/include/nspr/',
                '<(third_party_path)/http-parser/',
                '<(third_party_path)/leveldb/include/',
                '<(third_party_path)/jsoncpp/dist',
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
                '-include <(nidium_output_third_party)/js-config.h'
            ],
            'xcode_settings': {
                'OTHER_CFLAGS': [
                    '-Wno-c++0x-extensions',
                    '-Wno-invalid-offsetof',
                    '-Wno-mismatched-tags',
                    '-include <(nidium_output_third_party)/js-config.h',
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
        'target_name': 'nidiumcore-link',
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
                            '-lleveldb',
                        ]
                    }
                }]
            ],
        },
    }, {
        'target_name': 'nidiumcore',
        'type': 'static_library',
        'dependencies': [
            '../network/gyp/network.gyp:*',
            'nidiumcore.gyp:nidiumcore-includes',
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
        'cflags': [
        ],
        'sources': [
            '<(third_party_path)/jsoncpp/dist/jsoncpp.cpp',
            '../src/Net/HTTP.cpp',
            '../src/Net/HTTPParser.cpp',
            '../src/Net/HTTPServer.cpp',
            '../src/Net/HTTPStream.cpp',
            '../src/Net/WebSocket.cpp',
            '../src/Net/WebSocketClient.cpp',

            '../src/Binding/NidiumJS.cpp',
            '../src/Binding/JSGlobal.cpp',
            '../src/Binding/JSExposer.cpp',
            '../src/Binding/JSFileIO.cpp',
            '../src/Binding/JSHTTP.cpp',
            '../src/Binding/JSHTTPServer.cpp',
            '../src/Binding/JSModules.cpp',
            '../src/Binding/JSSocket.cpp',
            '../src/Binding/JSThread.cpp',
            '../src/Binding/JSDebug.cpp',
            '../src/Binding/JSDebugger.cpp',
            '../src/Binding/JSConsole.cpp',
            '../src/Binding/JSFS.cpp',
            '../src/Binding/JSNFS.cpp',
            '../src/Binding/JSProcess.cpp',
            '../src/Binding/JSUtils.cpp',
            '../src/Binding/JSStream.cpp',
            '../src/Binding/JSWebSocket.cpp',
            '../src/Binding/JSWebSocketClient.cpp',
            '../src/Binding/JSDB.cpp',

            '../src/Core/SharedMessages.cpp',
            '../src/Core/Utils.cpp',
            '../src/Core/Messages.cpp',
            '../src/Core/DB.cpp',
            '../src/Core/TaskManager.cpp',
            '../src/Core/Path.cpp',

            '../src/IO/File.cpp',
            '../src/IO/Stream.cpp',
            '../src/IO/FileStream.cpp',
            '../src/IO/NFSStream.cpp',
            '../src/IO/NFS.cpp',
        ],
    }],
}
