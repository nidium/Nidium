{
    'target_defaults': {
        'product_prefix':'',
        'include_dirs': [
            '<(native_src_path)',
            '<(native_spidermonkey_path)',
            '<(third_party_path)/http-parser/',
        ],
        'xcode_settings': {
            'OTHER_CFLAGS': [
                '-Wno-invalid-offsetof',
                '-Wno-c++0x-extensions',
            ],
            'OTHER_LDFLAGS': [
                '-undefined suppress',
                '-flat_namespace'
            ]
        },
        'cflags': [
            '-Wno-invalid-offsetof',
            '-Wno-c++0x-extensions'
        ],
        'product_dir': '../framework/modules/',
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
            },
            'Release': {
            }
        }
    },
}

