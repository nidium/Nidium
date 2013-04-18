{
    'targets': [{
        'target_name': 'nativeinterface',
        'type': 'static_library',
        'include_dirs': [
            '../src/',
            '../network/',
            '<(third_party_path)/jsoncpp/include/',
            '<(third_party_path)/libzip/lib/',
            '<(third_party_path)/SDL2/include/',
            '<(third_party_path)/c-ares/',
			'<(third_party_path)/http-parser/',
            '<(native_interface_path)/',
        ],
        'conditions': [
            ['OS=="mac"', {
                'sources': [
                    '<(native_interface_path)/osx/NativeUIInterface.mm',
                    '<(native_interface_path)/osx/NativeUIConsole.mm',
                ],
			}],
            ['OS=="linux"', {
                'sources': [
                    '<(native_interface_path)/linux/NativeUIInterface.cpp',
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
