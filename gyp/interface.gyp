{
    'targets': [{
        'target_name': 'nativeinterface',
        'type': 'static_library',
        'include_dirs': [
            '../src/',
            '<(native_network_path)',
            '<(native_nativejscore_path)/',
            '<(native_interface_path)/',
            '<(third_party_path)/libzip/lib/',
            '<(third_party_path)/SDL2/include/',
            '<(third_party_path)/c-ares/',
            '<(third_party_path)/angle/include/',
        ],
        'dependencies': [
            '<(native_network_path)/gyp/network.gyp:nativenetwork-includes',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore-includes',
            '<(native_nativejscore_path)/gyp/jsoncpp.gyp:jsoncpp',
        ],
        'sources': [
            '<(native_interface_path)/NativeUIInterface.cpp',
        ],
        'conditions': [
            ['OS=="mac"', {
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-fvisibility=hidden',
                        '-Wno-invalid-offsetof'
                    ],
                },
                'sources': [
                    '<(native_interface_path)/osx/NativeCocoaUIInterface.mm',
                    '<(native_interface_path)/osx/NativeUIConsole.mm',
                    '<(native_interface_path)/osx/NativeSystem.mm',
                    '<(native_interface_path)/osx/NativeDragNSView.mm',
                ],
			}],
            ['OS=="linux"', {
                'sources': [
                    '<(native_interface_path)/linux/NativeX11UIInterface.cpp',
                    '<(native_interface_path)/linux/NativeSystem.cpp',
                ],
                'include_dirs': [
                    '<(native_interface_path)/linux/',
                ],
                'cflags': [
                    '-Wno-c++0x-extensions',
                    '-std=c++11',
                    '-Wno-invalid-offsetof'
                ],
                'conditions': [
                    ['native_use_gtk==1', {
                        'defines': ['NATIVE_USE_GTK'],
                        'cflags': [
                            '<!@(pkg-config --cflags gtk+-2.0)',
                        ],
                    }],
                    ['native_use_qt==1', {
                        'defines': ['NATIVE_USE_QT'],
                        'cflags': [
                            '<!@(pkg-config --cflags QtCore QtGui)'
                        ],
                    }]
                ],
                'direct_dependent_settings': {
                    'libraries': [
                        '-lX11',
                    ],
                    'conditions': [
                        ['native_use_gtk==1', {
                            'defines': ['NATIVE_USE_GTK'],
                            'libraries': [
                                '<!@(pkg-config --libs gtk+-2.0)',
                            ]
                        }],
                        ['native_use_qt==1', {
                            'defines': ['NATIVE_USE_QT'],
                            'libraries': [
                                '<!@(pkg-config --libs QtCore QtGui)'
                            ]
                        }]
                    ]
                },
            }],
        ],
    }],
}
