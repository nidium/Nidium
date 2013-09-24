{
    'targets': [{
        'target_name': 'nativeinterface',
        'type': 'static_library',
        'include_dirs': [
            '../src/',
            '<(native_network_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/jsoncpp/include/',
            '<(third_party_path)/libzip/lib/',
            '<(third_party_path)/SDL2/include/',
            '<(third_party_path)/c-ares/',
			'<(third_party_path)/http-parser/',
            '<(native_nativejscore_path)/',
            '<(native_interface_path)/',
        ],
        'conditions': [
            ['native_interface=="terminal"', {
                'sources': [
                    '<(native_interface_path)/terminal/NativeTerminalUIInterface.cpp',
                ],
                'include_dirs': [
                    '<(native_interface_path)/terminal/',
                ],
            }],
            ['OS=="mac"', {
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-fvisibility=hidden',
                        '-Wno-invalid-offsetof'
                    ],
                },
                'sources': [
                    '<(native_interface_path)/osx/NativeUIInterface.mm',
                    '<(native_interface_path)/osx/NativeUIConsole.mm',
                    '<(native_interface_path)/osx/NativeSystem.mm',
                ],
			}],
            ['OS=="linux"', {
                'sources': [
                    '<(native_interface_path)/linux/NativeUIInterface.cpp',
                    '<(native_interface_path)/linux/NativeSystem.cpp',
                ],
                'include_dirs': [
                    '<(native_interface_path)/linux/',
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
                ]
            }],
        ],
    }],
}
