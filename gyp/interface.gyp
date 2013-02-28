{
    'targets': [{
        'target_name': 'nativeinterface',
        'type': 'static_library',
        'include_dirs': [
            '../src/',
            '../network/',
            '../third-party/jsoncpp/include/',
            '../third-party/libzip/lib/',
            '<(third_party_path)/SDL/include/',
            '<(third_party_path)/c-ares/',
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
                'cflags': [
                    '-fvisibility=hidden',
                    '-fvisibility-inlines-hidden',
                    '-Wno-invalid-offsetof'
                ],
            }],
        ],
    }],
}
