{
    'target_defaults': {
        'product_prefix':'',
        'include_dirs': [
            '<(native_src_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
        ],
        'xcode_settings': {
            'OTHER_CFLAGS': [
                '-Wno-invalid-offsetof',
                '-Wno-c++0x-extensions'
            ],
        },
        'cflags': [
            '-Wno-invalid-offsetof',
            '-Wno-c++0x-extensions'
        ],
        'product_dir': '../framework/nidium_modules/',
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
            },
            'Release': {
            }
        }  
    },
}
