{
    'targets': [{
        'target_name': 'js2bytecode',
        'type': 'executable',
        'product_dir': '../tools/',
        'include_dirs': [
            '<(native_src_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/mozilla-central/js/src/',
            '<(third_party_path)/mozilla-central/nsprpub/dist/include/nspr/',
            '<(native_nativejscore_path)/',
        ],
        'cflags': [
            '-Wno-c++0x-extensions',
            '-Wno-invalid-offsetof'
        ],
        'defines': [
            'PRIVATE_ROOT="<(native_exec_path)private"'
        ],
        'conditions': [
            ['OS=="linux"', {
                'link_settings': {
                    'libraries': [
                        '-lpthread',
                        '-lz',
                        '-ldl',
                        '-ljs_static',
                        '-lnspr4',
                        '-lrt',
                    ]
                },
            }],
            ['OS=="mac"', {
                'link_settings': {
                    'libraries': [
                        'libjs_static.a',
                        '/usr/lib/libz.dylib',
                        'libnspr4.a'
                    ]
                },
            }]
        ],
        'sources': [
            '<(native_src_path)/tools/js2bytecode.cpp',
        ],
    }]
}
